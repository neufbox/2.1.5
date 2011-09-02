/*
 * Original version:
 * From: Matt Mackall <mpm@selenic.com>
 * To: linux-kernel <linux-kernel@vger.kernel.org>, Andrew Morton <akpm@osdl.org>
 * Subject: [PATCH] /proc/kmalloc
 * Date: Sun, 20 Feb 2005 12:47:43 -0800
 * 
 * I've been sitting on this for over a year now, kicking it out in the
 * hopes that someone finds it useful. kernel.org was down when I was
 * tidying this up so it's against 2.6.10 which is what I had handy.
 * 
 * /proc/kmalloc allocation tracing
 *
 * This quick hack adds accounting for kmalloc/kfree callers. This can
 * aid in tracking down memory leaks and large dynamic memory users. The
 * stock version use ~280k of memory for hash tables and can track 32k
 * active allocations.
 *
 * Here's some sample output from my laptop:
 *
 * total bytes allocated: 47118848   
 * slack bytes allocated:  8717262
 * net bytes allocated:    2825920
 * number of allocs:        132796
 * number of frees:         122629
 * number of callers:          325
 * lost callers:                 0
 * lost allocs:                  0
 * unknown frees:                0
 *
 *   total    slack      net alloc/free  caller
 *   24576        0        0     3/3     copy_thread+0x1ad
 *   ...
 *
 * Modification History
 * Dec 12 2006: fabiang@broadcom.com
 * - Original version recorded a cummulative slack per caller and system.
 *   This is not informative of the internal fragmentation (wastage) if the
 *   block is freed. A new stats called netslack is maintained to represent
 *   only the wastage that is currently in use (so to say).
 *   Highwatermark and the application that allocated the largest slack is
 *   also recorded.
 *
 * Jan 7 2007: fabiang@broadcom.com
 * - Inserted rudimentary tracing of kmalloc/kfree invocations at bootup.
 *   The proc fs interface to kmalloc accounting is used with printk to
 *   display the trace.
 */

#include	<linux/config.h>
#include	<linux/seq_file.h>
#include	<linux/kallsyms.h>

	/* BCM Board bootup kmalloc/kfree sequence tracing */
#define KMALLOC_TRACE_BOOTUP

#ifdef KMALLOC_TRACE_BOOTUP

#include <linux/slab.h>   /* GFP_KERNEL */

struct kmalloc_trace_item {
	const void *caller, * addr;
	int size, slack;
};
int kmalloc_display_index = 0;

/*
 * On Boards with larger than 2M of flash,
 * trace buffers are allocated statically.
 * Kludgy ... LAB ONLY !!!
 */
#define KMALLOC_TRACE_SIZE (64 * 1024)
struct kmalloc_trace_item kmalloc_buffer[KMALLOC_TRACE_SIZE];
struct kmalloc_trace_item * kmalloc_trace_buffer_p = kmalloc_buffer;
int kmalloc_trace_index = 0;

/*
 * Tracing stops, when we start to display.
 */
static void kmalloc_trace_display_start(void)
{
	if (kmalloc_display_index == 0) {	/* Stop tracing */
		kmalloc_display_index = kmalloc_trace_index;
		kmalloc_trace_index = KMALLOC_TRACE_SIZE + 1;

		printk("Bootup Trace contains %d records\n\n",
			kmalloc_display_index );
	}
}

static void kmalloc_trace_display_stop(void)
{
	if ((kmalloc_trace_buffer_p != (struct kmalloc_trace_item *) NULL)
            && (kmalloc_display_index < kmalloc_trace_index)) {

                int i = 1;
		struct kmalloc_trace_item * item_p = kmalloc_trace_buffer_p;
		int index = kmalloc_display_index;

                kmalloc_display_index = kmalloc_trace_index;

		printk( "        [ Address ]   Sz=+A|-F  Slack  Caller_Name\n");

		while ( index ) {
			printk( "%6d. 0x%08x %8d %8d [<%08x>]\n",
				i++, (int) item_p->addr, item_p->size,
				item_p->slack, (int)item_p->caller );
			item_p++;
			index--;
		}

		printk("\nDone : total %d items printked\n\n", i-1 );
	}

        kmalloc_display_index = kmalloc_trace_index;
}

