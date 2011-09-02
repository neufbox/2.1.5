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
/*
 *******************************************************************************
 *
 * File Name	: klob.c
 *
 * Updates	: Jan 31st 2007
 *
 * Limitation	: Implemented for 32bit MIPS, No SMP.
 *
 * Background:
 *
 * SLAB allocator and SLOB (Small List Of Blocks 2.6.16) are the two
 * memory management systems in Linux. The SLAB allocator uses a
 * pre-allocated pool of fixed sized blocks, where the sizes are fixed at
 * powers-of-two. The two main issues with the SLAB allocator are:
 *    1. the "internal" fragmentation caused when a requested size is not
 *       equal to a power-of-2.
 *    2. sparsely populated slabs cannot be reaped,
 *	 see also  contribution from kmem_cache_alloc_node.
 *
 * The SLOB allocator on the other hand attempts to address wastage
 * by limiting the internal fragmentation to:
 *   - 8 bytes (rounding) for requests smaller than PAGE_SIZE=4K,
 *   - or PAGE_SIZE for sizes larger than PAGE_SIZE.
 * The main issue with SLOB allocator is the consequence of external
 * fragmentation. The size of the list of free small blocks can become
 * very large. This could impact both performance as well as memory over
 * time. Performance impact in an allocation request results from the
 * need to walk the free list to search for a candidate block that is
 * equal or larger than the requested size. In the latter case the
 * candidate block is split into two parts. Performance impact in a
 * deallocation requests results from the need to walk the free list
 * which is sorted by the block physical addresses, to determine whether
 * a coalesce with the lower or higher memory block is possible.
 * In the BCM963XX, the free list was noted to be over 200 blocks on
 * board bootup, with an average walk of 5 blocks on allocation or
 * deallocation. However, the worst case walk was noted to be maximum
 * size of the free list, i.e. 200 blocks. The Memory impact of
 * external fragmentation over time stems from the observation that while
 * the total free memory may be large, there may not be a single small
 * block that is contiguous, forcing the SLOB allocator to extend with
 * another page (4K).
 *
 * Derivation of KLOB allocator:
 *
 * Documentation Disclaimer:
 * The goal of this description is not to cover how split and coalesce
 * paradigm is implemented (See Paul Wilson's position paper
 * "Dynamic Storage Allocation: Survey and Critical Review"). 
 *
 * KLOB is based upon the split & coalesce paradigm, wherein all free
 * blocks are placed into slots. Slots are analogous to "bins"
 * in Doug Lea allocator as defined in dlmalloc, GNU C malloc.
 * In dlmalloc, bins are defined for incremental as well as power sizes.
 * A digital tree is used to search for appropriate bins. DL allocator is
 * available at: http://g.oswego.edu/dl/html/malloc.html and is released
 * to the public domain by the public domain license as explained at
 * http://creativecommons.org/licenses/publicdomain
 *
 * The concept of slotizing borrows from an n-way cache. In an n-way cache
 * bits in an address are used to identify a cache line, with evictions of
 * a least recently used cache line in the cache storage.
 * Now assuming "n" was variable, we would not need evictions.
 * In KLOB, (instead of address being  the key), let us use a requested
 * block size as a key with similar  hashing using bit shifting to
 * identify a slot.
 *
 * KLOB arranges free blocks in a free list segregated into x-way slots,
 * where each slot contains all blocks that have the "same size
 * characteristics", e.g. all sizes within a range. Here x, implies that a
 * slot may have a variable number of blocks. A slot may have x=0 blocks.
 *
 * The concept of segregating free list is described in Paul Wilson's
 * position paper. Next, if we link each slot's free list together, we
 * would get a free list sorted by the slot's size characteristics. The
 * size characteristics of a slot may be defined as a single size value or
 * sizes belonging to a range.
 *
 * KLOB Design:
 * Slots are defined for all block sizes that are a multiple of a
 * slot-unit (say 16). So to support block sizes upto 2K, we would need a
 * total of 128 slots. A 128bit bitmap suffices to identify whether a slot is
 * empty or not. These slots are referred to as "incremental" slots, as the
 * block size for these 128 slot-sequence is defined by an increment of
 * slot-unit.
 *
 * In addition to Incremental Slot Sequence, a Power Slot Sequence is defined.
 * Power slots are defined by a base power of 2. So continuing from the
 * last incremental slot (2048), we could have power slots wherein
 * 2K, 4K, 8K, 16K, 32K, ... etc, defines the slots base range. The end
 * range for a power slots is (one less than the base of the next
 * power slot in the sequence.
 *
 * As the last power slot does not have a "next" slot, its end range is
 * the maximum block size.
 *
 * Incremental Slot Sequence: (where slot unit = 16)
 *	i1<0..15>, i2<16..31>, i3<32..47>, i4<48..63> ... i128<2032..2047>
 * Power Slot Sequence:
 *	p1<2048..4095>, p2<4096..8191>, ... p6<128K.. MaxBlkSize>
 *
 * In SLAB, the free list is managed in a sorted order by address. This
 * requires a walk of the free list to determine whether adjacent blocks are
 * free. By including pointers to adjacent blocks in a blocks header, the
 * need to traverse the free list for coalescing can be avoided. However, this
 * introduces a memory overhead. The boundary tag solution (see Paul Wilson's
 * position survey paper) could be used to implement adjacent free blocks, as
 * used in dlmalloc.
 *
 * KLOB was designed with 16-byte alignment requirement, hence a 16byte
 * header serves well to implement a double linked list of all blocks and 
 * a double linked list of all free blocks.
 *
 * Free List Management:
 * The free list of blocks is managed sorted by slot ranges, and a slot would
 * point into the free list where all blocks belonging to its range begins.
 * Adjacent blocks in the free list belonging to a slot need not be
 * sorted by block size. Empty slots do not point into the free list.
 * An array of slot contexts is maintained for the incremental and power
 * slot sequence.
 *
 * See CONFIG_KLOB_ISLOT_SFLIST:
 * The free lists of I-Slots are segregated for easy insertion and deletion.
 *
 * Block List Management:
 * Every block, allocated or free, is prepended by a 16-byte header. The
 * header is used to link all blocks sorted by their addresses, to form the
 * block list. A bit in the header is used to tag whether the block is
 * free or allocated, similar to boundary tag.
 * PS. Three more bits are available for tagging blocks in implementing debug
 * tools or future enhancements of KLOB.
 *
 * Allocation Algorithm:
 * Given a requested size, determine a candidate slot and allocate the first
 * block in the slot. For sizes in the incremental slot range, the size value
 * is shifted right by 4 (for slot unit = 16) the incremntal slot array is
 * directly indexed to fetch the slot context. If this slot's pointer is NULL
 * then the incremental slot bit map is searched to find the next non empty
 * slot. If the slot's pointer is not null, then the first block in the
 * free list is simply removed and allocated to the user.
 *
 * In the case of non-empty I-Slots, the free lists are segregated. 
 * In the case of P-Slots, each non-empty P-Slot's free list is linked to
 * the next P-Slot's free list. Search for the tail of a P-Slot's free list
 * requires to search for the next non-empty P-Slot.
 *
 * Searching for the next not empty slot is performed using word access
 * and MIPS assembly instructions cl0 clz. When found, the first block
 * pointed to by the found slot context is deemed the candidate block. It is
 * removed from the free list. As the block in this found slot is larger
 * than the requested size, the block is split from the front. The left
 * over part is then re-inserted into a corresponding slot determined by
 * the length of the leftover block.
 *
 * A design decision to insert the block at the tail of the slot's free list
 * would make the leftover block available for coalescing in case the block
 * is freed quickly, thereby decreasing fragmentation. The cost of searching
 * the tail of the slot's free list is the tradeoff for low fragmentation.
 *
 * When a block is deleted from a slot, the slot is marked empty if there
 * are no more blocks in the slot's free list.
 *
 * Deallocation Algorithm:
 * When a block is freed, its header is used to identify the previous and
 * next "adjacent" neighbours. If either of the adjacent block is free, then
 * it is first removed from the slot/free list , coalesced with the current
 * block and the coalesced block is reinserted into the appropriate slot.
 *
 * Insertion:
 * On insertion, if the corresponding slot was not empty then the block is
 * simply inserted into the free list pointed to by the slot. If the slot
 * was empty then the slot is tagged as not empty. In the case of an I-Slot,
 * the free block is simply inserted at the tail. For P-Slots, the insertion
 * point is located by determining the next non-empty slot.
 * When a block is freed (and coalesced if necessary), a design decision to
 * insert at the front of a slot's free list is taken to facilitate better
 * cache performance on a likely next allocation.
 *
 *
 * SLAB and KLOB coexistence:
 * kmalloc and kfree interface are implemented using the KLOB allocator. This
 * allows for the performance gains of SLAB kernel caches to be retained.
 * A pre-configured amount of memory is assigned to KLOB for management.
 * In the event that the KLOB managed memory is depleted or fragmented, then
 * KLOB defaults to the "sizes cache" in SLAB.
 *
 *
 * KLOB Engineering:
 * Start with a large initial configuration memory for KLOB. Determine the 
 * steady state threshold for memory usage. Then engineer KLOB to be at or just
 * below this threshold. If engineered at below, then all allocation and free
 * requests would overflow into SLAB benefitting from the performance of
 * SLAB. Hence KLOB would only serve in reducing the memory wastage in bringing
 * the system to steady state.
 *
 * KLOB resides on the Buddy Allocator, which is capable of allocating
 * contiguous memory as page orders i.e. [ 4kPageSize * (1 << pageorder) ].
 * This limitation of granularity of powers-of-2 multiples of PageSize, is
 * addressed in a simplistic Pool extension of KLOB, wherein KLOB starts with
 * say 1M of memory and then extends itself in chunks upto a fixed number
 * of extensions.
 *
 * The address of a block is used to determine whether a block was allocated
 * from KLOB or SLAB. KLOB is the front end to kmalloc/kfree, and the
 * SLAB allocator need not be KLOB aware.
 *
 * CAUTION: A block allocated using kmalloc, cannot be freed using
 * an explicit call to kmem_cache_free() (read yikes).
 * A review of kmem_cache_init() step 4, and setup_cpu_cache() in slab.c
 * However, it is ok to kfree a block allocated via kmem_cache_alloc.
 *
 *
 * Implementation Notes:
 *	Inline functions with "__" prefix are performance critical.
 *	Implementation uses inline functions instead of #define macros to
 *	avail of type checking at compile time.
 *
 * Implementation Constraints:
 *	CONFIG_KLOB_ALIGN: Tested for 16 byte alignment.
 *	- if need 8 byte alignment ensure that all users do not assume
 *	  memory is allocated on cache line, i.e. 16 byte alignment.
 *
 *	KLOB_PSLOT_NUM and KLOB_PSLOT_LZO values dependancy.
 *
 *******************************************************************************
 */

/*
 *------------------------------------------------------------------------------
 *
 * Compile as standalone application
 *
 *	gcc -Wall -DKLOB_TEST_APPL kmalloc_trace.c klob.c
 *	gcc -Wall -DKLOB_TEST_APPL -DKLOB_DEBUG_APPL kmalloc_trace.c klob.c
 *
 * kmalloc_trace.c: externs void testklob();
 * kmalloc_trace.c: void testklob(){ ... sequence of kmalloc and kfrees ... }
 *
 *
 * Down calls to the page allocator and forwarding calls to SLAB allocator
 * are serviced by malloc() when KLOB is compiled as an application.
 *
 * As a standalone application, kmalloc() and kfree() implementations of KLOB
 * are not re-entrant as there is no mutual exclusion implemented for the
 * access to the KLOB state to include manipulation of statistics, block List,
 * free List and bitmaps.
 *
 * The goal of a deriving a KLOB application is to be able to analyze the
 * impacts of various design heuristics on fragmentation using an profile
 * of kmalloc and kfree sequence collected from a target on which KLOB is
 * to be deployed. Engineering of KLOB can also be performed offshelf given
 * a kmalloc/kfree test profile.
 *
 *------------------------------------------------------------------------------
 */

 /*
  * Version 1: original 
  * Version 2: I-Slot free lists are not linked and completely segregated
  *		Performance of inserts at tail is better.
  *     2.1 Deferred extension if in_atomic(), release lock during extension
  */
#define KLOB_VERSION		2


#ifdef CONFIG_SMP
#error "KLOB not compliant with SMP"
!!! FAIL COMPILATION HERE !!!
#endif


/*
 * Development of KLOB as an application on an **IX based OS.
 */
#ifdef KLOB_TEST_APPL

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 *------------------------------------------------------------------------------
 * Exhaustive audit with verbose display capability
 * to be only used when klob is tested as a standalone application
 *
 * Designer Note: KLOB_DEBUG_APPL may be used for host workstation testing.
 *------------------------------------------------------------------------------
 */
#define KLOB_AUDIT
#define KLOB_STATS
#define KLOB_DEBUG_APPL

/* Fake target Linux specific implementation, in host compiles */
#define __init
#define EXPORT_SYMBOL(sym)
#define printk printf
#define local_irq_save(flags)		flags=1;
#define local_irq_restore(flags)	flags=0;
#define kmalloc_account(a,b,c)
#define kfree_account(a,b)
#define KTEST(body)			do { body; } while(0);

unsigned int		kmalloc_count = 0;
unsigned int		kfree_count = 0;
extern unsigned int	klobappl_dump_blist(unsigned int verbose);
extern unsigned int	klobappl_dump_flist(unsigned int verbose);
extern void		klobappl_show(unsigned int verbose);

/* Forwarding downcalls to STDLIB */
void *			__kmalloc(size_t size, int g) {return malloc(size);}
void			__kfree(const void * mem_p) { free((void*)mem_p); }
unsigned int		__ksize(const void * mem_p) { return 0; }
unsigned int            in_atomic() { return 0; }

#else	/* ! defined( KLOB_TEST_APPL ) */

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/cache.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>

#define KTEST(body)

/* #define KLOB_STATS */
/*
 * Target LAB Testing ONLY as AUDITs involve block list and free list walks.
 * with Kernel IRQs disabled.
 *
 * #define KLOB_AUDIT
 *
 */
#define KLOB_AUDIT

/* Forwarding downcalls to default SLAB Allocator in Linux */
extern void *		__kmalloc(size_t size, int gfp);
extern void		__kfree(const void * mem_p);
extern unsigned int	__ksize(const void * mem_p);

#endif	/* else ! defined(KLOB_TEST_APPL) */



	/* Extended statistics collection for Host and Target */
#ifdef KLOB_STATS
#define KSTATS( body ) do { body; } while(0);
#else	/* ! defined(KLOB_STATS) */
#define KSTATS( body )
#endif	/* else ! defined(KLOB_STATS) */



#ifdef KLOB_DEBUG_APPL
unsigned int klob_dbg = 0;	/* Turn on and off debugging at runtime */
#define KDBG0(lvl,fmt)		if (klob_dbg>=lvl) printk("+++" fmt);
#define KDBG1(lvl,fmt,a1)	if (klob_dbg>=lvl) printk("+++" fmt,a1);
#define KDBG2(lvl,fmt,a1,a2)	if (klob_dbg>=lvl) printk("+++" fmt,a1,a2);
#define KDBG3(lvl,fmt,a1,a2,a3)	if (klob_dbg>=lvl) printk("+++" fmt,a1,a2,a3);
#define KDBGB(body)		do { body; } while(0);
#else	/* ! defined(KLOB_DEBUG_APPL) */
#define KDBG0(lvl,fmt)
#define KDBG1(lvl,fmt,a1)
#define KDBG2(lvl,fmt,a1,a2)
#define KDBG3(lvl,fmt,a1,a2,a3)
#define KDBGB(body)
#endif	/* else ! defined(KLOB_DEBUG_APPL) */


