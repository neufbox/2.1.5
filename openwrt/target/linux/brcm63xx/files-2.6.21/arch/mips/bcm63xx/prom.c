/*
<:copyright-gpl 
 Copyright 2004 Broadcom Corp. All Rights Reserved. 
 
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
 * prom.c: PROM library initialization code.
 *
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <dsp_mod_size.h>

#include <bcm63xx_io.h>
#include <bcm63xx_regs.h>
#include <bcm63xx_board.h>

#ifndef CONFIG_BOARD_NEUFBOX4
extern int  do_syslog(int, char *, int);
extern NVRAM_DATA bootNvramData;
#endif /* !CONFIG_BOARD_NEUFBOX4 */

UINT32 __init calculateCpuSpeed(void);


const char *get_system_type(void)
{
#ifndef CONFIG_BOARD_NEUFBOX4
    return( bootNvramData.szBoardId );
#else
    return board_get_name();
#endif /* !CONFIG_BOARD_NEUFBOX4 */
}


/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */
void __init prom_init(void)
{
	/* stop any running watchdog */
	bcm_wdt_writel(WDT_STOP_1, WDT_CTL_REG);
	bcm_wdt_writel(WDT_STOP_2, WDT_CTL_REG);

	PERF->IrqMask = 0;

	/* Count register increments every other clock */
	mips_hpt_frequency = calculateCpuSpeed() / 2;

	mips_machgroup = MACH_GROUP_BRCM;
	mips_machtype = MACH_BCM963XX;

	/* assign command line from kernel config */
	strcpy(arcs_cmdline, CONFIG_CMDLINE);

	/* do low level board init */
	board_prom_init();
}


/* --------------------------------------------------------------------------
    Name: prom_free_prom_memory
Abstract: 
 -------------------------------------------------------------------------- */
void __init prom_free_prom_memory(void)
{
}


/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM63xx CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fcpu_clk = (25 * MIPSDDR_NDIV) / MIPS_MDIV
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
#if defined(CONFIG_BCM96338)
UINT32 __init calculateCpuSpeed(void)
{
    return 240000000;
}
#endif

#if defined(CONFIG_BCM96348)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllStrap = PERF->PllStrap;
    
    numerator = 64000000 / 4 *
        (((pllStrap & PLL_N1_MASK) >> PLL_N1_SHFT) + 1) *
        (((pllStrap & PLL_N2_MASK) >> PLL_N2_SHFT) + 2);

    cpu_speed = (numerator / (((pllStrap & PLL_M1_CPU_MASK) >> PLL_M1_CPU_SHFT) + 1));

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96358)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;

    numerator = 64000000 / 4 *
        ((pllConfig & MIPSDDR_N2_MASK) >> MIPSDDR_N2_SHFT) * 
        ((pllConfig & MIPSDDR_N1_MASK) >> MIPSDDR_N1_SHFT);

    cpu_speed = numerator / ((pllConfig & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96368)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;
    UINT32 pllMDiv = DDR->MIPSDDRPLLMDiv;

    numerator = 64000000 / ((pllConfig & MIPSDDR_P1_MASK) >> MIPSDDR_P1_SHFT) * 
        ((pllConfig & MIPSDDR_P2_MASK) >> MIPSDDR_P2_SHFT) *
        ((pllConfig & MIPSDDR_NDIV_MASK) >> MIPSDDR_NDIV_SHFT);

    cpu_speed = numerator / ((pllMDiv & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96816)
UINT32 __init calculateCpuSpeed(void)
{
    return 400000000;
}
#endif
