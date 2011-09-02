#ifndef BCM63XX_IRQ_H_
#define BCM63XX_IRQ_H_

#define IRQ_MIPS_BASE			0
#define IRQ_INTERNAL_BASE		8

/*
 * IRQ external base
 * TODO : runtime macro
 */
#ifdef CONFIG_BCM63XX_CPU_6348
#define IRQ_EXT_BASE 			(IRQ_MIPS_BASE + 3)
#endif
#ifdef CONFIG_BCM63XX_CPU_6358
#define IRQ_EXT_BASE			(IRQ_MIPS_BASE + 33)
#endif

#define IRQ_EXT_0			(IRQ_EXT_BASE + 0)
#define IRQ_EXT_1			(IRQ_EXT_BASE + 1)
#define IRQ_EXT_2			(IRQ_EXT_BASE + 2)
#define IRQ_EXT_3			(IRQ_EXT_BASE + 3)

#define IRQ_SOFT_0           0
#define IRQ_SOFT_1           1

#endif /* ! BCM63XX_IRQ_H_ */
