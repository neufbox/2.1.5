#ifndef BCM63XX_IRQ_H_
#define BCM63XX_IRQ_H_

#include <bcm63xx_cpu.h>

#define IRQ_MIPS_BASE			0
#define IRQ_INTERNAL_BASE		8

#define IRQ_EXT_BASE			(IRQ_MIPS_BASE + 3)
#define IRQ_6358_EXT_BASE		(IRQ_MIPS_BASE + 33)

static int inline bcm63xx_external_irq_base(void)
{
	if (BCMCPU_IS_6358())
		return IRQ_6358_EXT_BASE;
	else
		return IRQ_EXT_BASE;
}

#define IRQ_EXT_0			(bcm63xx_external_irq_base() + 0)
#define IRQ_EXT_1			(bcm63xx_external_irq_base() + 1)
#define IRQ_EXT_2			(bcm63xx_external_irq_base() + 2)
#define IRQ_EXT_3			(bcm63xx_external_irq_base() + 3)

#endif /* ! BCM63XX_IRQ_H_ */
