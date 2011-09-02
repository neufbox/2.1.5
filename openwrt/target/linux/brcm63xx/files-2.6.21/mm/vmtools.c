/*
<:copyright-gpl
 Copyright 2006 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/

/********************************************************
    vmtools.c

    Getting memory information for memory footprint optimization
    Contains code from mm/vmscan.c and fs/proc

    4/18/2006  Xi Wang        Created  

 ********************************************************/


#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel_stat.h>
#include <linux/swap.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/writeback.h>
#include <linux/suspend.h>
#include <linux/blkdev.h>
#include <linux/mm_inline.h>
#include <linux/pagevec.h>
#include <linux/backing-dev.h>
#include <linux/rmap.h>
#include <linux/topology.h>
#include <linux/cpu.h>
#include <linux/notifier.h>

#include <asm/tlbflush.h>
#include <asm/div64.h>

#include <linux/swapops.h>

// Repeat some definintions from other kernel .c files
#define lru_to_page(_head) (list_entry((_head)->prev, struct page, lru))
struct scan_control {
	unsigned long nr_to_scan;
	unsigned long nr_scanned;
	unsigned long nr_reclaimed;
	unsigned long nr_mapped;
	int nr_to_reclaim;
	unsigned int priority;
	unsigned int gfp_mask;
	int may_writepage;
};
int shrink_list(struct list_head *page_list, struct scan_control *sc);


void full_scan_free_pages();
static void full_scan_shrink_cache(struct zone *zone, struct scan_control *sc);
int pagewalk(char *print);



// Scan all pages to free memory
void full_scan_free_pages()
{
	struct scan_control sc;
	unsigned long lru_pages = 0;
	int i;

    struct zones **zones = NODE_DATA(0)->node_zonelists;

	sc.gfp_mask = GFP_KERNEL;
	sc.may_writepage = 1;

	inc_page_state(allocstall);

	for (i = 0; zones[i] != NULL; i++) {
		struct zone *zone = zones[i];
		lru_pages += zone->nr_active + zone->nr_inactive;
	}

	sc.nr_mapped = read_page_state(nr_mapped);
	sc.nr_scanned = 0;
	sc.nr_reclaimed = 0;
	sc.priority = 0;
	full_scan_shrink_cache(zones[0], &sc);
    //shrink_slab(sc.nr_scanned, sc.gfp_mask, lru_pages);
}


// Scan all pages to free memory - freeing page cache
static void full_scan_shrink_cache(struct zone *zone, struct scan_control *sc)
{
	LIST_HEAD(page_list);
	LIST_HEAD(page_active_list);
	struct pagevec pvec;
	struct page *page;
	int nr_taken = 0;
    int nr_taken_total = 0;
	int nr_scan = 0;
	int nr_freed;

    zone->nr_scan_active = 0;
    zone->nr_scan_inactive = 0;

	pagevec_init(&pvec, 1);

	lru_add_drain();
	spin_lock_irq(&zone->lru_lock);
    
    // add full inactive list
	while (!list_empty(&zone->inactive_list)) {
		page = lru_to_page(&zone->inactive_list);

		//prefetchw_prev_lru_page(page, &zone->inactive_list, flags);
		if (!TestClearPageLRU(page))
			BUG();
		list_del(&page->lru);
		if (get_page_testone(page)) {
			/*
			 * It is being freed elsewhere
			 */
			__put_page(page);
			SetPageLRU(page);
			list_add(&page->lru, &zone->inactive_list);
			continue;
		}
		list_add(&page->lru, &page_list);
		nr_taken++;
	}
	zone->nr_inactive -= nr_taken;
	zone->pages_scanned += nr_taken;
    nr_taken_total += nr_taken;
    nr_taken = 0;

    // add full active list
	while (!list_empty(&zone->active_list)) {
		page = lru_to_page(&zone->active_list);

		//prefetchw_prev_lru_page(page, &zone->active_list, flags);
		if (!TestClearPageLRU(page)) {
			BUG();
			break;
        }
		list_del(&page->lru);
		if (get_page_testone(page)) {
			/*
			 * It is being freed elsewhere
			 */
			__put_page(page);
			SetPageLRU(page);
			list_add(&page->lru, &zone->active_list);
			continue;
		}
   		if (page_mapped(page)) {
            page_map_lock(page);
			if (page_referenced(page)) {
				page_map_unlock(page);
				__put_page(page);
				SetPageLRU(page);
				list_add(&page->lru, &zone->active_list);
				continue;
			}
            page_map_unlock(page);
        }
		if (!TestClearPageActive(page)) {
    		BUG();            
            break;
        }
		list_add(&page->lru, &page_list);
		nr_taken++;
	}
	zone->nr_active -= nr_taken;
	zone->pages_scanned += nr_taken;
    nr_taken_total += nr_taken;
    
	spin_unlock_irq(&zone->lru_lock);

	if (nr_taken_total == 0)
		goto done;

	mod_page_state_zone(zone, pgscan_direct, nr_scan);
	nr_freed = shrink_list(&page_list, sc);
	mod_page_state_zone(zone, pgsteal, nr_freed);
	sc->nr_to_reclaim -= nr_freed;

	spin_lock_irq(&zone->lru_lock);
	/*
	 * Put back any unfreeable pages.
	 */
	while (!list_empty(&page_list)) {
		page = lru_to_page(&page_list);
		if (TestSetPageLRU(page))
			BUG();
		list_del(&page->lru);
        SetPageActive(page);
		add_page_to_active_list(zone, page);
		if (!pagevec_add(&pvec, page)) {
			spin_unlock_irq(&zone->lru_lock);
			__pagevec_release(&pvec);
			spin_lock_irq(&zone->lru_lock);
		}
	}
	spin_unlock_irq(&zone->lru_lock);
done:
	pagevec_release(&pvec);
}