/*
 *------------------------------------------------------------------------------
 *
 *  The amount of memory managed by KLOB is restricted by the MAX_ORDER
 *  As the memory is allocated from the buddy (PAGE allocator), the memory size
 *  must be specified as a pageorder multiple of pages size, where,
 *      page size is 4096, and
 *      number of contiguous pages is ( 1 << pageorder) or (2^pageorder ).
 *
 *  For   2M, pageorder 9 = 2^9 = 512, i.e. 4096 * 512 = 2 MBytes
 *  For   1M, pageorder 8 = 2^8 = 256, i.e. 4096 * 256 = 1 MBytes
 *  For 512K, pageorder 7 = 2^7 = 128, i.e. 4096 * 128 = 512 KBytes
 *  For 256K, pageorder 6 = 2^6 =  64, i.e. 4096 *  64 = 256 KBytes
 *  For 128K, pageorder 5 = 2^5 =  32, i.e. 4096 *  32 = 128 KBytes
 *  For  64K, pageorder 4 = 2^4 =  16, i.e. 4096 *  16 =  64 KBytes
 *  For  32K, pageorder 3 = 2^3 =   8, i.e. 4096 *   8 =  32 KBytes
 *  For  16K, pageorder 2 = 2^2 =   4, i.e. 4096 *   4 =  16 KBytes
 *  For   8K, pageorder 1 = 2^1 =   2, i.e. 4096 *   2 =   8 KBytes
 * 
 * Default values:
 *  Initial memory reserved by Klob is CONFIG_KLOB_MEM_SIZE = 1 Mbytes
 *  On depletion,
 *     Klob will extend itself a maximum of CONFIG_KLOB_MAX_EXTN = 8 times,
 *     in chunks of CONFIG_KLOB_MEM_EXTN size = 128Kbytes
 *
 *
 * Limit Extension when the memory is fragmented.
 *  CONFIG_KLOB_LMT_EXTN defines the threshold for allowable fragmentation
 *  before an extension is admissable.
 *------------------------------------------------------------------------------
 */

#define CONFIG_KLOB_MEM_SIZE		(4096 * 256)	/*   1 MBytes */
#define CONFIG_KLOB_MEM_EXTN		(4096 *  32)	/* 128 KBytes */
#define CONFIG_KLOB_LMT_EXTN		(4096 *  16)	/*  64 KBytes */
#define CONFIG_KLOB_MAX_EXTN		8

/*
 * Free Lists are dis-continuous (segregated) for I-Slots.
 * Insertion (e.g. at tail) of an I-Slot does not require a search of the next
 * non-empty slot.
 */
#define CONFIG_KLOB_ISLOT_SFLIST

/* KLOB Implementation supported for alignment 8 or alignment 16 */
#define CONFIG_KLOB_ALIGN	16

#define KLOB_FREE_MARK		0x1
#define KLOB_FREE_MASK		(~ KLOB_FREE_MARK)
#define KLOB_CLEAR_TAG		KLOB_FREE_MASK

/*
 *------------------------------------------------------------------------------
 *
 * Double Linked List of Blocks.
 *
 * All blocks (used or free) are maintained in double linked list sorted by the
 * block address for block list traversal during audits or coalescing on freies.
 * During free, to facilitate coalescing, the previous and next negighbours of a
 * block can be quickly located at a tradeoff of 8 bytes per allocated block.
 * Note that, one bit in the previous pointer is used to determine whether a
 * block is free or in use.
 *
 *-----------------------------------------------------------------------------
 */

typedef struct bhdr {		/* Assuming size = 8 */
	struct bhdr	* next_p;
	struct bhdr	* prev_p;	/* CAUTION: Overlay USED/FREE tag bit */
} bhdr_t;

/*
 *------------------------------------------------------------------------------
 *
 * Double Linked List of all Free Blocks
 *
 * All freed blocks are linked together to implement a free list. An insertion
 * algorithm is used to ensure that all blocks are sorted by their lengths.
 *
 *------------------------------------------------------------------------------
 */

typedef struct fhdr {		/* Assuming size == 8 */
	struct fhdr	* next_p;
	struct fhdr	* prev_p;
} fhdr_t;

/*
 *------------------------------------------------------------------------------
 *
 * klob free block layout:
 *
 * When the alignment is equal to the size of a bhdr, fhdr is
 * overlayed on the user part of a free block.
 * The klob block header and the user part of a block are in contiguous memory.
 * A freed block must never be accessed by the user.
 *
 *------------------------------------------------------------------------------
 */

typedef struct khdr {
	bhdr_t		bhdr;
	fhdr_t		fhdr;
} khdr_t;

/*
 *------------------------------------------------------------------------------
 *
 *  KLOB Memory Pool Extension Support
 *
 *------------------------------------------------------------------------------
 */

typedef struct kpool {
	void		* pbgn_p;
	void		* pend_p;
	khdr_t		* hkhdr_p;
	khdr_t		* tkhdr_p;
} kpool_t;


/*
 *------------------------------------------------------------------------------
 *
 *			Klob Header Manipulation Utilities.
 *
 *------------------------------------------------------------------------------
 */

/* Given a pointer to a klob k-hdr, get the pointer to the b-hdr */
static inline bhdr_t * __khdr_to_bhdr(const khdr_t * khdr_p)
{
    return ((bhdr_t*)khdr_p);	/* Simple C-style cast */
}
/* Given a pointer to a klob b-hdr, get the pointer to the k-hdr */
static inline khdr_t * __bhdr_to_khdr(const bhdr_t * bhdr_p)
{
    return ((khdr_t*)bhdr_p);	/* Simple C-style cast */
}


/* Given a pointer to a klob k-hdr, get the pointer to the f-hdr */
static inline fhdr_t * __khdr_to_fhdr(const khdr_t * khdr_p)
{
    return ((fhdr_t*)(&khdr_p->fhdr));
}
/* Given a pointer to a klob b-hdr, get the pointer to the f-hdr */
static inline fhdr_t * __bhdr_to_fhdr(const bhdr_t * bhdr_p)
{
    return __khdr_to_fhdr(__bhdr_to_khdr(bhdr_p));
}

/* Given a pointer to a klob f-hdr, get the pointer to the k-hdr */
static inline khdr_t * __fhdr_to_khdr(const fhdr_t * fhdr_p)
{
    return ((khdr_t *)(((unsigned int)fhdr_p) - sizeof(bhdr_t)));
}
/* Given a pointer to a klob f-hdr, get the pointer to the b-hdr */
static inline bhdr_t * __fhdr_to_bhdr(const fhdr_t * fhdr_p)
{
    return __khdr_to_bhdr(__fhdr_to_khdr(fhdr_p));
}


/*
 * Size of a header attached to a memory block
 * depends upon CONFIG_KLOB_ALIGN
 */
static inline unsigned int __mem_hdr_size(void)
{
    return (sizeof(bhdr_t)
#if (CONFIG_KLOB_ALIGN > 8)
            + (CONFIG_KLOB_ALIGN - sizeof(bhdr_t))
#endif
           );
}

/* Given a pointer to a klob header, get the pointer to the user block */
static inline void * __khdr_to_mem(const khdr_t * khdr_p)
{
    return ((void *)(((unsigned int)khdr_p) + __mem_hdr_size()));
}

/* Tag a klob block as "free", by setting the free bit */
static inline void __khdr_tag_free(khdr_t * khdr_p)
{
    khdr_p->bhdr.prev_p = (bhdr_t*)
      ((unsigned int)(khdr_p->bhdr.prev_p) | KLOB_FREE_MARK);
}

/* Tag a klob block as "in-use", by clearing the free bit */
static inline void __khdr_tag_used(khdr_t * khdr_p)
{
    khdr_p->bhdr.prev_p = (bhdr_t*)
      ((unsigned int)(khdr_p->bhdr.prev_p) & KLOB_FREE_MASK);
}

/* Check whether a klob block is free */
static inline int __khdr_is_free(const khdr_t * khdr_p)
{
    return (((unsigned int)(khdr_p->bhdr.prev_p)) & KLOB_FREE_MARK);
}

static inline khdr_t * __khdr_next(const khdr_t * khdr_p)
{
    return __bhdr_to_khdr((khdr_p->bhdr.next_p));
}

static inline khdr_t * __khdr_prev(const khdr_t * khdr_p)
{
    return ((khdr_t*)
            ((unsigned int)(khdr_p->bhdr.prev_p) & KLOB_CLEAR_TAG));
}

static inline void __khdr_set_next(khdr_t * khdr_p, const khdr_t * next_khdr_p)
{
    khdr_p->bhdr.next_p = __khdr_to_bhdr(next_khdr_p);
}

static inline void __khdr_set_prev(khdr_t * khdr_p, const khdr_t * prev_khdr_p)
{
    if ( __khdr_is_free(khdr_p) )
    {
        khdr_p->bhdr.prev_p = __khdr_to_bhdr(prev_khdr_p);
        __khdr_tag_free(khdr_p);	/* CAUTION: Overlay USED/FREE tag bit */
    }
    else
    {
        khdr_p->bhdr.prev_p = __khdr_to_bhdr(prev_khdr_p);
    }
}


/* Size of the entire klob block, including free hdr  */
static unsigned int __khdr_size(const khdr_t * khdr_p)
{
    return (((unsigned int)(__khdr_next(khdr_p))) - ((unsigned int)khdr_p));
}

/* Size of user part of a klob block */
static unsigned int __khdr_mem_size(const khdr_t * khdr_p)
{
    return (__khdr_size(khdr_p) - __mem_hdr_size());
}

/*
 *------------------------------------------------------------------------------
 *
 *			Memory Block Manipulation Utilities
 *
 *------------------------------------------------------------------------------
 */

/* Given a pointer to a user block, get the pointer to it's klob header */
static inline khdr_t * __mem_to_khdr(const void * mem_p)
{
    return ((khdr_t*)(((unsigned int)mem_p) - __mem_hdr_size()));
}

/* Tag a block as "free", by setting the free bit */
static inline void __mem_tag_free(const void * mem_p)
{
    __khdr_tag_free(__mem_to_khdr(mem_p));
}

/* Tag a block as "in-use", by clearing the free bit */
static inline void __mem_tag_used(const void * mem_p)
{
    __khdr_tag_used(__mem_to_khdr(mem_p));
}

static inline int __mem_is_free(const void * mem_p)
{
    return __khdr_is_free(__mem_to_khdr(mem_p));
}

/* Size of user part given a block pointer */
/* CAUTION: Only if mem_p is a valid KLOB block */
static inline unsigned int __mem_size(const void * mem_p)
{
    return __khdr_mem_size(__mem_to_khdr(mem_p));
}

/* Round up a size to CONFIG_KLOB_ALIGN */
static inline unsigned int __klob_roundup(unsigned int size)
{
    return ( (size + CONFIG_KLOB_ALIGN - 1) & ~(CONFIG_KLOB_ALIGN - 1) );
}

/*
 *------------------------------------------------------------------------------
 *
 *			Incremental Slot Sequence Constants:
 *
 * KLOB_ISLOT_OFF : The offset of the ranges of each slot in sequence
 *
 * KLOB_ISLOT_SFT : Shift bits corresponding to offset
 *
 * KLOB_ISLOT_LMT : Limit of memory size supported by the last incremental slot
 *	The Power Slot Sequence begins from KLOB_ISLOT_LMT
 *
 * KLOB_ISLOT_NUM : Total number of Incremental Slots in Sequence to support
 *		upto KLOB_ISLOT_LMT block size.
 *
 * KLOB_ISLOT_MAP : Number of 32bit words required to map KLOB_ISLOT_NUM
 *		number of Incremental Slots.
 *
 * KLOB_ISLOT_IFG : Number of slots to skip while searching for the next
 *		non-empty slot to avoid an internal fragmentation or
 *		a split resulting into a block that could not be allocated.
 *
 * Designer Note:
 * The current implementation has been defined for 8 and 16 byte alignments
 * see. #define CONFIG_KLOB_ALIGN
 *
 * For alignment 8: I-Slot Sequence: <0,7> <8,15> <16,23>...<2040,2047>
 * Total number of slots in sequence is : 256.
 * A bitmap of 8 words (i.e. 256 bits) is used to track the segregated segments
 * of the global free list containing all blocks belonging to a slot.
 *
 * For alignment 16: I-Slot Sequence: <0,15> <16,31> <32,47>...<2032,2047>
 * Total number of slots in sequence is : 128.
 * A bitmap of 4 words (i.e. 128 bits) is used to track the segregated segments
 * of the global free list containing all blocks belonging to a slot.
 *
 * Note: C-Implementation: 1st bit is bitix=0, 32nd bit is bitix=31.
 *       The 3rd bit (i.e. bitix=2) represents 3rd slot, (i.e. slotix=2),
 *
 *------------------------------------------------------------------------------
 */

#define KLOB_BITS32	32
#define KLOB_DIV32(ix)	((ix) >> 5)
#define KLOB_MOD32(ix)	((ix) & 31UL)
#define KLOB_MUL32(ix)	((ix) << 5)

#define KLOB_ISLOT_OFF	CONFIG_KLOB_ALIGN

/*
 * Not very elegant code follows ... excuse me.
 */
#if (KLOB_ISLOT_OFF == 8)	/* 8 word bitmap */
#define KLOB_ISLOT_SFT	3
#elif (KLOB_ISLOT_OFF == 16)	/* 4 word bitmap */
#define KLOB_ISLOT_SFT	4
#elif (KLOB_ISLOT_OFF == 32)	/* 2 word bitmap */
#define KLOB_ISLOT_SFT	5
#elif (KLOB_ISLOT_OFF == 64)	/* 1 word bitmap */
#define KLOB_ISLOT_SFT	6
#else
#error "KLOB : Unsupported alignment"
#endif

#define KLOB_ISLOT_LMT	2048
#define KLOB_ISLOT_NUM	(KLOB_ISLOT_LMT/KLOB_ISLOT_OFF)
#define KLOB_ISLOT_MAP	(KLOB_ISLOT_NUM/KLOB_BITS32)
#define KLOB_ISLOT_IFG	1

/*
 *------------------------------------------------------------------------------
 *
 * Definitions for P-Slot Sequence are constrained by implementation !!!
 *
 * In powers of 2, larger than 2k.
 *
 *  Total of 8 P-Slots in Sequence.
 *	P-Slot Ix=0 [  2k ...   4k)
 *	P-Slot Ix=1 [  4k ...   8k)
 *	P-Slot Ix=2 [  8k ...  16k)
 *	P-Slot Ix=3 [ 16k ...  32k)
 *	P-Slot Ix=4 [ 32k ...  64k)
 *	P-Slot Ix=5 [ 64k ... 128k)
 *	P-Slot Ix=6 [128k ... 256k)
 *	P-Slot Ix=7 [256k ...  max)
 *
 *------------------------------------------------------------------------------
 */

#define KLOB_PSLOT_NUM	8
#if (KLOB_PSLOT_NUM != 8)
#error "KLOB: Implementation constraint for KLOB_PSLOT_NUM"
!!! FAIL COMPILATION HERE !!!
#endif

#if (KLOB_ISLOT_LMT == 2048)
#define KLOB_PSLOT_LZO	20	/* P-Slot offset 2048 has 20 leading zeros */
#else
#error "KLOB: Implementation constraint for KLOB_PSLOT_LZO"
#endif

#define KLOB_PSLOT_TRY	8	/* Search at most 8 blocks for best fit */


/*
 *------------------------------------------------------------------------------
 *				KLOB Context
 *------------------------------------------------------------------------------
 */

#ifdef KLOB_DEBUG_APPL
typedef struct klob_slot_ctx {
	unsigned int	rbgn;		/* Range begin block size */
	unsigned int	rend;		/* Range end block size */
	unsigned long	cnt;		/* Number of free blocks in slot */
	unsigned long	hwm;		/* High water mark of slot size */
	unsigned long	del;		/* Cummulative deletions from slot */
	unsigned long	ins;		/* Cummulative insertions into slot */
} klob_slot_ctx_t;
#endif

