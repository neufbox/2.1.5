/*
<:copyright-gpl
 Copyright 2007 Broadcom Corp. All Rights Reserved.

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

/************************************************************
    proc_brcm.c

    procfs entries like proc/brcm/pagewalk and proc/brcm/cstat

    9/27/2006  Xi Wang        Created  

 ************************************************************/

#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/tty.h>
#include <linux/string.h>
#include <linux/mman.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/pagemap.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/signal.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/smp_lock.h>
#include <linux/seq_file.h>
#include <linux/times.h>
#include <linux/profile.h>
#include <linux/blkdev.h>
#include <linux/hugetlb.h>
#include <linux/jiffies.h>
#include <linux/sysrq.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/tlb.h>
#include <asm/div64.h>

#ifdef CONFIG_BRCM_CSTAT

#define PERF_C_INTERVAL (HZ*1)
#define DIV 1000
#define N_INST
#define COUNTER_RESET_V 0xffffffffu

#define BRCM_PERFREG_BASE 0xff420000
typedef struct {
    unsigned global_ctrl;
    unsigned ctrl[2];
    unsigned donottouch[1];
    unsigned counters[4];
} PerformanceControl;
#define BRCM_PERF ((volatile PerformanceControl *) BRCM_PERFREG_BASE)

struct timer_list cachestat_timer;
int cachestat_interval = 0;

extern int proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len);
void static brcm_perf_timer_func(unsigned long data);
static int perf_counters_proc(char *page, char **start, off_t off, int count, int *eof, void *data);


static void cachestat_timer_func(unsigned long data)
{
    static int tp = 0;
    static int item = 0;
    register unsigned long temp;
    int i,ratio;
    unsigned tdiv = cachestat_interval*DIV;
    unsigned counters[4];
    
    for (i=0;i<4;i++) {
        counters[i] = COUNTER_RESET_V - BRCM_PERF->counters[i];
        BRCM_PERF->counters[i]=COUNTER_RESET_V;
    }
    
    if (item == 0) {   
        printk("TP %d instruction miss %uk/sec\n", tp, counters[0]/tdiv);
        printk("TP %d instruction hit %uk/sec\n", tp, counters[1]/tdiv);
        ratio = (counters[0]+counters[1])? counters[0]*1000/(counters[0]+counters[1]) : 0;
        printk("TP %d miss ratio %u\n", tp, ratio);
    }

    if (item == 1) {   
        printk("TP %d data miss %uk/sec\n", tp, counters[0]/tdiv);
        printk("TP %d data hit %uk/sec\n", tp, counters[1]/tdiv);
        ratio = (counters[0]+counters[1])? counters[0]*1000/(counters[0]+counters[1]) : 0;
        printk("TP %d miss ratio %u\n", tp, ratio);
    }

#if defined(N_INST)   
    printk("TP %d number of instructions %uk/sec\n", tp, counters[2]/tdiv);
    printk("TP %d number of cycles %uk/sec\n", tp, counters[3]/tdiv);
#endif

    if (tp >= 1) {
        printk("\n");
        tp = 0;
        if (item >= 1) {
            item = 0;
        }
        else {            
            item++;
        }
    }
    else {
        tp++;
    }
    
    if (tp ==0) {
        asm("mfc0 %0,$22,2" : "=d" (temp));
        temp &= 0x3fffffff;
        temp |= 0x00000000;
        asm("mtc0 %0,$22,2" :: "d" (temp));
    }
    else {
        asm("mfc0 %0,$22,2" : "=d" (temp));
        temp &= 0x3fffffff;
        temp |= 0x40000000;
        asm("mtc0 %0,$22,2" :: "d" (temp));
    }    

    if (item == 0) {
        BRCM_PERF->global_ctrl = 0x0;
        BRCM_PERF->global_ctrl = 0x80000018;
        if (tp == 0) {
            BRCM_PERF->ctrl[0] = 0x80188014;
        }
        else {
            BRCM_PERF->ctrl[0] = 0xa018a014;
        }
    }
    
    if (item == 1) {
        BRCM_PERF->global_ctrl = 0x0;
        BRCM_PERF->global_ctrl = 0x80000011;
        if (tp == 0) {
            BRCM_PERF->ctrl[0] = 0x80288024;
        }
        else {
            BRCM_PERF->ctrl[0] = 0xa028a024;
        }
    }

#if defined(N_INST)   
    if (tp ==0) {
        BRCM_PERF->ctrl[1] = 0x80488044;
    }
    else {
        BRCM_PERF->ctrl[1] = 0xa048a044;
    }
#endif

    cachestat_timer.expires = jiffies+cachestat_interval*HZ;
    add_timer(&cachestat_timer);
}