static void kmalloc_trace_log(const void *caller, const void *addr,
                              int size, int slack )
{
	struct kmalloc_trace_item * item_p;

	if (kmalloc_trace_index >= KMALLOC_TRACE_SIZE)
		return;

        item_p = kmalloc_trace_buffer_p + kmalloc_trace_index;

	kmalloc_trace_index++; 

	item_p->caller = caller;
	item_p->addr = addr;
	item_p->size = size;
	item_p->slack = slack;
}

#else

#define kmalloc_trace_display_start()
#define kmalloc_trace_display_stop()
#define kmalloc_trace_log(c,a,s,k)

#endif


struct kma_caller {
	const void *caller;
	int total, net, totalslack, netslack, allocs, frees;
};

struct kma_list {
	int callerhash;
	const void *address;
        int blockslack;
};

#define MAX_CALLER_TABLE 512
#define MAX_ALLOC_TRACK 4096

#define kma_hash(address, size) (((u32)address / (u32)size) % size)

static struct kma_list kma_alloc[MAX_ALLOC_TRACK];
static struct kma_caller kma_caller[MAX_CALLER_TABLE];

static int kma_callers;
static int kma_lost_callers, kma_lost_allocs, kma_unknown_frees;
static int kma_total, kma_net, kma_totalslack, kma_netslack, kma_allocs, kma_frees;

static const void * kma_caller_maxslack;
static int kma_maxslack;
static int kma_netslackhwm;

static spinlock_t kma_lock = SPIN_LOCK_UNLOCKED;

void __kmalloc_account(const void *caller, const void *addr, int size, int req)
{
	int i, hasha, hashc;
	unsigned long flags;
        int slack;

	spin_lock_irqsave(&kma_lock, flags);
	if(req >= 0)  /* kmalloc */
	{
                slack = size - req;

                kmalloc_trace_log( caller, addr, size, slack );

                if ( slack > kma_maxslack ) {
                   kma_caller_maxslack = caller;
                   kma_maxslack =  slack;
                }

		/* find callers slot */
		hashc = kma_hash(caller, MAX_CALLER_TABLE);
		for (i = 0; i < MAX_CALLER_TABLE; i++) {
			if (!kma_caller[hashc].caller ||
			    kma_caller[hashc].caller == caller)
				break;
			hashc = (hashc + 1) % MAX_CALLER_TABLE;
		}

		if (!kma_caller[hashc].caller)
			kma_callers++;

		if (i < MAX_CALLER_TABLE) {
			/* update callers stats */
			kma_caller[hashc].caller = caller;
			kma_caller[hashc].total += size;
			kma_caller[hashc].net += size;
			kma_caller[hashc].totalslack += slack;
			kma_caller[hashc].netslack += slack;
			kma_caller[hashc].allocs++;

			/* add malloc to list */
			hasha = kma_hash(addr, MAX_ALLOC_TRACK);
			for (i = 0; i < MAX_ALLOC_TRACK; i++) {
				if (!kma_alloc[hasha].callerhash)
					break;
				hasha = (hasha + 1) % MAX_ALLOC_TRACK;
			}

			if(i < MAX_ALLOC_TRACK) {
				kma_alloc[hasha].callerhash = hashc;
				kma_alloc[hasha].address = addr;
				kma_alloc[hasha].blockslack = slack;
			}
			else
				kma_lost_allocs++;
		}
		else {
			kma_lost_callers++;
			kma_lost_allocs++;
		}

		kma_total += size;
		kma_net += size;
		kma_totalslack += slack;
		kma_netslack += slack;

                if ( kma_netslack > kma_netslackhwm )
                   kma_netslackhwm = kma_netslack;

		kma_allocs++;
	}
	else { /* kfree */
		hasha = kma_hash(addr, MAX_ALLOC_TRACK);
		for (i = 0; i < MAX_ALLOC_TRACK ; i++) {
			if (kma_alloc[hasha].address == addr)
				break;
			hasha = (hasha + 1) % MAX_ALLOC_TRACK;
		}

		if (i < MAX_ALLOC_TRACK) {
			hashc = kma_alloc[hasha].callerhash;
                        slack = kma_alloc[hasha].blockslack;
			kma_alloc[hasha].callerhash = 0;
                        kma_caller[hashc].netslack -= slack;
			kma_caller[hashc].net -= size;
			kma_caller[hashc].frees++;
		}
		else {
			kma_unknown_frees++;
                        slack = 0;
                }

                kmalloc_trace_log( caller, addr, (0 - size), (0 - slack) );

                kma_netslack -= slack;
		kma_net -= size;
		kma_frees++;
	}
	spin_unlock_irqrestore(&kma_lock, flags);
}