typedef struct klob_ctx {

	bhdr_t	blist;		/* List of allocated|free blocks */
	fhdr_t	flist;		/* List of free blocks */

	/*
	 * I-Slot Sequence multi word bit map to denote
	 * empty [bit balue=0] or nonempty slot [bit value=1]
	 * An extra 32 bits demarcation is apprended at the end for
	 * faster bit search algorithms. The demarcation bits are always slot.
	 */
	unsigned long	islot_map[KLOB_ISLOT_MAP + 1];

	/*
	 * P-Slot Sequence bit map. As only 8 bits are needed, the
	 * rest of the 24 bits in the 32bit map are used as demarcation
	 * bits for fast search algorithms.
	 */
	unsigned long	pslot_map;
	unsigned int	numpools;

#ifdef CONFIG_KLOB_ISLOT_SFLIST
	/*
	 * Segregated Free List for I-Slots, identifying head and tail.
	 */
	fhdr_t    islot_flist[KLOB_ISLOT_NUM + KLOB_BITS32];
#else
	/*
	 * Pointers into free list to identify the start of blocks belonging
	 * to each (Incremental and Power) Slot. Note that only the start
	 * of each slot's free list is maintained. To locate the end, the bit
	 * map will be used to determine the next non-empty slot, whose start
	 * pointer denotes the end pointer for a given slot.
	 */
	fhdr_t	* islot_flp[KLOB_ISLOT_NUM + KLOB_BITS32];
#endif

	/* Total of 32 PLOT free list pointers are defined, although there
	 * are only KLOB_PSLOT_NUM P-Slots.
	 * The remaining (32 - 8 = 24 entries) serve as demarcation, and their
	 * free list pointer points to end of the global free list.
	 */
	fhdr_t	* pslot_flp[KLOB_BITS32];


	/*
	 * Information of the memory region managed by KLOB.
	 *
	 * When KLOB memory is depleted, all allocation requests are
	 * forwarded to the default memory system (i.e. SLAB). Hence
	 * when a block is freed, KLOB needs to quickly detrmine
	 * whether the block was obtained from it's own memory region
	 * or from the overflow SLAB managed memory region.
	 *
	 * See __is_mem_klob(void * mem_p)
	 *
	 * Support for KLOB memory extension would require a slower
	 * __is_mem_klob() implementation.
	 */

	unsigned long		extend;
	kpool_t			pool[CONFIG_KLOB_MAX_EXTN+1];
	unsigned long		memory_sz;
	unsigned long		free_sz;

#ifdef KLOB_DEBUG_APPL
	/*
	 * Debug information per slot.
	 */
	klob_slot_ctx_t		islot_ctx[KLOB_ISLOT_NUM];
	klob_slot_ctx_t		pslot_ctx[KLOB_ISLOT_NUM];
#endif

#ifdef KLOB_STATS
	unsigned long failexts;		/* Number of disallowed extensions */

        unsigned long bcnt;		/* Count of blocks in block list */
        unsigned long icnt;		/* Count of blocks in I-Slot list */
        unsigned long pcnt;		/* Count of blocks in P-Slot list */
        unsigned long fhwm;		/* High watermark of free list size */

	/* Cummulative counts for KLOB */
	unsigned long long frees;	/* Count of klob_frees */
	unsigned long long iallocs;	/* Count of klob_allocs from I-Slot */
	unsigned long long pallocs;	/* Count of klob_allocs from P-Slot */
	unsigned long long bytes;	/* Count of bytes allocated */

	unsigned long long splits;	/* Cummulative splits */
	unsigned long long coalesces;	/* Cummulative coalesces */

	/* Cummulative counts for KLOB Overflow into secondary allocator */
	unsigned long fwd_frees;	/* Count of forwarded frees */
	unsigned long fwd_allocs;	/* Count of forwarded allocs */
	unsigned long long fwd_bytes;	/* Count of overflow bytes allocated */
#endif

} klob_ctx_t;

/* Forward declaration */
void * klob_extend(size_t mem_sz, int gfp);

/*
 * KLOB Memory Manager Global Context.
 */
klob_ctx_t	klob_g;

/*
 *------------------------------------------------------------------------------
 * Determine whether a block of memory is a KLOB block.
 *------------------------------------------------------------------------------
 */
static inline int __is_mem_klob(const void * mem_p)
{
    unsigned int p = klob_g.numpools;

    while( p )	/* we do not use pool[0] */
    {
	/* Can we have a single test here based upon the memory layout */
        if ((mem_p < klob_g.pool[p].pend_p) && (mem_p > klob_g.pool[p].pbgn_p))
           return 1;
        p--;
    }

    return 0;
}

static inline int __is_khdr(const khdr_t * khdr_p)
{
    unsigned int p = klob_g.numpools;

    while( p )	/* we do not use pool[0] */
    {
        /* Can we have a single test here based upon the memory layout */
        if (((void*)khdr_p < klob_g.pool[p].pend_p)
            && ((void*)khdr_p >= klob_g.pool[p].pbgn_p))
            return 1;
         p--;
    }

    return 0;
}

static int __is_kpool(const khdr_t * khdr_p)
{
    unsigned int p = klob_g.numpools;

    while( p )
    {
        if (khdr_p == klob_g.pool[p].hkhdr_p)
            return 1;
        if (khdr_p == klob_g.pool[p].tkhdr_p)
            return 1;
        p--;
    }

    return 0;
}



/*
 *------------------------------------------------------------------------------
 *			Generic Bit Manipulation Routines
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 * Count Leading Zeros in a 32-bit bitmap
 *------------------------------------------------------------------------------
 */
static inline int __count_leading_zeros(unsigned long bitmap)
{
#ifdef KLOB_TEST_APPL
    int shifts = 0;
    while (bitmap) { shifts++; bitmap >>= 1; }
    return (32-shifts);
#else
    int zeros;
	/* Count leading (higher order) zeros */
    __asm__ volatile (
                "clz    %0, %1          \n"
                : "=r" (zeros)
                : "r"  (bitmap));
    return zeros;
#endif
}

/*
 *------------------------------------------------------------------------------
 *
 * Find Trailing Set Bit in a 32bit bitmap
 *
 * Differences between ffs(), flz(), ...
 *
 * ffs() in string.h, and flz() in bitops.h are in-efficient in comparison to
 * _find_trailing_set_bit() which uses the native processor's instruction set
 *
 * CAUTION:
 *   _find_trailing_set_bit() uses bit positions 0..31 as opposed to 1..32
 *   A -ve return value denotes no set bit, as opposed to 0
 *
 * #include <string.h>
 * extern int ffs(int __i);
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 *
 * Find Trailing Set Bit in a 32bit bitmap
 *
 * Return the position of the trailing set bit in I, or -ve if none are set.
 * The least-significant bit is position 0, the most-significant is 31
 *
 *  E.g.                         Higher order   -------  Lower order
 *  for a bitmap 0x0F0FF080 = 0b 00001111 00001111 11110000 10000000
 *                               ^                          ^      ^
 *                              31                          7      0
 *      7 = _find_trailing_set_bit(0x0F0FF080)
 *     -1 = _find_trailing_set_bit 0x0)
 *
 * PS. In the case of ffs() from string.h, the value returned is 1 .. 32.
 *     where a returned value 0 implies that no bit is set.
 *
 * Using MIPS assembly instruction clz, we need 5 compute shadow instructions
 *
 *------------------------------------------------------------------------------
 */
static inline int _find_trailing_set_bit(unsigned long bitmap)
{
    int zeros;

	/* Clear all ones except for the trailing "lowest order" 1 */
    bitmap = (bitmap ^ (bitmap - 1)) & bitmap;
	/* bitmap would now have a single 1 bit and the rest all 0s */

    zeros = __count_leading_zeros(bitmap);
    return 31 - zeros;	/* excluding the 1, trailing zeros */
}

/*
 *------------------------------------------------------------------------------
 * Find Trailing Set Bit in a 32bit bitmap using float.
 * Return the position of the trailing set bit in I, or -ve if none are set.
 * The least-significant bit is position 0, the most-significant is 31
 *
 * MIPS boasts of integer to float point performance,
 * if so, and your board supports FPU, use  __find_trailing_set_bit_wfloat
 * Uses 4 compute shadow instructions.
 *
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 *------------------------------------------------------------------------------
 */
static inline int __find_trailing_set_bit_wfloat(unsigned long bitmap)
{
	/* Cast the least signifiant bit to a float */
    float f = (float)(bitmap & -bitmap);
    return ((*(unsigned int*) &f >> 23) - 0x7f);
}

/*
 *------------------------------------------------------------------------------
 * Count Set Bits in a 32bit bitmap, B. Kernighan's way.
 *------------------------------------------------------------------------------
 */
static inline unsigned int __count_set_bits(unsigned long bitmap)
{
    unsigned int count;
	/* loops as many times as set bits */
    for (count=0; bitmap; count++)
        bitmap &= bitmap - 1;	/* Clear least significant bit */

    return count;
}

/*
 *------------------------------------------------------------------------------
 *
 * Round a 32-bit value to a power of 2 value
 * e.g. 1024 rounds to 1024
 *      1025 rounds to 2048
 *
 * Reference: Bit Twiddling Hacks, Sean Anderson
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __round_to_power2(unsigned long value)
{
    value--;
    value |= value>>1;
    value |= value>>2;
    value |= value>>4;
    value |= value>>8;
    value |= value>>16;
    value++;
    return value;
}

/*
 *------------------------------------------------------------------------------
 *
 * Check whether a value is a power of 2.
 * 0 is included as a power of 2
 *
 * Reference: Bit Twiddling Hacks, Sean Anderson
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __is_power2(unsigned long value)
{
    return ((value & (value-1)) == 0);
}


/*
 *------------------------------------------------------------------------------
 *
 * Compute the log to the base 2 of a given value.
 *
 * Reference: Bit Twiddling Hacks, Sean Anderson
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __log_base2(unsigned long value)
{
    static const int MultiplyDeBruijnBitPosition[32] =
    {
         0,  1, 28,  2, 29, 14, 24,  3, 30, 22, 20, 15, 25, 17,  4,  8,
        31, 27, 13, 23, 21, 19, 16,  7, 26, 12, 18,  6, 11,  5, 10,  9
    };

    value |= value >>  1;
    value |= value >>  2;
    value |= value >>  4;
    value |= value >>  8;
    value |= value >> 16;
    value = (value >> 1) + 1;

    return MultiplyDeBruijnBitPosition[ (value * 0x077CB531UL) >> 27 ];
}

/*
 *------------------------------------------------------------------------------
 *
 * Given a block size in the I-Slot Sequence, fetch the
 * index of the I-Slot.
 *
 * CAUTION: No check is done whether block size belongs to I-Slot range.
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __memsz_to_islot(size_t mem_sz)
{
    return (mem_sz >> KLOB_ISLOT_SFT);
}

/*
 *------------------------------------------------------------------------------
 *
 * Given a block size in the P-Slot Sequence, fetch the
 * index of the P-Slot.
 *
 *  CAUTION: No check is done whether block size belongs to P-Slot range.
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __memsz_to_pslot(size_t mem_sz)
{
    int zeros = __count_leading_zeros(mem_sz);
    zeros = KLOB_PSLOT_LZO - zeros;
    return (zeros > (KLOB_PSLOT_NUM - 1) ? (KLOB_PSLOT_NUM - 1) : zeros);
}

/*
 *------------------------------------------------------------------------------
 *
 * Get a bit in the I-Slot multiword bit map
 *
 * CAUTION: No check is done whether ix is in I-Slot sequence
 * 	i.e. 0 <= ix < KLOB_ISLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned long __get_islot(unsigned int ix)
{
    return (klob_g.islot_map[KLOB_DIV32(ix)] & (1UL << KLOB_MOD32(ix)));
}

/*
 *------------------------------------------------------------------------------
 *
 * Set a bit in the I-Slot multiword bit map
 *
 * CAUTION: No check is done whether ix is in I-Slot sequence
 * 	i.e. 0 <= ix < KLOB_ISLOT_NUM
 *------------------------------------------------------------------------------
 */
static inline void __set_islot(unsigned int ix)
{
    unsigned long * bitmap =
       ((unsigned long *) klob_g.islot_map) + KLOB_DIV32(ix);
    *bitmap |= (1UL << KLOB_MOD32(ix));
}

/*
 *------------------------------------------------------------------------------
 *
 * Clear a bit in the I-Slot multiword bit map
 *
 * CAUTION: No check is done whether ix is in I-Slot sequence
 * 	i.e. 0 <= ix < KLOB_ISLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline void __clear_islot(unsigned int ix)
{
    unsigned long * bitmap =
       ((unsigned long *) klob_g.islot_map) + KLOB_DIV32(ix);
    *bitmap &= ~(1UL << KLOB_MOD32(ix));
}

/*
 *------------------------------------------------------------------------------
 *
 * Toggle a bit in the I-Slot multiword bit map
 *
 * CAUTION: No check is done whether ix is in I-Slot sequence
 * 	i.e. 0 <= ix < KLOB_ISLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline void __toggle_islot(unsigned int ix)
{
    unsigned long * bitmap =
       ((unsigned long *) klob_g.islot_map) + KLOB_DIV32(ix);
    *bitmap ^= (1UL << KLOB_MOD32(ix));
}

/*
 *------------------------------------------------------------------------------
 * Test whether a given I-Slot is non-empty
 *
 * CAUTION: No check is done whether ix is in I-Slot sequence
 *      i.e. 0 <= ix < KLOB_ISLOT_NUM
 *
 *------------------------------------------------------------------------------
 */

static inline unsigned int __isempty_islot_flist(unsigned int ix)
{
#ifdef CONFIG_KLOB_ISLOT_SFLIST
    KDBGB(
        if (__get_islot(ix)
            && ( (klob_g.islot_flist[ix].next_p == &klob_g.islot_flist[ix])
               ||(klob_g.islot_flist[ix].prev_p == &klob_g.islot_flist[ix])))
            printk("KLOB ERROR: I-Slot[%d] set __isempty_islot_flist\n",
               ix);
        if (!__get_islot(ix)
            && ( (klob_g.islot_flist[ix].next_p != &klob_g.islot_flist[ix])
               ||(klob_g.islot_flist[ix].prev_p != &klob_g.islot_flist[ix])))
            printk("KLOB ERROR: I-Slot[%d] clr __isempty_islot_flist\n",
               ix);
    );

    return (klob_g.islot_flist[ix].next_p == &klob_g.islot_flist[ix]);

#else

    KDBGB(
        if (__get_islot(ix) && (klob_g.islot_flp[ix] == NULL))
            printk("KLOB ERROR: I-Slot[%d] inconsistent map, flist pointer\n",
               ix);
    );

    return (klob_g.islot_flp[ix] == NULL);

#endif
}

static inline unsigned int __test_islot(unsigned int ix)
{
    KDBGB(
        if (__get_islot(ix) /* Not empty */
            && ( (klob_g.islot_flist[ix].next_p == &klob_g.islot_flist[ix])
               ||(klob_g.islot_flist[ix].prev_p == &klob_g.islot_flist[ix])))
            printk("KLOB ERROR: I-Slot[%d] set __test_islot\n",
               ix);
        if (!__get_islot(ix) /* Empty */
            && ( (klob_g.islot_flist[ix].next_p != &klob_g.islot_flist[ix])
               ||(klob_g.islot_flist[ix].prev_p != &klob_g.islot_flist[ix])))
            printk("KLOB ERROR: I-Slot[%d] clr __test_islot\n",
               ix);
    );

    return __get_islot(ix);
}

/*
 *------------------------------------------------------------------------------
 *
 * Get a bit in the P-Slot bit map
 *
 * CAUTION: No check is done whether ix is in P-Slot sequence
 * 	i.e. 0 <= ix < KLOB_PSLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned long __get_pslot(unsigned int ix)
{
    return (klob_g.pslot_map & (1UL << KLOB_MOD32(ix)));
}

/*
 *------------------------------------------------------------------------------
 *
 * Set a bit in the P-Slot bit map
 *
 * CAUTION: No check is done whether ix is in P-Slot sequence
 * 	i.e. 0 <= ix < KLOB_PSLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline void __set_pslot(unsigned int ix)
{
    klob_g.pslot_map |= (1UL << KLOB_MOD32(ix));
}

/*
 *------------------------------------------------------------------------------
 *
 * Clear a bit in the P-Slot bit map
 *
 * CAUTION: No check is done whether ix is in P-Slot sequence
 * 	i.e. 0 <= ix < KLOB_PSLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline void __clear_pslot(unsigned int ix)
{
    klob_g.pslot_map &= ~(1UL << KLOB_MOD32(ix));
}

/*
 *------------------------------------------------------------------------------
 *
 * Toggle a bit in the P-Slot bit map
 *
 * CAUTION: No check is done whether ix is in P-Slot sequence
 * 	i.e. 0 <= ix < KLOB_PSLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline void __toggle_pslot(unsigned int ix)
{
    klob_g.pslot_map ^= (1UL << KLOB_MOD32(ix));
}

/*
 *------------------------------------------------------------------------------
 *
 * Test whether a given P-Slot is non-empty
 *
 * CAUTION: No check is done whether ix is in P-Slot sequence
 *      i.e. 0 <= ix < KLOB_PSLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __test_pslot(unsigned int ix)
{
    KDBGB(
        if (__get_pslot(ix) && (klob_g.pslot_flp[ix] == NULL))
            printk("KLOB ERROR: P-Slot[%d]"
                   " inconsistent map and flist pointer\n", ix);
    );
    return __get_pslot(ix);
}

/*
 *------------------------------------------------------------------------------
 *
 * Find the next set bit in the I-Slot multiword bitmap starting from bitix
 * Returns >= KLOB_ISLOT_NUM if no valid I-Slot is found.
 *
 * CAUTION: ix must be a valid index in the multiword bitmap!
 *      No check is done whether ix is in I-Slot sequence
 * 	i.e. 0 <= ix < (KLOB_ISLOT_NUM + 32)
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __find_next_set_islot(unsigned int bitix)
{
    unsigned int mapix;
    unsigned long bitmap;

    mapix = KLOB_DIV32(bitix);
    bitmap = klob_g.islot_map[ mapix ];
    bitmap &= (~0UL << KLOB_MOD32(bitix));
    while (bitmap == 0)
    {
        ++mapix;
        bitmap = klob_g.islot_map[ mapix ];
    }
    return (_find_trailing_set_bit(bitmap) + KLOB_MUL32(mapix));
}

/*
 *------------------------------------------------------------------------------
 *
 * Find the first set bit in the P-Slot bitmap
 *
 * Returns >= KLOB_PSLOT_NUM if no valid P-Slot is found.
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __find_first_set_pslot(void)
{
    return _find_trailing_set_bit(klob_g.pslot_map);
}

/*
 *------------------------------------------------------------------------------
 *
 * Find the next set bit in the P-Slot bitmap
 * Returns >= KLOB_PSLOT_NUM if no valid P-Slot is found.
 *
 * CAUTION: pslotix must be a valid index in the P-Slot bitmap!
 *	No check is done whether ix is in P-Slot sequence
 *      i.e. 0 <= ix < KLOB_PSLOT_NUM
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int __find_next_set_pslot(unsigned int ix)
{
    unsigned long bitmap;

    bitmap = klob_g.pslot_map;
    bitmap &= (~0UL << (ix + 1));
    return _find_trailing_set_bit(bitmap);
}

/*------------------------------------------------------------------------------
 *		Block List and Free List Manipulation
 *----------------------------------------------------------------------------*/

