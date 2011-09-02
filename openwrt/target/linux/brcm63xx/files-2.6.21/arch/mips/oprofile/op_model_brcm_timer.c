/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */
 
#include <linux/init.h>
#include <linux/oprofile.h>
#include <linux/interrupt.h>
#include <linux/smp.h>
#include <linux/percpu.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <asm/ptrace.h>

#include <bcm_intr.h>
#include "op_impl.h"


static DEFINE_PER_CPU(int, op_en) = 0;
static DEFINE_PER_CPU(int, op_timer_int_count) = 0;
static unsigned int op_timer_int_reset = 0;

static void BCM_optimer_reg_setup(struct op_counter_config *ctr)
{
    op_timer_int_reset = ctr->count/(2*loops_per_jiffy);
}

static void BCM_optimer_cpu_setup (void *args)
{
}

static void BCM_optimer_cpu_start(void *args)
{
    __get_cpu_var(op_timer_int_count) = op_timer_int_reset;
    __get_cpu_var(op_en) = 1;
}

static void BCM_optimer_cpu_stop(void *args)
{
    __get_cpu_var(op_en) = 0;
}

void BCM_OProfile_timerhandler(struct pt_regs *regs)
{
    if (__get_cpu_var(op_en)) {
        if (--__get_cpu_var(op_timer_int_count) <= 0) {
            oprofile_add_sample(instruction_pointer(regs),!user_mode(regs),0, smp_processor_id());
            __get_cpu_var(op_timer_int_count) = op_timer_int_reset;
        }
    }
}

static int __init BCM_optimer_init(void)
{
	return 0;
}

static void BCM_optimer_exit(void)
{
}

struct op_mips_model op_model_bcm_optimer = {
	.reg_setup	= BCM_optimer_reg_setup,
	.cpu_setup	= BCM_optimer_cpu_setup,
	.init		= BCM_optimer_init,
	.exit		= BCM_optimer_exit,
	.cpu_start	= BCM_optimer_cpu_start,
	.cpu_stop	= BCM_optimer_cpu_stop,
	.cpu_type	= "mips/bcm63xx",
	.num_counters	= 1
};