static void cachestat_start()
{
    int i;

    printk("Starting cache performance counters..\n\n");
    
    init_timer(&cachestat_timer);
    cachestat_timer.expires = jiffies+HZ;
    cachestat_timer.data = 0;
    cachestat_timer.function = cachestat_timer_func;

    for (i=0;i<4;i++) {
        BRCM_PERF->counters[i]=COUNTER_RESET_V;
    }

    BRCM_PERF->global_ctrl = 0x80000018;
    BRCM_PERF->global_ctrl = 0x80000011;

    add_timer(&cachestat_timer);
}

static void cachestat_stop()
{
    del_timer_sync(&cachestat_timer);
    printk("Cache performance counting stopped..\n");
}

static int cachestat_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;

    len += sprintf(page, "%d\n", cachestat_interval);

	return proc_calc_metrics(page, start, off, count, eof, len);

}


static int cachestat_write_proc(struct file *file, const char *buf, unsigned long count, void *data)
{
    char ibuf[20];
    int arg;

    if (count<1 || count>sizeof(ibuf)) {
        return -EFAULT;
    }
    if (copy_from_user(ibuf, buf, count)) {
        return -EFAULT;
    }
    ibuf[count] = 0;

    if (sscanf(ibuf, "%d\n", &arg) == 1) {
        if (arg>=0) {
            if (arg && !cachestat_interval) {
                cachestat_interval = arg;
                cachestat_start();
            }
            else if (!arg && cachestat_interval) {
                cachestat_interval = arg;
                cachestat_stop();
            }
            else {
                cachestat_interval = arg;
            }
        }
        return count;
    }
    return -EFAULT;
}


static int cp0regs_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    register unsigned long temp;
    unsigned long *mips_core_base = (unsigned long *)0xff400000;
	int len=0;

    len += sprintf(page+len, "Running on processor %d\n", smp_processor_id());
    asm("mfc0 %0,$22,0" : "=d" (temp));
    len += sprintf(page+len, "BRCM Config_0 = %x\n", temp);
    asm("mfc0 %0,$22,2" : "=d" (temp));
    len += sprintf(page+len, "CMT Control = %x\n", temp);
    asm("mfc0 %0,$22,3" : "=d" (temp));
    len += sprintf(page+len, "CMT Local = %x\n", temp);

    asm("mfc0 %0,$22,6" : "=d" (temp));
    len += sprintf(page+len, "Core Base = %x\n", temp);
    len += sprintf(page+len, "RAC Config (%x) = %x\n", mips_core_base, *mips_core_base);
    len += sprintf(page+len, "RAC Range (%x) = %x\n", mips_core_base+1, *(mips_core_base+1));
    len += sprintf(page+len, "RAC Config1 (%x) = %x\n", mips_core_base+2, *(mips_core_base+2));
    len += sprintf(page+len, "LMB (%x) = %x\n", mips_core_base+7, *(mips_core_base+7));
    
    *(page+len) = 0;
    len++;
	return proc_calc_metrics(page, start, off, count, eof, len);
}

#endif


#ifdef CONFIG_BRCM_VMTOOLS

extern int pagewalk(char *print);
extern void full_scan_free_pages();

static int pagewalk_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

    printk("\nList of all vm pages in use and their owners\n");
    printk("(output produced by printk)\n\n");

    len = pagewalk(page);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int free_pagewalk_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

    printk("\nList of all vm pages in use and their owers - sampled after freeing pages with a full memory scan\n");
    printk("(output produced by printk)\n\n");

    full_scan_free_pages();

    len = pagewalk(page);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

#endif

void __init proc_brcm_init(struct proc_dir_entry *pentry)
{
    struct proc_dir_entry *entry;

#ifdef CONFIG_BRCM_VMTOOLS
	create_proc_read_entry("pagewalk", 0, pentry, pagewalk_read_proc, NULL);
	create_proc_read_entry("free_pagewalk", 0, pentry, free_pagewalk_read_proc, NULL);
#endif
#ifdef CONFIG_BRCM_CSTAT
	entry = create_proc_entry("cstat", 0, pentry);
	entry->read_proc = cachestat_read_proc;
	entry->write_proc = cachestat_write_proc;
	create_proc_read_entry("cp0regs", 0, pentry, cp0regs_read_proc, NULL);
	//create_seq_entry("cstat_options", 0, &cachestat_options_read_proc;);
#endif
}