/*
 *------------------------------------------------------------------------------
 *
 * _khdr_insert: Insert a block into an appropriate slot, at head or tail.
 *
 * On insertion, the Slot is tagged not empty if this is the first block.
 * Insertion at head with the slot being previously empty or an insertion
 * at tail requires a search of the first non-empty "next" slot.
 * The block is then inserted prior to the free list pointed to by the found
 * non-empty next slot.
 *
 *------------------------------------------------------------------------------
 */
static void _khdr_insert(khdr_t * khdr_p, unsigned int mem_sz,
                         unsigned int at_head)
{
    unsigned int slot;
    fhdr_t * fhdr_p = __khdr_to_fhdr(khdr_p);
    fhdr_t * at_fhdr_p = (fhdr_t *)NULL;

    klob_g.free_sz += mem_sz;

    if (mem_sz < KLOB_ISLOT_LMT)	/* I-Slot */
    {
		/* Determine I-Slot index */
        slot = __memsz_to_islot(mem_sz);

	KSTATS(
           klob_g.icnt++;/* Insert a free block into an I-Slot free list */
           if ((klob_g.icnt + klob_g.pcnt) > klob_g.fhwm)
               klob_g.fhwm = klob_g.icnt + klob_g.pcnt;
	);

        KDBGB(
            klob_g.islot_ctx[slot].cnt++;
            if (klob_g.islot_ctx[slot].cnt > klob_g.islot_ctx[slot].hwm)
                klob_g.islot_ctx[slot].hwm = klob_g.islot_ctx[slot].cnt;
            klob_g.islot_ctx[slot].ins++;
        );

#ifdef CONFIG_KLOB_ISLOT_SFLIST
        at_fhdr_p = &klob_g.islot_flist[slot];

        if (at_fhdr_p->next_p == at_fhdr_p)
        {
		/* Tag I-Slot as non-empty */
            __set_islot(slot);

            at_fhdr_p->next_p = at_fhdr_p->prev_p = fhdr_p;
            fhdr_p->next_p = fhdr_p->prev_p = at_fhdr_p;
        }
        else
        {
            if (at_head)
            {
                fhdr_p->prev_p = at_fhdr_p;
                fhdr_p->next_p = at_fhdr_p->next_p;
                at_fhdr_p->next_p->prev_p = fhdr_p;
                at_fhdr_p->next_p = fhdr_p;
            }
            else
            {
                fhdr_p->next_p = at_fhdr_p;
                fhdr_p->prev_p = at_fhdr_p->prev_p;
                at_fhdr_p->prev_p->next_p = fhdr_p;
                at_fhdr_p->prev_p = fhdr_p;
            }
        }
#else
		/* Quick insert at head, if slot not empty */
        if (at_head && (klob_g.islot_flp[slot] != (fhdr_t*)NULL))
        {
            at_fhdr_p = klob_g.islot_flp[slot];
            klob_g.islot_flp[slot] = fhdr_p;
        }

        if (klob_g.islot_flp[slot] == (fhdr_t*)NULL)
        {
		/* Tag I-Slot as non-empty */
            klob_g.islot_flp[slot] = fhdr_p;
            __set_islot(slot);
        }

        if (at_fhdr_p == ((fhdr_t*)NULL))
        {
		/* Find insertion point */
            slot = __find_next_set_islot(slot + 1);

		/* Found a non-empty slot in I-Slots */
            if (slot < KLOB_ISLOT_NUM)
            {
                at_fhdr_p = klob_g.islot_flp[slot];
            }
            else
            {
                slot = __find_first_set_pslot();

		/* Found a non-empty slot in P-Slots */
                if (slot < KLOB_PSLOT_NUM)
                    at_fhdr_p = klob_g.pslot_flp[slot];
                else	/* Insert at end of free list */
                    at_fhdr_p = & klob_g.flist;
            }
        }

		/* Insert before "at_khdr_p" */
        khdr_p->fhdr.next_p = at_fhdr_p;
        khdr_p->fhdr.prev_p = at_fhdr_p->prev_p;
        at_fhdr_p->prev_p->next_p = &khdr_p->fhdr;
        at_fhdr_p->prev_p = &khdr_p->fhdr;
#endif
    }
    else
    {
		/* Determine P-Slot index */
        slot = __memsz_to_pslot(mem_sz);

        KSTATS(
            klob_g.pcnt++;/* Insert a free block into a P-Slot free list */
            if ((klob_g.icnt + klob_g.pcnt) > klob_g.fhwm)
                klob_g.fhwm = klob_g.icnt + klob_g.pcnt;
        );

        KDBGB(
            klob_g.pslot_ctx[slot].cnt++;
            if (klob_g.pslot_ctx[slot].cnt > klob_g.pslot_ctx[slot].hwm)
                klob_g.pslot_ctx[slot].hwm = klob_g.pslot_ctx[slot].cnt;
            klob_g.pslot_ctx[slot].ins++;
        );

		/* Quick insert at head, if slot not empty */
        if (at_head && (klob_g.pslot_flp[slot] != (fhdr_t*)NULL))
        {
            at_fhdr_p = klob_g.pslot_flp[slot];
            klob_g.pslot_flp[slot] = fhdr_p;
        }

        if (klob_g.pslot_flp[slot] == (fhdr_t*)NULL)
        {
		/* Tag P-Slot as non-empty */
            klob_g.pslot_flp[slot] = fhdr_p;
            __set_pslot(slot);
        }

        if (at_fhdr_p == ((fhdr_t*)NULL))
        {
		/*
		 * The block needs to be inserted at the head of the NEXT
		 * non-empty P-Slot.
		 */
            slot = __find_next_set_pslot(slot);

		/* Found a non-empty slot in P-Slots */
            if (slot < KLOB_PSLOT_NUM)
                at_fhdr_p = klob_g.pslot_flp[slot];
            else	/* Insert at end of free list */
                at_fhdr_p = & klob_g.flist;
        }

		/* Insert before "at_khdr_p" */
        khdr_p->fhdr.next_p = at_fhdr_p;
        khdr_p->fhdr.prev_p = at_fhdr_p->prev_p;
        at_fhdr_p->prev_p->next_p = &khdr_p->fhdr;
        at_fhdr_p->prev_p = &khdr_p->fhdr;
    }
}

/*
 *------------------------------------------------------------------------------
 *
 * __khdr_coalesce: Coalesce the neighbouring block headers.
 *
 * Returns a pointer to the lower order block header.
 *
 *------------------------------------------------------------------------------
 */
static inline khdr_t * __khdr_coalesce(khdr_t * a_khdr_p, khdr_t * b_khdr_p)
{
    khdr_t * c_khdr_p = __khdr_next(b_khdr_p);
    __khdr_set_next(a_khdr_p, c_khdr_p);
    __khdr_set_prev(c_khdr_p, a_khdr_p);

    KSTATS(
        klob_g.bcnt--;	/* Decrement block count on block coalescing */
        klob_g.coalesces++; /* Cummulative coalesces */
    );

    KDBGB(
	/* Clear the block header of the coalesced block */
	/* Useful in debugging double frees */
        __khdr_set_next(b_khdr_p,(khdr_t *)NULL);
        __khdr_tag_used(b_khdr_p);
        __khdr_set_prev(b_khdr_p,(khdr_t *)NULL);
    );

    return a_khdr_p;
}

/*
 *------------------------------------------------------------------------------
 *
 * _khdr_split: Carve a free block into an "lower order" aloocated block and
 * a "higher order" free block, with Slot-insertion of the free block using
 * _khdr_insert_tail.
 *
 *------------------------------------------------------------------------------
 */
static void _khdr_split(khdr_t * khdr_p, size_t blk_sz)
{
    unsigned int split_mem_sz;
    khdr_t * split_khdr_p, * next_khdr_p;

	/* Higher order split block to be retained in free list */
    split_khdr_p = (khdr_t *)(((unsigned int)khdr_p) + blk_sz);

    __khdr_tag_free(split_khdr_p);	/* Tag split block as free */

    next_khdr_p = __khdr_next(khdr_p);

	/* Link split block between khdr and khdr->next */
    __khdr_set_next(khdr_p, split_khdr_p);
    __khdr_set_prev(split_khdr_p,khdr_p);
    __khdr_set_next(split_khdr_p, next_khdr_p);
    __khdr_set_prev(next_khdr_p, split_khdr_p);

	/*
	 * Determine the slot into which the split free block
	 * needs to be inserted.
	 */
    split_mem_sz = __khdr_mem_size(split_khdr_p);

	/* Insert the split block at the slot's free list tail, i.e. 1 */
    _khdr_insert(split_khdr_p, split_mem_sz, 1);

    KSTATS(
        klob_g.bcnt++;	/* Increment block count on block splitting */
        klob_g.splits++;	/* Cummulative splits */
    );
}

/*
 *------------------------------------------------------------------------------
 *
 * _delete_islot: Delete a free block from a I-Slot.
 *
 * The block is deleted from the free list. The I-Slot is then tagged empty
 * if this was the only block in its free list.
 *
 *  CAUTION: Assumes ix is a valid non-empty I-Slot index.
 *
 *------------------------------------------------------------------------------
 */
static void _delete_islot(unsigned int ix, fhdr_t * fhdr_p)
{
    klob_g.free_sz -= __khdr_mem_size(__fhdr_to_khdr(fhdr_p));

#ifdef CONFIG_KLOB_ISLOT_SFLIST
	/* Delete the block pointed to by fhdr_p from the free list */
    fhdr_p->prev_p->next_p = fhdr_p->next_p;
    fhdr_p->next_p->prev_p = fhdr_p->prev_p;

	/* determine whether I-Slot ix becomes empty on deletion */
    if (klob_g.islot_flist[ix].next_p == &klob_g.islot_flist[ix])
    {
        __clear_islot(ix);
    }
#else
	/* determine whether I-Slot ix becomes empty on deletion */
    if (klob_g.islot_flp[ix] == fhdr_p)
    {
        fhdr_t * next_fhdr_p = fhdr_p->next_p;

        if ((next_fhdr_p == &klob_g.flist)
           || (__memsz_to_islot(__khdr_mem_size(__fhdr_to_khdr(next_fhdr_p)))
               != ix))
        {
            __clear_islot(ix);
            klob_g.islot_flp[ix] = (fhdr_t *)NULL;
        }
        else
        {
            klob_g.islot_flp[ix] = fhdr_p->next_p;
        }
    }

	/* Delete the block pointed to by fhdr_p from the free list */
    fhdr_p->prev_p->next_p = fhdr_p->next_p;
    fhdr_p->next_p->prev_p = fhdr_p->prev_p;
#endif

    KSTATS(
        klob_g.icnt--;	/* Deleted a block from an I-Slot free list */
    );

    KDBGB(
        klob_g.islot_ctx[ix].cnt--;
        klob_g.islot_ctx[ix].del++;

        fhdr_p->next_p = fhdr_p->prev_p = (fhdr_t*)NULL;
    );
}

/*
 *------------------------------------------------------------------------------
 *
 * _delete_pslot: Delete a free block from a P-Slot.
 *
 * The block is deleted from the free list. The P-Slot is then tagged empty
 * if this was the only block in its free list.
 *
 *  CAUTION: Assumes ix is a valid non-empty P-Slot index.
 *
 *------------------------------------------------------------------------------
 */
static void _delete_pslot(unsigned int ix, fhdr_t * fhdr_p)
{
    klob_g.free_sz -= __khdr_mem_size(__fhdr_to_khdr(fhdr_p));

	/* determine whether P-Slot ix becomes empty on deletion */
    if ( klob_g.pslot_flp[ix] == fhdr_p )
    {
        fhdr_t * next_fhdr_p = fhdr_p->next_p;

        if ( (next_fhdr_p == &klob_g.flist)
           || (__memsz_to_pslot(__khdr_mem_size(__fhdr_to_khdr(next_fhdr_p)))
               != ix))
        {
            __clear_pslot(ix);
            klob_g.pslot_flp[ix] = (fhdr_t *)NULL;
        }
        else
        {
            klob_g.pslot_flp[ix] = fhdr_p->next_p;
        }
    }

	/* Delete the block pointed to by fhdr_p from the free list */
    fhdr_p->prev_p->next_p = fhdr_p->next_p;
    fhdr_p->next_p->prev_p = fhdr_p->prev_p;

    KSTATS(
        klob_g.pcnt--;	/* Deleted a block from a P-Slot free list */
    );

    KDBGB(
        klob_g.pslot_ctx[ix].cnt--;
        klob_g.pslot_ctx[ix].del++;

        fhdr_p->next_p = fhdr_p->prev_p = (fhdr_t*)NULL;
    );
}

/*
 *------------------------------------------------------------------------------
 *
 *  __khdr_delete: Delete a block from a slot's free list.
 *
 *------------------------------------------------------------------------------
 */
static void __khdr_delete(khdr_t * khdr_p)
{
    unsigned int mem_sz = __khdr_mem_size(khdr_p);

    if (mem_sz < KLOB_ISLOT_LMT)
       _delete_islot( __memsz_to_islot(mem_sz), __khdr_to_fhdr(khdr_p));
    else
       _delete_pslot( __memsz_to_pslot(mem_sz), __khdr_to_fhdr(khdr_p));
}


/*
 *------------------------------------------------------------------------------
 *
 *  _allocate_islot: Allocate the first block from the Slot's free list.
 *
 *  On deletion of the first block, if the Slot does not contain any blocks
 *  it is tagged ampty.
 *
 *  CAUTION: No check is done to ensure that the free list is not empty.
 *
 *------------------------------------------------------------------------------
 */
static void * _allocate_islot(unsigned int ix, size_t mem_sz)
{
    khdr_t * khdr_p;
    fhdr_t * fhdr_p;

	/*
	 * The first block is a candidate block, no need for search.
	 * delete_islot
	 */
#ifdef CONFIG_KLOB_ISLOT_SFLIST
    fhdr_p = klob_g.islot_flist[ix].next_p;
#else
    fhdr_p = klob_g.islot_flp[ix];
#endif
    _delete_islot(ix, fhdr_p);

    khdr_p = __fhdr_to_khdr(fhdr_p);

    if (mem_sz)
    {
       _khdr_split(khdr_p, mem_sz + __mem_hdr_size());
    }

    __khdr_tag_used(khdr_p);		/* Tag block as IN USE */

    KSTATS(
        klob_g.bytes += mem_sz;
        klob_g.iallocs++;
    );

    return __khdr_to_mem(khdr_p);	/* Return a pointer to the data */
}

/*
 *------------------------------------------------------------------------------
 *
 *  _search_with_split_pslot: Search a P-Slot free list for a candidate block.
 *
 * A best fit search of KLOB_PSLOT_TRY number of blocks is performed. If an
 * exact match is found then the block is simply deleted from the P-Slot's
 * free list using _delete_pslot(). The block is then carved into two parts.
 * The lower order part is returned whereas the higher order part is reinserted
 * into the appropriate I-Slot or P-Slot using _khdr_split().
 *
 *  CAUTION: Assumes ix is a valid non-empty P-Slot index.
 *
 *------------------------------------------------------------------------------
 */