static void *as_start(struct seq_file *m, loff_t *pos)
{
	int i;
	loff_t n = *pos;

	if (!n) {

		kmalloc_trace_display_start();

		seq_printf(m, "total bytes allocated: %8d\n"
			      "total slack bytes:     %8d\n\n"
			      "net bytes allocated:   %8d\n"
			      "net slack bytes:       %8d\n"
			      "net slack HighWMark:   %8d\n\n",
			kma_total, kma_totalslack,
			kma_net, kma_netslack, kma_netslackhwm);

		seq_printf(m, "number of allocs:      %8d\n"
			      "number of frees:       %8d\n"
			      "number of callers:     %8d\n\n",
			kma_allocs, kma_frees, kma_callers);

		seq_printf(m, "lost callers:          %8d\n"
			      "lost allocs:           %8d\n"
			      "unknown frees:         %8d\n\n"
			      "maximum slack in block %8d\n"
			      "maximum slack caller   [<%08x>]\n",
			kma_lost_callers, kma_lost_allocs,
			kma_unknown_frees, kma_maxslack,
			(int) kma_caller_maxslack);

		seq_puts(m, "\n   total    slack |      net netslack |"
				" alloc/free  caller\n");
	}

	for (i = 0; i < MAX_CALLER_TABLE; i++) {
		if(kma_caller[i].caller)
			n--;
		if(n < 0)
			return (void *)(i+1);
	}

	return 0;
}

static void *as_next(struct seq_file *m, void *p, loff_t *pos)
{
	int n = (int)p-1, i;
	++*pos;

	for (i = n + 1; i < MAX_CALLER_TABLE; i++)
		if(kma_caller[i].caller)
			return (void *)(i+1);

	return 0;
}

static void as_stop(struct seq_file *m, void *p)
{
	kmalloc_trace_display_stop();
}

static int as_show(struct seq_file *m, void *p)
{
	int n = (int)p-1;
	struct kma_caller *c;
#ifdef CONFIG_KALLSYMS
	char *modname;
	const char *name;
	unsigned long offset = 0, size;
	char namebuf[128];

	c = &kma_caller[n];
	name = kallsyms_lookup((int)c->caller, &size, &offset, &modname,
			       namebuf);
	seq_printf(m, "%8d %8d | %8d %8d | %5d/%-5d %s+0x%lx\n",
		   c->total, c->totalslack, c->net, c->netslack,
                   c->allocs, c->frees, name, offset);
#else
	c = &kma_caller[n];
	seq_printf(m, "%8d %8d | %8d %8d | %5d/%-5d [<%08x>]\n",
		   c->total, c->totalslack, c->net, c->netslack,
                   c->allocs, c->frees, (int) c->caller);
#endif

	return 0;
}

struct seq_operations kmalloc_account_op = {
	.start	= as_start,
	.next	= as_next,
	.stop	= as_stop,
	.show	= as_show,
};