// Traverse all pages in use and print out owners 
int pagewalk(char *print)
{
    #define PGS 4
    struct zones **zones = NODE_DATA(0)->node_zonelists;
    struct zone *zone = zones[0];
    struct list_head *curlist;
	struct page *page;
    int i;
    int total=0, file_mapped=0, anon_mapped=0, unmapped=0, dirty=0;
    
	spin_lock_irq(&zone->lru_lock);

    for (i=0; i<2; i++) {

        curlist= i?&zone->inactive_list:&zone->active_list;

        list_for_each_entry(page, curlist, lru) {
            struct address_space *a_space;
            struct inode *i_node;
            struct dentry *d_entry;
            total++;
            printk("addr:%x    ", page_address(page));
            if (page_count(page) == 0) {
                printk("free    ");
            }
            else {
                if (page_mapped(page)) {
                    printk("mapped    ");
                    if (!PageAnon(page)) {
                        file_mapped++;
                        printk("to_file    ");
                        a_space = page_mapping(page);
                        if (a_space) {
                            //printk("pages %d    ", (unsigned)(a_space->nrpages));
                        	i_node = a_space->host;
                            if (i_node) {
                                //printk("size %d    ", (int)(i_node->i_size));
                                d_entry = list_entry(i_node->i_dentry.next, struct dentry, d_alias);
                                if (d_entry) {
                                    printk("name:%s    ", d_entry->d_name.name);
                                }
                            }
                        }
                    }
                    else {
                        struct anon_vma *anon_mapping;
                        struct vm_area_struct *vm_area;
                        struct mm_struct *mmstruct;
                        struct task_struct *itask;
                        int flag;
                        anon_mapped++;
                        printk("anonymous    ");
                        anon_mapping = (struct anon_vma *) page->mapping;
                        list_for_each_entry(vm_area, &(anon_mapping->head), anon_vma_node) {
                            mmstruct = vm_area->vm_mm;
                            flag = 0; 
                            for_each_process(itask) {
                                if (itask->mm == mmstruct) {
                                  	int res = 0;
                                    unsigned int len;
                                    char buffer[256];
                                	struct mm_struct *mm = get_task_mm(itask);

                                 	len = mm->arg_end - mm->arg_start;
                                 
                                	if (len > sizeof(buffer))
                                		len = sizeof(buffer);
                                 
                                	res = access_process_vm(itask, mm->arg_start, buffer, len, 0);

                                	// If the nul at the end of args has been overwritten, then
                                	// assume application is using setproctitle(3).
                                	if (res > 0 && buffer[res-1] != '\0') {
                                		len = strnlen(buffer, res);
                                		if (len < res) {
                                		    res = len;
                                		} else {
                                			len = mm->env_end - mm->env_start;
                                			if (len > sizeof(buffer) - res)
                                				len = sizeof(buffer) - res;
                                			res += access_process_vm(itask, mm->env_start, buffer+res, len, 0);
                                			res = strnlen(buffer, res);
                                		}
                                	}
                                	mmput(mm);

                                    printk("owner:%d/%s    ", itask->pid, buffer);
                                }
                            }
                        }
                    }
                        
                }
                else {
                    unmapped++;
                    printk("unmapped (kernel mem)    ");
                }
            }

            if (PageLocked(page)) {
                printk("locked ");
            }
            if (PageDirty(page)) {
                dirty++;
                printk("dirty ");
            }
            if (PageSlab(page)) {
                printk("slab ");
            }
            if (PagePrivate(page)) {
                printk("private field used (buffer pages) ");
            }
            if (PageWriteback(page)) {
                printk("writeback ");
            }
            if (PageSwapCache(page)) {
                printk("swapcache ");
            }
            printk("\n");
        }
    }
    
	spin_unlock_irq(&zone->lru_lock);

    printk("\ntotal paged memory in use %dk    file mapped %dk    anonymously mapped %dk    unmapped %dk    dirty %dk\n", total*PGS, file_mapped*PGS, anon_mapped*PGS, unmapped*PGS, dirty*PGS);

    return 0;
 }