static khdr_t * _search_with_split_pslot(unsigned int ix, size_t blk_sz)
{
    khdr_t * khdr_p, * tmp_khdr_p;
    fhdr_t * fhdr_p;
    unsigned int try, tmp_sz, better_sz;

    khdr_p = (khdr_t*)NULL;
    fhdr_p = klob_g.pslot_flp[ix];	/* Start of slots free list */

    blk_sz += __mem_hdr_size();	/* Fix mem_sz to KLOB block size */

    try = KLOB_PSLOT_TRY;
    better_sz = ~0UL;

	/* Search for a candidate block */
    while (try)
    {
        if (fhdr_p == &klob_g.flist)
        {
           try = 0;
           break;
        }

        tmp_khdr_p = __fhdr_to_khdr(fhdr_p);	/* Pointer to KLOB block */
        tmp_sz = __khdr_size(tmp_khdr_p);	/* Size of KLOB block */

		/* Is this a better fit */
        if ((tmp_sz < better_sz) && (tmp_sz >= blk_sz))
        {
            khdr_p = tmp_khdr_p;	/* new better candidate */
            better_sz = tmp_sz;

            if (tmp_sz == blk_sz)	/* Exact fit */
            {
                break;
            }
        }

        try--;
        fhdr_p = fhdr_p->next_p;	/* next free block */
    }

	/*
	 * Design Decision:
	 * On a split, the block is always removed from the current slot
	 * even though it may need to be reinserted into same slot.
	 * We always reinsert the leftover split block at tail of slot.
	 * The hypothesis is that we would like to give the leftover block
	 * a chance for coalescing in case the allocated block is quickly
	 * freed. The tradeoff of CPU for list removal and insertion is
	 * deemed acceptable in favour of less fragmentation. This could
	 * be changed to a compiler directive.
	 */
    if (khdr_p != (khdr_t*)NULL)
    {
	/*
	 * Update the P-Slot's free list pointer if the P-Slot becomes
	 * empty of deletion of this block and delete the block from
	 * the free list.
	 */
        _delete_pslot(__memsz_to_pslot(__khdr_mem_size(khdr_p)),
                      __khdr_to_fhdr(khdr_p));

	/* Check whether we need to split the block */
        if (try == 0)	/* did not find an exact fit */
        {
		/*
		 * Split the block into two parts, the higher order part is
		 * reinserted into an appropriate slot's free list, and is
		 * tagged free.
		 */
           _khdr_split(khdr_p, blk_sz);

        }
    }

    return khdr_p;
}

/*
 *------------------------------------------------------------------------------
 *
 * _allocate_pslot: Allocate a free block from a P-Slot.
 *
 * The P-Slot index is first checked to see whether we overflow.
 * If we do not overflow, a candidate block is fetched starting from
 * the free list pertaining to the P-Slot index using
 * _search_with_split_pslot().
 * The returned block is tagged as USED and a pointer to the data
 * memory is returned to the user.
 *
 * On an overflow, a block is fetched from the overflow memory
 * allocator and directly returned to the user.
 *
 *------------------------------------------------------------------------------
 */
static void * _allocate_pslot(unsigned int ix, size_t mem_sz, int gfp,
                              unsigned long * lock_flag_p)
{
    khdr_t * khdr_p = NULL;
    void * mem_p;

    if (ix < KLOB_PSLOT_NUM)	/* Did we hit P-Slot demarcation ? */
    {
	/* Find a candidate block, may fall into a different P-Slot */
        khdr_p = _search_with_split_pslot(ix, mem_sz);

        if (khdr_p != (khdr_t*)NULL)
        {
            __khdr_tag_used(khdr_p);		/* Tag block as IN USE */

            KSTATS(
                klob_g.bytes += mem_sz;
                klob_g.pallocs++;
            );

            return __khdr_to_mem(khdr_p);	/* Return pointer to data */
        }
    }

     /* CAUTION: This recursively calls klob_alloc()->_allocate_pslot() */

    if ((klob_g.extend)
        && !( in_atomic())	/* DO NOT EXTEND IF ATOMIC */
#if (CONFIG_KLOB_ALIGN > 8)
        && (mem_sz <= (CONFIG_KLOB_MEM_EXTN - (3*CONFIG_KLOB_ALIGN)))
#else
        && (mem_sz <= (CONFIG_KLOB_MEM_EXTN - (3*sizeof(khdr_t))))
#endif
        )
    {
        if (klob_g.numpools < CONFIG_KLOB_MAX_EXTN)
        {
            if (klob_g.free_sz < CONFIG_KLOB_LMT_EXTN)
            {
                local_irq_restore( *lock_flag_p );/* <--- KLOB CRITICAL SECION END */

                mem_p = klob_extend(mem_sz, gfp);

                local_irq_save( *lock_flag_p );/* ---> KLOB CRITICAL SECION BEGIN */

                if (mem_p != (void *)NULL)
                {
                    return mem_p;
                }
                else
                {
                    klob_g.extend = 0;
                    KDBGB(printk("\tKLOB Extension did not help!\n"););
                }
            }
        }
        else
        {
            klob_g.extend = 0;	/* Disable future extension */
        }
    }

    KDBGB(
        printk("\tOverflow: Free[%lu] Req[%d] %d %d\n",
               klob_g.free_sz, mem_sz,
               CONFIG_KLOB_MEM_EXTN, CONFIG_KLOB_LMT_EXTN);
    );

    KSTATS(
	/* If we reached here, it means we could not find a KLOB block */
        klob_g.fwd_allocs++;
        klob_g.fwd_bytes += mem_sz;
        klob_g.failexts++;
    );

    local_irq_restore( *lock_flag_p );/* <--- KLOB CRITICAL SECION END */

    mem_p = __kmalloc(mem_sz, gfp);

    local_irq_save( *lock_flag_p );/* ---> KLOB CRITICAL SECION BEGIN */

    return mem_p;
}

/*
 *------------------------------------------------------------------------------
 *
 * 	Exported Interface to KLOB for kmalloc, kfree, init, audit and display
 *
 *	INTERFACE	:
 *	=================
 *	klob_alloc	: Allocate a block of memory.
 *	klob_free	: Deallocate a block of memory.
 *	klob_size	: Determine the size of an allocated block of memory.
 *
 *	INITIALIZATION	:
 *	=================
 *	klob_init	: Initializae KLOB with a region of memory.
 *
 *	AUDIT		:
 *	=================
 *	klob_scan_blist	: Scan the Block List forward and backwards.
 *	klob_scan_flist	: Scan the Free List forward and backwards.
 *	klob_audit	: Audit Klob State: Lists, Maps, Counters.
 *      (CAUTION: IRQ disabled for duration of audit, for LAB only use)
 *
 *	DISPLAY		:
 *	=================
 *	klob_seq_show	: Dump the klob state into proc fs	[TARGET ONLY]
 *
 *	klobappl_dump_blist : Dump the Block List		[APPL ONLY]
 *	klobappl_dump_flist : Dump the Block List		[APPL ONLY]
 *	klobappl_show	: Dump the KLOB global state		[APPL ONLY]
 *
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 *
 * Allocate a variable length of memory from KLOB managed memory.
 *
 * Determine the Slot that could service the request.
 *    if the mem_sz < KLOB_ISLOT_LMT,
 *         Find the index of the corresponding I-Slot.
 *         if corresponding I-Slot is not empty
 *              Allocate a block from corresponding I-Slot.
 *         else
 *              Determine next non-empty I-Slot.
 *              if found a next non-empty I-Slot
 *                   Allocate a block from found next non-empty I-Slot.
 *              else
 *                   Determine the first non-empty P-Slot.
 *                   Allocate a block from found first non-empty P-Slot.
 *    else
 *         Find the index of the corresponding P-Slot.
 *         if corresponding P-Slot is not empty
 *              Allocate a block from corresponding P-Slot.
 *         else
 *              Determine next non-empty P-Slot.
 *              Allocate a block from found next non-empty P-Slot.
 *
 * Return allocated block from I-Slot or P-Slot or SLAB.
 *
 *------------------------------------------------------------------------------
 */
static inline void * klob_alloc(size_t mem_sz, int gfp)
{
    unsigned long lock_flag;
    unsigned int ix;
    void * blk_p;

    mem_sz = __klob_roundup(mem_sz);

    if (mem_sz < KLOB_ISLOT_LMT)	/* mem_sz is in I-Slot */
    {
        ix = __memsz_to_islot(mem_sz);

        local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

        if (__test_islot(ix))	/* Candidate I-Slot not empty ? */
        {
             blk_p = _allocate_islot(ix, 0);	/* no split */
        }
        else	/* Corresponding mem_sz I-Slot is empty */
        {
 		/* Find next candidate I-Slot, if any */
            ix = __find_next_set_islot(ix + KLOB_ISLOT_IFG + 1);

            if (ix < KLOB_ISLOT_NUM)	/* Found next candidate I-Slot */
            {
                blk_p = _allocate_islot(ix, mem_sz);
            }
            else	/* All following I-Slots are empty, search P-Slot */
            {
                blk_p = _allocate_pslot(__find_first_set_pslot(),
                                        mem_sz, gfp, &lock_flag);
            }		/* else slot < KLOB_ISLOT_NUM */
        }		/* else __test_islot(slot) */
    }			/* else mem_sz < KLOB_ISLOT_LMT */
    else	/* mem_sz < KLOB_ISLOT_LMT */
    {
        ix = __memsz_to_pslot(mem_sz);

        local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

        if (__test_pslot(ix))	/* Candidate P-Slot not empty ? */
        {
            blk_p = _allocate_pslot(ix, mem_sz, gfp, &lock_flag);
        }
        else	/* Corresponding mem_sz P-Slot is empty */
        {
            blk_p = _allocate_pslot(__find_next_set_pslot(ix),
                                    mem_sz, gfp, &lock_flag);
        }		/* else  __test_pslot(slot) */
    }		/* else mem_sz < KLOB_ISLOT_LMT */

    local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */

    return blk_p;
}

/*
 *------------------------------------------------------------------------------
 *
 * Deallocate a block of memory that was allocated through klob_alloc
 *
 *------------------------------------------------------------------------------
 */
static inline void klob_free(const void * mem_p)
{
    unsigned long lock_flag;
    local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

    if (__is_mem_klob(mem_p))
    {
        khdr_t * nbr_khdr_p;
        khdr_t * khdr_p = __mem_to_khdr(mem_p);


	/* Check whether we can coalesce with left neighbour */
        nbr_khdr_p = __khdr_prev(khdr_p);
        if (__khdr_is_free(nbr_khdr_p))
        {
            __khdr_delete(nbr_khdr_p);
            khdr_p = __khdr_coalesce(nbr_khdr_p, khdr_p);
        }

	/* Check whether we can coalesce with right neighbour */
        nbr_khdr_p = __khdr_next(khdr_p);
        if (__khdr_is_free(nbr_khdr_p))
        {
            __khdr_delete(nbr_khdr_p);
            khdr_p = __khdr_coalesce(khdr_p, nbr_khdr_p);
        }

	/* Insert coalesced block at the slot's free list head, i.e. 0 */
        _khdr_insert(khdr_p, __khdr_mem_size(khdr_p), 0);

        __khdr_tag_free(khdr_p);

        KSTATS(
            klob_g.frees++;
        );

        local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */
    }
    else
    {
        local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */

#ifdef KLOB_TEST_APPL
        free((void*)mem_p);
#else
        __kfree(mem_p);
#endif

        KSTATS(
            local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */
            klob_g.fwd_frees++;
            local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END*/
        );
    }
}

/*
 *------------------------------------------------------------------------------
 *
 * Determine the size of a klob allocated block
 *
 *------------------------------------------------------------------------------
 */
static inline unsigned int klob_size(const void * mem_p)
{
    unsigned int mem_sz;
    unsigned long lock_flag;

    local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

    if (__is_mem_klob(mem_p))
    {
        mem_sz = __mem_size(mem_p);
        local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */
    }
    else
    {
        local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */
        mem_sz = __ksize(mem_p);
    }

    return mem_sz;
}

#ifdef KLOB_AUDIT
/*
 *------------------------------------------------------------------------------
 * Audit Block List:
 *------------------------------------------------------------------------------
 */
void klob_scan_blist(unsigned int bidirect)
{
    khdr_t * khdr_p;
    unsigned long lock_flag;

    local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

	/* Forward walk */
    khdr_p = __bhdr_to_khdr(klob_g.blist.next_p);
    while (__khdr_to_bhdr(khdr_p) != & klob_g.blist)
        khdr_p = __khdr_next(khdr_p);

	/* Backward walk */
    if ( bidirect )
    {
        khdr_p = __bhdr_to_khdr(klob_g.blist.prev_p);
        while (__khdr_to_bhdr(khdr_p) != & klob_g.blist)
            khdr_p = __khdr_prev(khdr_p);
    }

    local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */
}

void klob_scan_flist(unsigned int bidirect)
{
    unsigned long lock_flag;
    fhdr_t * cur_p, * last_p;
#ifdef CONFIG_KLOB_ISLOT_SFLIST
    unsigned int ix;
#endif

    local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

#ifdef CONFIG_KLOB_ISLOT_SFLIST
    for (ix=0; ix<KLOB_ISLOT_NUM; ix++)
    {
        cur_p = klob_g.islot_flist[ix].next_p;
        last_p = &klob_g.islot_flist[ix];
        while (cur_p != last_p)
            cur_p = cur_p->next_p;
    }
    if (bidirect)
    {
        for (ix=0; ix<KLOB_ISLOT_NUM; ix++)
        {
            cur_p = klob_g.islot_flist[ix].prev_p;
            last_p = &klob_g.islot_flist[ix];
            while (cur_p != last_p)
                cur_p = cur_p->prev_p;
        }
    }
#endif

    last_p = &klob_g.flist;

	/* Forward walk */
    cur_p = klob_g.flist.next_p;
    while (cur_p != last_p)
        cur_p = cur_p->next_p;

    if (bidirect)
    {
        cur_p = klob_g.flist.prev_p;
        while (cur_p != last_p)
            cur_p = cur_p->prev_p;
    }

    local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */

}

/*
 *------------------------------------------------------------------------------
 *
 * Audit: Rigorous audit of block and free list
 *
 * CAUTION:
 * Invoking klob_audit on target will lock IRQs for the duration of the audit
 *
 *------------------------------------------------------------------------------
 */
int klob_audit(unsigned int verbose)
{
    int i, errors;
    unsigned long lock_flag;
    unsigned int blocks, allocated_blocks, free_blocks;
    unsigned long block_sz, allocated_bytes, free_bytes, memory_bytes;
    khdr_t * khdr_p;
    fhdr_t * cur_p, * last_p;
#ifndef CONFIG_KLOB_ISLOT_SFLIST
    unsigned int prev_mem_sz;
#endif
    unsigned int cur_mem_sz, px, ix;
#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
    unsigned long cnt;
#endif
#ifdef KLOB_STATS
    unsigned long fcnt;
#endif

    local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECION BEGIN */

	/*
	 * Audit the double linked block list, using khdr_p
	 * - blocks: cummulate number of blocks
	 * - free_blocks: cummulate number of free blocks
	 * - allocated_blocks: cummulate number of allocated blocks
	 * - memory_bytes: cummulate total block size
	 * - free_bytes: cummulate total free size (including header)
	 * - allocated_bytes: cummulate total allocated size 
	 */
    khdr_p = __bhdr_to_khdr(klob_g.blist.next_p);

    errors = blocks = allocated_blocks = free_blocks = 0;
    allocated_bytes = free_bytes = memory_bytes = 0;

    while (__khdr_to_bhdr(khdr_p) != & klob_g.blist)
    {
        if (! __is_khdr(khdr_p))
        {
            errors++;
            printk("ERROR klob_audit blist: khdr_p[0x%08x] not in pools\n",
                   (int) khdr_p);
        }
        else
        {
            blocks++;

            if ((__khdr_to_bhdr(__khdr_next(khdr_p)) != & klob_g.blist)
                && (khdr_p != __khdr_prev(__khdr_next(khdr_p))))
            {
                errors++;
                printk("ERROR klob_audit blist: khdr_p[0x%08x] "
                       "next_p[0x%08x]->prev_p[0x%08x] mismatch\n",
                       (int)khdr_p, (int)__khdr_next(khdr_p),
                       (int)__khdr_prev(__khdr_next(khdr_p)));
                break;
            }

            if (__is_kpool(khdr_p))
            {
#if (CONFIG_KLOB_ALIGN > 8)
                block_sz = CONFIG_KLOB_ALIGN; /* No data */
#else
                block_sz = sizeof(khdr_t); /* No data */
#endif
            }
            else
            {
                block_sz = __khdr_size(khdr_p);
            }

            memory_bytes += block_sz;

            if (__khdr_is_free(khdr_p))
            {
                free_blocks++;
                free_bytes += block_sz;
            }
            else
            {
                allocated_blocks++;
                allocated_bytes += block_sz;
            }
        }

        khdr_p = __khdr_next(khdr_p);
    }

	/*
	 * Audit runtime statistics: klob_g.memory_sz
	 */
    if (memory_bytes != klob_g.memory_sz)
    {
        errors++;
        printk("ERROR klob_audit blist: memory_bytes[%lu] mismatch "
               "klob_g.memory_sz[%lu]\n", memory_bytes, klob_g.memory_sz);
    }

	/*
	 * Audit the bit maps for I-Slot. Audit count of blocks in I-Slot.
	 * If bitmap of a slot is set, assert that free list and count
	 * for the I-Slot are correspondingly non-NULL.
	 * - cnt: cummulate blocks in I-Slots using runtime slot count
	 */

#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
    cnt = 0;
#endif
    for (i=0; i<KLOB_ISLOT_NUM; i++)
    {
        if (__test_islot(i))
        {
#ifdef CONFIG_KLOB_ISLOT_SFLIST
            if ( (klob_g.islot_flist[i].next_p == &klob_g.islot_flist[i])
               ||(klob_g.islot_flist[i].prev_p == &klob_g.islot_flist[i]))
            {
                errors++;
                printk("ERROR klob_audit I-Slot map: set ix[%d]"
                       " flist[0x%08x,0x%08x] mismatch [0x%08x]\n", i,
                        (int)klob_g.islot_flist[i].next_p,
                        (int)klob_g.islot_flist[i].prev_p,
                        (int)&klob_g.islot_flist[i] );
            }
#else
            if (klob_g.islot_flp[i] == NULL)
            {
                errors++;
                printk("ERROR klob_audit I-Slot map: set ix[%d] flp NULL\n", i);
            }
#endif
            KDBGB(
                if (klob_g.islot_ctx[i].cnt == 0)
                {
                    errors++;
                    printk("ERROR klob_audit I-Slot map: set ix[%d] cnt 0\n",i);
                }
            );

#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
            cnt += klob_g.islot_ctx[i].cnt;
#endif
        }
        else
        {
#ifdef CONFIG_KLOB_ISLOT_SFLIST
            if ( (klob_g.islot_flist[i].next_p != &klob_g.islot_flist[i])
               ||(klob_g.islot_flist[i].prev_p != &klob_g.islot_flist[i]))
            {
                errors++;
                printk("ERROR klob_audit I-Slot map: clr ix[%d]"
                       " flist[0x%08x,0x%08x] mismatch [0x%08x]\n", i,
                       (int)klob_g.islot_flist[i].next_p,
                       (int)klob_g.islot_flist[i].prev_p,
                       (int)&klob_g.islot_flist[i] );
            }
#else
            if (klob_g.islot_flp[i])
            {
                errors++;
                printk("ERROR klob_audit I-Slot map: clr ix[%d] flp[0x%08x]\n",
                       i, (int)klob_g.islot_flp[i]);
            }
#endif
            KDBGB(
                if (klob_g.islot_ctx[i].cnt)
                {
                    errors++;
                    printk("ERROR klob_audit I-Slot map: clr ix[%d] cnt[%lu]\n",
                           i, klob_g.islot_ctx[i].cnt);
                }
            );
        }
    }
#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
	/*
	 * Validate cnt of number of blocks in I-Slots computed by
	 * traversing all I-Slots against run-time statistics klob_g.icnt
	 */
    if (cnt != klob_g.icnt)
    {
        errors++;
        printk("ERROR klob_audit I-Slot: cnt[%lu] != klob_g.icnt[%lu]\n",
               cnt, klob_g.icnt);
    }
#endif

	/*
	 * Audit the bit maps for P-Slot. Audit count of blocks in I-Slot
	 * If bitmap of a P-slot is set, assert that free list and count
	 * for the P-Slot are correspondingly non-NULL.
	 * - cnt: cummulate blocks in P-Slots using runtime slot count
	 */

#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
    cnt=0;
#endif
    for (i=0; i<KLOB_PSLOT_NUM; i++)
    {
        if (__test_pslot(i))
        {
            if (klob_g.pslot_flp[i] == NULL)
            {
                errors++;
                printk("ERROR klob_audit P-Slot map: set ix[%d] flp NULL\n", i);
            }
            KDBGB(
                if (klob_g.pslot_ctx[i].cnt == 0)
                {
                    errors++;
                    printk("ERROR klob_audit P-Slot map: set ix[%d] cnt 0\n",i);
                }
            );
#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
            cnt += klob_g.pslot_ctx[i].cnt;
#endif
        }
        else
        {
            if (klob_g.pslot_flp[i])
            {
                errors++;
                printk("ERROR klob_audit P-Slot map: clr ix[%d] flp[0x%08x]\n",
                       i, (int)klob_g.pslot_flp[i]);
            }
            KDBGB(
                if (klob_g.pslot_ctx[i].cnt)
                {
                    errors++;
                    printk("ERROR klob_audit P-Slot map: clr ix[%d] cnt[%lu]\n",
                           i, klob_g.pslot_ctx[i].cnt);
                }
            );
        }
    }
#if defined(KLOB_DEBUG_APPL) && defined(KLOB_STATS)
	/*
	 * Validate cnt of number of blocks in P-Slots computed by
	 * traversing all P-Slots against run-time statistics klob_g.icnt
	 */
    if (cnt != klob_g.pcnt)
    {
        errors++;
        printk("ERROR klob_audit P-Slot: cnt[%lu] != klob_g.pcnt[%lu]\n",
               cnt, klob_g.pcnt);
    }
#endif

	/*
	 * Audit Free List Sorted in I-Slot Range.
	 * Audit each I-Slot's flp points to the start of its free list.
	 * ASSUMING ALL BLOCKS IN AN I-SLOT HAVE THE SAME SIZE.
	 */
#ifdef CONFIG_KLOB_ISLOT_SFLIST
    cur_mem_sz = 0;
    for (ix=0; ix<KLOB_ISLOT_NUM; ix++)
    {
	/* I-Slot free list is not empty */
        if (klob_g.islot_flist[ix].next_p != &klob_g.islot_flist[ix])
        {
            cur_p = klob_g.islot_flist[ix].next_p;
            last_p = &klob_g.islot_flist[ix];
            while (cur_p != last_p)
            {
                if (__khdr_mem_size(__fhdr_to_khdr(cur_p)) != cur_mem_sz)
                {
                   errors++;
                   printk("ERROR klob_audit I-Slot[%u] Free List not sorted\n",
                          ix);
                }
                cur_p = cur_p->next_p;
            }
        }
        cur_mem_sz += KLOB_ISLOT_OFF;
    }
#else
    ix=0;
    cur_p = klob_g.flist.next_p;
    px = __find_first_set_pslot();
    last_p = klob_g.pslot_flp[px];

    prev_mem_sz = 0;
    while (cur_p != last_p)
    {
        cur_mem_sz = __khdr_mem_size(__fhdr_to_khdr(cur_p));
        if ((cur_mem_sz < prev_mem_sz)
           || (cur_mem_sz % CONFIG_KLOB_ALIGN)
           || ((ix=__memsz_to_islot(cur_mem_sz)) >= KLOB_ISLOT_NUM))
        {
            errors++;
            printk("ERROR klob_audit I-Slot[%u] Free List not sorted\n", ix);
        }
        if (cur_mem_sz > prev_mem_sz)
        {
           if (klob_g.islot_flp[ix] != cur_p)
           {
               errors++;
               printk("ERROR klob_audit I-Slot[%u] Free List pointer\n", ix);
           }
           prev_mem_sz = cur_mem_sz;
        }
        cur_p = cur_p->next_p;
    }
#endif

	/*
	 * Assert that all blocks in a P-Slot belong to the P-Slot's range
	 */
    for (px=0; px<KLOB_PSLOT_NUM; px++)
    {
        if (klob_g.pslot_flp[px])
        {
            cur_p = klob_g.pslot_flp[px];
            last_p = klob_g.pslot_flp[__find_next_set_pslot(px)];
            while (cur_p != last_p)
            {
                cur_mem_sz = __khdr_mem_size(__fhdr_to_khdr(cur_p));
                KDBGB(
                    if ( (cur_mem_sz < klob_g.pslot_ctx[px].rbgn)
                       ||(cur_mem_sz > klob_g.pslot_ctx[px].rend))
                    {
                        errors++;
                        printk("ERROR klob_audit P-Slot[%u]"
                               " Free List not sorted\n", px);
                    }
                );
                cur_p = cur_p->next_p;
            }
        }
    }

	/*
	 * Audit Double Linked Free List
	 */
    KSTATS( fcnt = 0; );

#ifdef CONFIG_KLOB_ISLOT_SFLIST
    for (ix=0; ix<KLOB_ISLOT_NUM; ix++)
    {
        /* I-Slot free list is not empty */
        if (!__isempty_islot_flist(ix))
        {
            cur_p = klob_g.islot_flist[ix].next_p;
            last_p = &klob_g.islot_flist[ix];
            while (cur_p != last_p)
            {
                if (cur_p->next_p->prev_p != cur_p)
                {
                    errors++;
                    printk("ERROR klob_audit flist: cur_p[0x%08x] "
                           "next_p[0x%08x]->prev_p[0x%08x] mismatch\n",
                           (int)cur_p, (int)cur_p->next_p,
                           (int)cur_p->next_p->prev_p);
                }
                KSTATS( fcnt++; );
                cur_p = cur_p->next_p;
            }
        }
    }
#endif

    cur_p = klob_g.flist.next_p;
    last_p = &klob_g.flist;
    while (cur_p != last_p)
    {
        if (cur_p->next_p->prev_p != cur_p)
        {
            errors++;
            printk("ERROR klob_audit flist: cur_p[0x%08x] "
                   "next_p[0x%08x]->prev_p[0x%08x] mismatch\n",
                   (int)cur_p, (int)cur_p->next_p,(int)cur_p->next_p->prev_p);
        }
        KSTATS( fcnt++; );
        cur_p = cur_p->next_p;
    }
    KSTATS(
        if (fcnt != (klob_g.icnt + klob_g.pcnt))
        {
            errors++;
            printk("ERROR klob_audit I+P Slot:"
                   " cnt[%lu] != icnt[%lu] + pcnt[%lu]\n",
                   fcnt, klob_g.icnt, klob_g.pcnt);
        }
    );

    KSTATS(
	/*
	 * Audit statictics
	 */
        if ( (unsigned long long)(klob_g.bcnt - (klob_g.icnt + klob_g.pcnt
                                                 + 2*klob_g.numpools))
           !=((klob_g.iallocs + klob_g.pallocs) - klob_g.frees) )
        {
            errors++;
            printk("ERROR klob_audit statistics: mismatch\n"
                   "\tInuse[%lu] = bcnt<%lu> - (icnt<%lu> + pcnt<%lu> + 2*%d)\n"
                   "\tInuse[%llu] = (iallocs<%llu> + pallocs<%llu>)"
                   " - frees<%llu>\n",
                   klob_g.bcnt - (klob_g.icnt + klob_g.pcnt + 2),
                   klob_g.bcnt, klob_g.icnt, klob_g.pcnt, klob_g.numpools,
                   (klob_g.iallocs + klob_g.pallocs) - klob_g.frees,
                   klob_g.iallocs, klob_g.pallocs, klob_g.frees);
        }
    );

    KDBGB(
        if (verbose)
        {
           printk("\nKLOB Audit: Errors[%d] "
                  "Blks[%u,%lu] = A[%u,%lu] + F[%u,%lu]\n",
                  errors, blocks, memory_bytes,
                  allocated_blocks, allocated_bytes,
                  free_blocks, free_bytes);
        }
    );

    local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECION END */

    return -errors;
}
#endif

#ifdef KLOB_TEST_APPL
/*
 * The following implementation is "nearly" a duplicate of klob_seq_show().
 * The two implementations could be collapsed into a single implementations,
 * with intermingled ifdefs, defines, ... making it "tedious" to read but
 * "easier" to maintain ... an oxymoron argument.
 */

/*
 *------------------------------------------------------------------------------
 *
 * klobappl_dump_map: Dump a bitmap, formatted.
 *
 *------------------------------------------------------------------------------
 */
static inline void klobappl_dump_map(unsigned long word)
{
    int i;
    unsigned long mask = 1UL << 31;
    printf("\t");
    for (i=1; i<=32; i++, mask = mask >> 1)
    {
        if (word & mask) printf("1"); else printf("0");
        if ((i % 8) == 0) printf("  ");
    }
    printf("\n");
}

/*
 *------------------------------------------------------------------------------
 *
 * klobappl_dump_blist: Dump the KLOB block list
 *
 * Verbosity level defines the degree of information displayed.
 * - Verbosity 0: Display summary of total blocks, free and allocated, bytes
 * - Verbosity 2: Display each block
 *
 *------------------------------------------------------------------------------
 */
unsigned int klobappl_dump_blist(unsigned int verbose)
{
    bhdr_t * bhdr_p = klob_g.blist.next_p;

    unsigned int khdr_sz, free_bytes, allocated_bytes, free_maxbytes,
       allocated_ibytes, allocated_pbytes, free_ibytes, free_pbytes,
       blocks, free_blocks, allocated_iblocks, allocated_pblocks,
       free_iblocks, free_pblocks;

    khdr_sz = free_bytes = allocated_bytes = free_maxbytes
    = allocated_ibytes = allocated_pbytes = free_ibytes = free_pbytes
    = blocks = free_blocks = allocated_iblocks = allocated_pblocks
    = free_iblocks = free_pblocks = 0;

    KDBGB(
        if (verbose >= 2)
            printf("\nBlock List Walk:\n");
    );

    while (bhdr_p != & klob_g.blist)
    {
        blocks++;

        if (__is_kpool(__bhdr_to_khdr(bhdr_p)))
        {
#if (CONFIG_KLOB_ALIGN > 8)
            khdr_sz = CONFIG_KLOB_ALIGN;	/* No data */
#else
            khdr_sz = sizeof(khdr_t);	/* No data */
#endif
        }
        else
        {
            khdr_sz = __khdr_size(__bhdr_to_khdr(bhdr_p));
        }

        KDBGB(
            if (verbose >= 2)
                printf("\t\tBlk[0x%08x  %8d ",
                       (int)bhdr_p, khdr_sz - __mem_hdr_size() );
        );

        if (__khdr_is_free(__bhdr_to_khdr(bhdr_p)))
        {
           free_blocks++;
           free_bytes += khdr_sz;

           if (khdr_sz < KLOB_ISLOT_LMT + __mem_hdr_size())
           {
               free_iblocks++;
               free_ibytes += khdr_sz;
           }
           else
           {
               free_pblocks++;
               free_pbytes += khdr_sz;
           }

           if (khdr_sz > free_maxbytes)
              free_maxbytes = khdr_sz;

           KDBGB(
               if (verbose >= 2)
               {
                   if (khdr_sz < KLOB_ISLOT_LMT + __mem_hdr_size() )
                       printf("+%2d  F] Slot-I[%u]\n", (int)__mem_hdr_size(),
                              __memsz_to_islot(khdr_sz - __mem_hdr_size()));
                   else
                       printf("+%2d  F] Slot-P[%u]\n", (int)__mem_hdr_size(),
                              __memsz_to_pslot(khdr_sz - __mem_hdr_size()));
               }
           );
        }
        else
        {
           allocated_bytes += khdr_sz;
           if (khdr_sz < KLOB_ISLOT_LMT + __mem_hdr_size())
           {
               allocated_iblocks++;
               allocated_ibytes += khdr_sz;
           }
           else
           {
               allocated_pblocks++;
               allocated_pbytes += khdr_sz;
           }

           KDBGB(
               if (verbose >= 2)
                   printf("     U]\n");
           );
        }

        bhdr_p = __khdr_to_bhdr(__khdr_next(__bhdr_to_khdr(bhdr_p)));
    }

    if (verbose)
    {
        printf("\n\tTotal Bytes[%u] = Allocated[%u] + Free[%u]\n"
               "\t\tAllocated ISlot[%u]\n"
               "\t\tAllocated PSlot[%u]\n"
               "\t\tFree ISlot[%u]\n"
               "\t\tFree PSlot[%u]\n"
               "\t\tMax Free[%u]\n",
               allocated_bytes + free_bytes, allocated_bytes, free_bytes,
               allocated_ibytes, allocated_pbytes,
               free_ibytes, free_pbytes, free_maxbytes);

        printf("\n\tTotal Blocks[%u] = Allocated[%u] + Free[%u]\n"
               "\t\tAllocated ISlot[%u]\n"
               "\t\tAllocated PSlot[%u]\n"
               "\t\tFree ISlot[%u]\n"
               "\t\tFree PSlot[%u]\n",
               blocks, blocks - free_blocks, free_blocks,
               allocated_iblocks, allocated_pblocks,
               free_iblocks, free_pblocks);

        printf("\nOverhead [%u] bytes (included in total allocated bytes)\n",
              (blocks - free_blocks) * __mem_hdr_size());

    }

    return blocks;
}

/*
 *------------------------------------------------------------------------------
 *
 *  klobappl_dump_flist: Dump the KLOB Free list along with the memory maps.
 *
 *  Verbosity level defines the degree of information displayed.
 *  - Verbosity 0: Display total free blocks.
 *  - Verbosity 1: Display Bit Maps.
 *  - Verbosity 2: Display each free block in list.
 *
 *------------------------------------------------------------------------------
 */
unsigned int klobappl_dump_flist(unsigned int verbose)
{
    int i;
    fhdr_t * fhdr_p;
    khdr_t * khdr_p;
    unsigned int free_blks, free_bytes, khdr_sz;
    free_blks = free_bytes = 0;

    if (verbose)
    {
        printf("\nIncremental Slot Sequence Map:\n");
        for (i = 0; i<KLOB_ISLOT_MAP; i++)
        {
            printf("\t%3d .. %3d :", (i*32)+31, (i*32));
            klobappl_dump_map(klob_g.islot_map[i]);
        }
        printf("\tDemarcation:");
        klobappl_dump_map(klob_g.islot_map[KLOB_ISLOT_MAP]);

        printf("\nPower Slot Sequence Map:\n\t\t");
        klobappl_dump_map(klob_g.pslot_map);
    }

    KDBGB(
        if (verbose >= 2)
            printf("\nFree List Walk:\n");
    );

#ifdef CONFIG_KLOB_ISLOT_SFLIST
    for (i=0; i<KLOB_ISLOT_NUM; i++)
    {
        if (!__isempty_islot_flist(i))
        {
           fhdr_p = klob_g.islot_flist[i].next_p;
           while (fhdr_p != & klob_g.islot_flist[i])
           {
               khdr_p = __fhdr_to_khdr(fhdr_p);
               free_blks += __khdr_is_free(khdr_p);
               khdr_sz = __khdr_size(khdr_p);
               free_bytes += khdr_sz;

               KDBGB(
                   if (verbose >= 2)
                   {
                        printf("\t\tBlk[0x%08x  %8d +%2d  F]",
                               (int)khdr_p, khdr_sz - __mem_hdr_size(),
                               (int)__mem_hdr_size());

                       if (__khdr_is_free(khdr_p))
                       {
                           printf(" Slot-I[%u]\n",
                              __memsz_to_islot(khdr_sz - __mem_hdr_size()));
                       }
                       else
                       {
                           printf("  U]\n");
                       }
                   }
               );

               fhdr_p = fhdr_p->next_p;
           }
        }
    }
#endif

    fhdr_p = klob_g.flist.next_p;
    while (fhdr_p != & klob_g.flist)
    {
        khdr_p = __fhdr_to_khdr(fhdr_p);
        free_blks += __khdr_is_free(khdr_p);
        khdr_sz = __khdr_size(khdr_p);
        free_bytes += khdr_sz;

        KDBGB(
            if (verbose >= 2)
            {
                 printf("\t\tBlk[0x%08x  %8d +%2d  F]",
                        (int)khdr_p, khdr_sz - __mem_hdr_size(),
                        (int)__mem_hdr_size());

                if (__khdr_is_free(khdr_p))
                {
                    if (khdr_sz < KLOB_ISLOT_LMT)
                        printf(" Slot-I[%u]\n",
                               __memsz_to_islot(khdr_sz - __mem_hdr_size()));
                    else
                        printf(" Slot-P[%u]\n",
                               __memsz_to_pslot(khdr_sz - __mem_hdr_size()));
                }
                else
                {
                    printf("  U]\n");
                }
            }
        );

        fhdr_p = fhdr_p->next_p;
    }

    if (verbose)
       printf("\tFreeBlocks[%d] FreeBytes[%d = %d + %d]\n",
              free_blks, free_bytes,
              (free_bytes - (free_blks*__mem_hdr_size())),
              (free_blks*__mem_hdr_size()));

    return free_blks;
}

/*
 *------------------------------------------------------------------------------
 *
 * klobappl_show: Display KLOB debug and statistics
 * - Dump the block and free list.
 * The verbosity parameter defines the degree of information displayed.
 * Verbosity level 0, display summary.
 *
 *------------------------------------------------------------------------------
 */
void klobappl_show(unsigned int verbose)
{
    unsigned int p;
    unsigned long bgn_p, end_p;

    KDBGB(
        int i;

        if (verbose>=3)
        {
            printf("\nKLOB Configuration:\n"
                   "\tCONFIG_KLOB_ALIGN = %d\n"
                   "\tKLOB_ISLOT_OFF = %d\n"
                   "\tKLOB_ISLOT_SFT = %d\n"
                   "\tKLOB_ISLOT_LMT = %d\n"
                   "\tKLOB_ISLOT_NUM = %d\n"
                   "\tKLOB_ISLOT_MAP = %d\n"
                   "\tKLOB_ISLOT_IFG = %d\n"
                   "\tKLOB_PSLOT_NUM = %d\n"
                   "\tKLOB_PSLOT_LZO = %d\n", CONFIG_KLOB_ALIGN,
                   KLOB_ISLOT_OFF, KLOB_ISLOT_SFT, KLOB_ISLOT_LMT,
                   KLOB_ISLOT_NUM, KLOB_ISLOT_MAP, KLOB_ISLOT_IFG,
                   KLOB_PSLOT_NUM, KLOB_PSLOT_LZO);

            printf("\nIncremental Slot Sequence:\n"
                 "\t%4s  %18s  %12s : %6s %6s %10s %10s\n",
                 "Slot", "Block Size Range", "FreeList Ptr",
                 "Count", "HighWM", "Deletes", "Inserts");
            for (i=0; i<KLOB_ISLOT_NUM; i++)
            {
                printf("\t%4d   %6u .. %6u   [", i,
                       klob_g.islot_ctx[i].rbgn, klob_g.islot_ctx[i].rend);
#ifdef CONFIG_KLOB_ISLOT_SFLIST
                if (klob_g.islot_flist[i].next_p != &klob_g.islot_flist[i])
                    printf("0x%08x",
                       (int) __fhdr_to_khdr(klob_g.islot_flist[i].next_p));
                else
                    printf("          ");
#else
                if (klob_g.islot_flp[i] != (fhdr_t*)NULL)
                    printf("0x%08x", (int) __fhdr_to_khdr(klob_g.islot_flp[i]));
                else
                    printf("          ");
#endif
                printf("] : %6lu %6lu %10lu %10lu\n",
                       klob_g.islot_ctx[i].cnt, klob_g.islot_ctx[i].hwm,
                       klob_g.islot_ctx[i].del, klob_g.islot_ctx[i].ins);
            }

            printf("\nPower Slot Sequence:\n"
                 "\t%4s  %18s  %12s : %6s %6s %10s %10s\n",
                 "Slot", "Block Size Range", "FreeList Ptr",
                 "Count", "HighWM", "Deletes", "Inserts");
            for (i=0; i<KLOB_PSLOT_NUM; i++)
            {
                printf("\t%4d   %6u .. %6d   [", i,
                       klob_g.pslot_ctx[i].rbgn, (int)klob_g.pslot_ctx[i].rend);
                if (klob_g.pslot_flp[i] != (fhdr_t*)NULL)
                    printf("0x%08x", (int) __fhdr_to_khdr(klob_g.pslot_flp[i]));
                else
                    printf("          ");
                printf("] : %6lu %6lu %10lu %10lu\n",
                       klob_g.pslot_ctx[i].cnt, klob_g.pslot_ctx[i].hwm,
                       klob_g.pslot_ctx[i].del, klob_g.pslot_ctx[i].ins);
            }
            printf("\t%4d  %18s  [0x%08x]\n",
                   KLOB_PSLOT_NUM, "Demarcation FList",
                   (int) klob_g.pslot_flp[KLOB_PSLOT_NUM]);
        }
    );

    klobappl_dump_blist(verbose);
    klobappl_dump_flist(verbose);

    KSTATS(
        printf("\nKLOB Statistics:\n"
               "\tBlocks[%lu] : AllocatedBlocks[%lu] "
               "+ FreeBlocks[%lu = I<%lu> + P<%lu>]\n"
               "\tAllocations : [%llu = I<%llu> + P<%llu>],"
               " CummulativeBytes[%llu]\n"
               "\tFrees       : [%llu] Splits[%llu] Coalesces[%llu]\n"
               "\tFreeList HWM: [%lu]\n"
               "\tFailed Extensions[%lu]\n",
               klob_g.bcnt, klob_g.bcnt - (klob_g.icnt + klob_g.pcnt),
               (klob_g.icnt + klob_g.pcnt), klob_g.icnt, klob_g.pcnt,
               (klob_g.iallocs+klob_g.pallocs), klob_g.iallocs, klob_g.pallocs,
               klob_g.bytes, klob_g.frees, klob_g.splits, klob_g.coalesces,
               klob_g.fhwm, klob_g.failexts );

        printf("\tOverflow blocks: Allocs[%lu] Frees[%lu] Bytes[%llu]\n",
               klob_g.fwd_allocs, klob_g.fwd_frees, klob_g.fwd_bytes);
    );

    printf("\nKLOB Version: %d\n"
            "\tMemory managed (bytes): Size[%lu] Free[%lu]\n",
            KLOB_VERSION, klob_g.memory_sz, klob_g.free_sz);

    for (p=1; p<=klob_g.numpools; p++)
    {
        bgn_p = (unsigned long)klob_g.pool[p].pbgn_p;
        end_p = (unsigned long)klob_g.pool[p].pend_p;
        printk("\t\tPool[%2u] [%7lu] bytes [ 0x%08x .. 0x%08x ]\n",
               p, end_p - bgn_p, (int)bgn_p, (int)end_p );
    }

}
#endif

#ifdef CONFIG_PROC_FS
/*
 *------------------------------------------------------------------------------
 *
 *  Dump a bitmap into a sequential file
 *
 *------------------------------------------------------------------------------
 */
void seq_put_map(struct seq_file *s, unsigned long word)
{
    int i;
    unsigned long mask = 1UL << 31;
    seq_putc(s, '\t');
    for (i=1; i<=32; i++, mask = mask >> 1)
    {
        if (word & mask) seq_putc(s, '1'); else seq_putc(s, '0');
        if ((i % 8) == 0) seq_puts(s, "  ");
    }
    seq_putc(s, '\n');
}

/*
 *------------------------------------------------------------------------------
 *
 * Dump the KLOB state into fs/proc/klob
 *
 *------------------------------------------------------------------------------
 */
static inline void klob_seq_show(struct seq_file *s)
{
    unsigned long lock_flag;
    unsigned long i, islot_map[KLOB_ISLOT_MAP], pslot_map;
    unsigned int khdr_sz, free_bytes, allocated_bytes, free_maxbytes,
       allocated_ibytes, allocated_pbytes, free_ibytes, free_pbytes,
       blocks, free_blocks, allocated_iblocks, allocated_pblocks,
       free_iblocks, free_pblocks, p;
    unsigned long bgn_p, end_p;
    bhdr_t * bhdr_p;

    khdr_sz = free_bytes = allocated_bytes = free_maxbytes
    = allocated_ibytes = allocated_pbytes = free_ibytes = free_pbytes
    = blocks = free_blocks = allocated_iblocks = allocated_pblocks
    = free_iblocks = free_pblocks = 0;

    seq_printf(s, "KLOB - Version: %d\n\n"
       "\tMemory managed (bytes): Size[%lu] Free[%lu]\n",
        KLOB_VERSION, klob_g.memory_sz, klob_g.free_sz);

    for (p=1; p<=klob_g.numpools; p++)
    {
        bgn_p = (unsigned long)klob_g.pool[p].pbgn_p;
        end_p = (unsigned long)klob_g.pool[p].pend_p;
        seq_printf(s, "\t\tPool[%u] [%7lu] bytes [ 0x%08x .. 0x%08x ]\n",
            p, end_p - bgn_p, (int)bgn_p, (int)end_p);
    }

#ifdef KLOB_AUDIT
    if (klob_audit(0) != 0)
       seq_puts(s, "ERROR: klob_seq_show klob_audit FAILED !!!\n");
    else
       seq_puts(s, "\n\tklob_audit SUCCESS\n");
#endif

    local_irq_save(lock_flag);/* ---> KLOB CRITICAL SECTION BEGIN */

    for (i=0; i<KLOB_ISLOT_MAP; i++)
        islot_map[i] = klob_g.islot_map[i];
    pslot_map = klob_g.pslot_map;

    bhdr_p = klob_g.blist.next_p;
    while (bhdr_p != & klob_g.blist)
    {
        blocks++;

        if (__is_kpool(__bhdr_to_khdr(bhdr_p)))
        {
#if (CONFIG_KLOB_ALIGN > 8)
            khdr_sz = CONFIG_KLOB_ALIGN;	/* No data */
#else
            khdr_sz = sizeof(khdr_t);	/* No data */
#endif
        }
        else
        {
            khdr_sz = __khdr_size(__bhdr_to_khdr(bhdr_p));
        }

        if (__khdr_is_free(__bhdr_to_khdr(bhdr_p)))
        {
            free_blocks++;
            free_bytes += khdr_sz;

            if (khdr_sz < KLOB_ISLOT_LMT + __mem_hdr_size())
            {
                free_iblocks++;
                free_ibytes += khdr_sz;
            }
            else
            {
                free_pblocks++;
                free_pbytes += khdr_sz;
            }

            if (khdr_sz > free_maxbytes)
               free_maxbytes = khdr_sz;
        }
        else
        {
            allocated_bytes += khdr_sz;

            if (khdr_sz < KLOB_ISLOT_LMT + __mem_hdr_size())
            {
                allocated_iblocks++;
                allocated_ibytes += khdr_sz;
            }
            else
            {
                allocated_pblocks++;
                allocated_pbytes += khdr_sz;
            }
        }

        bhdr_p = __khdr_to_bhdr(__khdr_next(__bhdr_to_khdr(bhdr_p)));
    }

    local_irq_restore(lock_flag);/* <--- KLOB CRITICAL SECTION END */

    seq_printf(s, "\n\tTotal Bytes[%u] = Allocated[%u] + Free[%u]\n"
       "\t\t Allocated ISlot[%u]\n"
       "\t\t Allocated PSlot[%u]\n"
       "\t\t Free ISlot[%u]\n"
       "\t\t Free PSlot[%u]\n"
       "\t\t Max Free[%u]\n",
       allocated_bytes + free_bytes, allocated_bytes, free_bytes,
       allocated_ibytes, allocated_pbytes,
       free_ibytes, free_pbytes, free_maxbytes);

    seq_printf(s, "\n\tTotal Blocks[%u] = Allocated[%u] + Free[%u]\n"
       "\t\t Allocated ISlot[%u]\n"
       "\t\t Allocated PSlot[%u]\n"
       "\t\t Free ISlot[%u]\n"
       "\t\t Free PSlot[%u]\n",
       blocks, blocks - free_blocks, free_blocks,
       allocated_iblocks, allocated_pblocks,
       free_iblocks, free_pblocks);

    seq_printf(s, "\n\tOverhead [%u] bytes (incld in total bytes)\n"
                  "\n\tExtends Allowed : [%lu]\n",
              (blocks - free_blocks) * __mem_hdr_size(), klob_g.extend );

    seq_puts(s, "\n\tIncremental Slot Sequence Map:\n");
    for (i=0; i<KLOB_ISLOT_MAP; i++)
    {
        seq_printf(s, "\t\t%3lu .. %3lu :\t", (i*32)+31, (i*32));
        seq_put_map(s, islot_map[i]);
    }
    seq_puts(s, "\n\tPower Slot Sequence Map:\n\t\t\t\t");
    seq_put_map(s, pslot_map);

    KSTATS(
        seq_printf(s,
               "\n\nKLOB Statistics:\n"
               "\tBlocks[%lu] : AllocatedBlocks[%lu] "
               "+ FreeBlocks[%lu = I<%lu> + P<%lu>]\n"
               "\tAllocations : [%llu = I<%llu> + P<%llu>],"
               " CummulativeBytes[%llu]\n"
               "\tFrees       : [%llu] Splits[%llu] Coalesces[%llu]\n"
               "\tFreeList HWM: [%lu]\n"
               "\tFailed Extensions[%lu]\n",
               klob_g.bcnt, klob_g.bcnt - (klob_g.icnt + klob_g.pcnt),
               (klob_g.icnt + klob_g.pcnt), klob_g.icnt, klob_g.pcnt,
               (klob_g.iallocs+klob_g.pallocs), klob_g.iallocs, klob_g.pallocs,
               klob_g.bytes, klob_g.frees, klob_g.splits, klob_g.coalesces,
               klob_g.fhwm, klob_g.failexts );

        seq_printf(s,
               "\tOverflow blocks: Allocs[%lu] Frees[%lu] Bytes[%llu]\n",
               klob_g.fwd_allocs, klob_g.fwd_frees, klob_g.fwd_bytes);
    );

}

static void * k_start(struct seq_file *s, loff_t * pos)
{
    loff_t n = * pos;
    if (!n)
    {
        klob_seq_show(s);
    }
    return pos;
}

static void * k_next(struct seq_file *s, void *p, loff_t * pos) { return NULL; }
static void k_stop(struct seq_file *s, void *p) {}
static int k_show(struct seq_file *s, void *p) { return 0; }

struct seq_operations klobinfo_op = {
    .start = k_start,
    .next  = k_next,
    .stop  = k_stop,
    .show  = k_show,
};
#endif

/*
 *------------------------------------------------------------------------------
 *
 * klob_init: Initialize KLOB with preconfigured memory
 *
 * Initialize the KLOB state variables and statistics.
 * - Block list and free list are set to empty.
 * - Bit maps for I-Slot and P-Slots are tagged empty.
 * _ Demarcation bits for I-Slot and P-Slots are set.
 * - Debug context for I-Slot and P-Slots are initialized.
 * - The managed memory is partitioned into boundary blocks and a middle
 *   free block which is inserted into its appropriate slot.
 *
 *------------------------------------------------------------------------------
 */
void __init klob_init(void)
{

    khdr_t * bgn_p, * mid_p, * end_p;
    void * klob_mem_p;
    unsigned int i, ix, mem_sz;
#ifndef KLOB_TEST_APPL
    unsigned long klob_order;
#endif
#ifdef CONFIG_KLOB_ISLOT_SFLIST
    fhdr_t * flist_p, * fblk_p;
#endif

	/* KLOB state, pointers, stats maps */
    memset((void*)& klob_g, 0, sizeof(klob_ctx_t));

    klob_g.extend = 1;
    for (i=0; i<=CONFIG_KLOB_MAX_EXTN; i++)
    {
        klob_g.pool[i].pbgn_p = (void*)(0xFFFFFFFF);
        klob_g.pool[i].pend_p = (void*)0x0;
    }

#ifdef CONFIG_KLOB_ISLOT_SFLIST
	/* Initialize each I-Slot's free list to be empty */
    for (i=0; i<KLOB_ISLOT_NUM + KLOB_BITS32; i++)
    {
        flist_p = & klob_g.islot_flist[i];
        flist_p->next_p = flist_p->prev_p = flist_p;
    }
#endif
	/* Set the demarcation of the I and P slots */
    klob_g.islot_map[KLOB_ISLOT_MAP] = ~(0UL);
#if (KLOB_PSLOT_NUM == 8)
    klob_g.pslot_map = 0xFFFFFF00;
#else
    klob_g.pslot_map = ... FAIL COMPILATION HERE ...;
#endif

	/* P-Slot Demarcations' Free List pointer points to end of freelist*/
    for (i=8; i<32; i++)
        klob_g.pslot_flp[i] = &klob_g.flist;

    KDBGB(
	/* Setup the begin and end regions for each slot */
        mem_sz = 0;
        for (i=0; i<KLOB_ISLOT_NUM; i++)
        {
            klob_g.islot_ctx[i].rbgn = mem_sz;
            mem_sz += KLOB_ISLOT_OFF;
            klob_g.islot_ctx[i].rend = mem_sz - 1;
        }
        mem_sz = KLOB_ISLOT_LMT;
        for (i=0; i<KLOB_PSLOT_NUM; i++)
        {
            klob_g.pslot_ctx[i].rbgn = mem_sz;
            mem_sz = mem_sz << 1;
            klob_g.pslot_ctx[i].rend = mem_sz - 1;
        }
        klob_g.pslot_ctx[KLOB_PSLOT_NUM-1].rend = ~0;
    );

	/* Allocate the memory managed by KLOB */

#ifdef KLOB_TEST_APPL
    klob_mem_p = (void*) malloc(CONFIG_KLOB_MEM_SIZE);
#else
	/* Convert memory size in bytes to GFP order */
    klob_order = get_order(CONFIG_KLOB_MEM_SIZE);
    klob_mem_p = (void*) __get_free_pages(GFP_KERNEL|GFP_DMA, klob_order);

    if ( ! klob_mem_p )
       panic("Out of memory detected\n");
#endif

    klob_g.memory_sz = CONFIG_KLOB_MEM_SIZE;
    klob_g.numpools = 1;

	/* Setup the range of the memory managed by KLOB */
    klob_g.pool[klob_g.numpools].pbgn_p = (void*)klob_mem_p;
    klob_g.pool[klob_g.numpools].pend_p
        = (void*)(((unsigned int)klob_mem_p) + CONFIG_KLOB_MEM_SIZE);

	/*
	 * Split the memory into 3 blocks.
	 * The first and last blocks serve as boundary blocks
	 * The middle block is tagged as free and inserted into free list
	 *
	 * Initialize block list and place three blocks in block list.
	 * Initialize free list and place middle block in free list.
	 */
    bgn_p = (khdr_t*)klob_mem_p;
#if (CONFIG_KLOB_ALIGN > 8)
    mid_p = (khdr_t*)((unsigned int)klob_mem_p + CONFIG_KLOB_ALIGN);
    end_p = (khdr_t*)((unsigned int)klob_mem_p
                          + CONFIG_KLOB_MEM_SIZE - CONFIG_KLOB_ALIGN);
#else
    mid_p = (khdr_t*)((unsigned int)klob_mem_p + sizeof(khdr_t));
    end_p = (khdr_t*)((unsigned int)klob_mem_p
                          + CONFIG_KLOB_MEM_SIZE - sizeof(khdr_t));
#endif

    klob_g.pool[klob_g.numpools].hkhdr_p = bgn_p;
    klob_g.pool[klob_g.numpools].tkhdr_p = end_p;

    klob_g.blist.prev_p = (bhdr_t*)end_p;
    klob_g.blist.next_p = (bhdr_t*)bgn_p;

    bgn_p->bhdr.prev_p = & klob_g.blist;
    bgn_p->bhdr.next_p = (bhdr_t*)mid_p;
    __khdr_tag_used(bgn_p);			/* TAG: bgn as INUSE */

    mid_p->bhdr.prev_p = (bhdr_t*)bgn_p;
    mid_p->bhdr.next_p = (bhdr_t*)end_p;

	/* Insert mid block into free list */
    klob_g.flist.next_p = klob_g.flist.prev_p = & mid_p->fhdr;
    mid_p->fhdr.next_p = mid_p->fhdr.prev_p = & klob_g.flist;
    __khdr_tag_free(mid_p);			/* TAG: mid as FREE */

    end_p->bhdr.prev_p = (bhdr_t*)mid_p;
    end_p->bhdr.next_p = & klob_g.blist;
    __khdr_tag_used(end_p);			/* TAG: end as INUSE */

    KSTATS( klob_g.bcnt = 3;);

	/*
	 * The boundary blocks bgn and end are linked together.
	 * If ever, we need to extend KLOB, we could link each
	 * extension together. This would impact the cost on freeing
	 * when we need to determine whether the address falls within
	 * a KLOB managed memory. see __is_mem_klob().
	 */
    bgn_p->fhdr.next_p = &end_p->fhdr;
    bgn_p->fhdr.prev_p = &bgn_p->fhdr;
    end_p->fhdr.next_p = &end_p->fhdr;
    end_p->fhdr.prev_p = &bgn_p->fhdr;

	/* Explicitly place the middle free block into the appropriate slot */
    mem_sz = __khdr_mem_size(mid_p);
    klob_g.free_sz = mem_sz;

    if (mem_sz < KLOB_ISLOT_LMT)
    {
        ix = __memsz_to_islot(mem_sz);
#ifdef CONFIG_KLOB_ISLOT_SFLIST
	/* Insert the free block into the I-Slot's double-ended free list */
        flist_p = & klob_g.islot_flist[ix];
        fblk_p = &mid_p->fhdr;

        fblk_p->prev_p = fblk_p->next_p = flist_p;
        flist_p->prev_p = flist_p->next_p = fblk_p;
#else
        klob_g.islot_flp[ix] = &mid_p->fhdr;
#endif
        __set_islot(ix);

        KSTATS( klob_g.icnt = 1; );
        KDBGB( klob_g.islot_ctx[ix].cnt = 1; );
    }
    else
    {
        ix = __memsz_to_pslot(mem_sz);
        klob_g.pslot_flp[ix] = &mid_p->fhdr;
        __set_pslot(ix);

        KSTATS( klob_g.pcnt = 1; );
        KDBGB( klob_g.pslot_ctx[ix].cnt = 1; );
    }

    printk("KLOB Pool 1 Initialized: %lu bytes <0x%08x ... 0x%08x>\n",
               klob_g.memory_sz,
               (int)klob_g.pool[klob_g.numpools].pbgn_p,
               (int)klob_g.pool[klob_g.numpools].pend_p);

    KDBGB( printk("\tAllocatable block size %d\n", __khdr_mem_size(mid_p)); );

}

/*
 *------------------------------------------------------------------------------
 *
 * klob_extend : Extend the KLOB memory region by an additional pool of memory
 * See klob_init()
 *------------------------------------------------------------------------------
 */

void * klob_extend(size_t mem_sz, int gfp)
{
    unsigned long lock_flag;
    void * mem_p;
    khdr_t * bgn_p, * mid_p, * end_p;
    bhdr_t * tail_p;
    void * klob_mem_p;
#ifndef KLOB_TEST_APPL
    unsigned long klob_order;
#endif

#ifdef KLOB_TEST_APPL
    klob_mem_p = (void*) malloc(CONFIG_KLOB_MEM_EXTN);
#else

	/* Convert memory size in bytes to GFP order */
    klob_order = get_order(CONFIG_KLOB_MEM_EXTN);

    klob_mem_p = (void*) __get_free_pages(GFP_KERNEL|GFP_DMA|__GFP_NOWARN,
                                          klob_order);

    if (!klob_mem_p)
    {
        return NULL;
    }
#endif

    local_irq_save( lock_flag );/* ---> KLOB CRITICAL SECION BEGIN */

    klob_g.memory_sz += CONFIG_KLOB_MEM_EXTN;
    klob_g.numpools++;

	/* Setup the range of the memory managed by KLOB */
    klob_g.pool[klob_g.numpools].pbgn_p = (void*)klob_mem_p;
    klob_g.pool[klob_g.numpools].pend_p
        = (void*)(((unsigned int)klob_mem_p) + CONFIG_KLOB_MEM_EXTN);
    
	/*
	 * Split the memory into 3 blocks.
	 * The first and last blocks serve as boundary blocks
	 * The middle block is tagged as free and inserted into free list
	 */

    bgn_p = (khdr_t*)klob_mem_p;
#if (CONFIG_KLOB_ALIGN > 8)
    mid_p = (khdr_t*)((unsigned int)klob_mem_p + CONFIG_KLOB_ALIGN);
    end_p = (khdr_t*)((unsigned int)klob_mem_p
                          + CONFIG_KLOB_MEM_EXTN - CONFIG_KLOB_ALIGN);
#else
    mid_p = (khdr_t*)((unsigned int)klob_mem_p + sizeof(khdr_t));
    end_p = (khdr_t*)((unsigned int)klob_mem_p
                          + CONFIG_KLOB_MEM_EXTN - sizeof(khdr_t));
#endif

    klob_g.pool[klob_g.numpools].hkhdr_p = bgn_p;
    klob_g.pool[klob_g.numpools].tkhdr_p = end_p;
	/* 
	 * Chain the three blocks into the block list
	 */
    tail_p = klob_g.blist.prev_p;
    __khdr_set_next( __bhdr_to_khdr(tail_p), bgn_p );

    bgn_p->bhdr.prev_p = tail_p;
    bgn_p->bhdr.next_p = (bhdr_t*)mid_p;
    __khdr_tag_used(bgn_p);                     /* TAG: bgn_p as INUSE */

    mid_p->bhdr.prev_p = (bhdr_t*)bgn_p;
    mid_p->bhdr.next_p = (bhdr_t*)end_p;

    end_p->bhdr.prev_p = (bhdr_t*)mid_p;
    end_p->bhdr.next_p = & klob_g.blist;
    __khdr_tag_used(end_p);        		/* TAG: end_p as INUSE */

    klob_g.blist.prev_p = (bhdr_t*)end_p;

    KSTATS( klob_g.bcnt += 3; );

        /* Insert mid block into free list */
    __khdr_tag_free(mid_p);	/* Tag before insert !!! */
    _khdr_insert(mid_p, __khdr_mem_size(mid_p), 1);

	/*
	 * The boundary blocks bgn and end are linked together.
	 * If ever, we need to extend KLOB, we could link each
	 * extension together. This would impact the cost on freeing
	 * when we need to determine whether the address falls within
	 * a KLOB managed memory. see __is_mem_klob().
	 */
    bgn_p->fhdr.next_p = &end_p->fhdr;
    bgn_p->fhdr.prev_p = &bgn_p->fhdr;
    end_p->fhdr.next_p = &end_p->fhdr;
    end_p->fhdr.prev_p = &bgn_p->fhdr;

    KDBGB(
       printk("%d. Extended by %d, memory[%lu]\n",
                 klob_g.numpools, CONFIG_KLOB_MEM_EXTN, klob_g.memory_sz);

       if (klob_audit(0) < 0)
       {
           printk("FAILURE post-klob_extend %d, klob_audit\n", mem_sz);
           exit(-1);
       }

    );

    local_irq_restore( lock_flag );/* <--- KLOB CRITICAL SECION END */

    printk("KLOB extended to %d pools\n", klob_g.numpools );

    mem_p = klob_alloc(mem_sz, gfp);	/* recursive call */

    return mem_p;
}


/*----------------------------------------------------------------------------*/

void *kmalloc(size_t size, int gfp)
{
    void * mem_p;

    KDBGB(
        int errors;
        if ((errors=klob_audit(0)) < 0)
        {
            printk("FAILURE pre-kmalloc %d, klob_audit %d\n",
                   size, -errors);
            exit(-errors);
        }
    );

    KTEST( kmalloc_count++;);

    mem_p = klob_alloc(size, gfp);

    kmalloc_account(mem_p, klob_size(mem_p), size);

    KDBGB(
        if (size == 0)
            printk("\t WARNING: 0x%08x = kmalloc(0)\n", (int)mem_p);
    );

    return mem_p;
}
EXPORT_SYMBOL(kmalloc);


void kfree(const void * mem_p)
{
    KDBGB(
        int errors;
        if ((errors=klob_audit(0)) < 0)
        {
            printk("FAILURE pre-kfree 0x%08x, klob_audit %d\n",
                    (int)mem_p, -errors);
            exit(-errors);
        }

        if (mem_p == (void*)NULL)
            printk("\t WARNING: kfree( NULL )\n");
    );

    KTEST(kfree_count++;);

    kfree_account(mem_p, klob_size(mem_p));

    // klob_audit(0);

    klob_free(mem_p);

}
EXPORT_SYMBOL(kfree);


unsigned int ksize(const void * mem_p)
{
    return klob_size(mem_p);
}
EXPORT_SYMBOL(ksize);

#ifdef KLOB_TEST_APPL

extern void testklob();		/* See kmalloc_trace.c */

int main()
{
    int errors;

    klob_init();

    printk("\n=============== TEST START ===============\n\n");
    testklob();
    printk("\n============= TEST END ==============\n\n");

    klobappl_show(3);

    printk("\n\nTest Completed: kmalloc[%u] kfree[%u], in use[%d]\n",
            kmalloc_count, kfree_count, (kmalloc_count - kfree_count));

    KSTATS(
       printk("\tFragmentation[%lu]\n\n", (klob_g.icnt + klob_g.pcnt) );
    );


    errors = klob_audit(1);

    if (errors)
        printk("\n============= TEST FAILURE ==============\n\n");
    else
        printk("\n============= TEST SUCCESS ==============\n\n");

    return errors;
}
#endif
