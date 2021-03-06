#ifndef BCM63XX_CPU_H_
#define BCM63XX_CPU_H_

#include <linux/types.h>
#include <linux/init.h>

#include <bcm63xx_regs.h>

/*
 * Macro to fetch bcm63xx cpu id and revision, should be optimized at
 * compile time if only one CPU support is enabled (idea stolen from
 * arm mach-types)
 */
#define BCM6338_CPU_ID		0x6338
#define BCM6345_CPU_ID		0x6345
#define BCM6348_CPU_ID		0x6348
#define BCM6358_CPU_ID		0x6358

void __init bcm63xx_cpu_init(void);
u16 __bcm63xx_get_cpu_id(void);
u16 bcm63xx_get_cpu_rev(void);
unsigned int bcm63xx_get_cpu_freq(void);

#ifdef CONFIG_BCM63XX_CPU_6338
# ifdef bcm63xx_get_cpu_id
#  undef bcm63xx_get_cpu_id
#  define bcm63xx_get_cpu_id()	__bcm63xx_get_cpu_id()
#  define BCMCPU_RUNTIME_DETECT
# else
#  define bcm63xx_get_cpu_id()	BCM6338_CPU_ID
# endif
# define BCMCPU_IS_6338()	(bcm63xx_get_cpu_id() == BCM6338_CPU_ID)
#else
# define BCMCPU_IS_6338()	(0)
#endif

#ifdef CONFIG_BCM63XX_CPU_6345
# ifdef bcm63xx_get_cpu_id
#  undef bcm63xx_get_cpu_id
#  define bcm63xx_get_cpu_id()	__bcm63xx_get_cpu_id()
#  define BCMCPU_RUNTIME_DETECT
# else
#  define bcm63xx_get_cpu_id()	BCM6345_CPU_ID
# endif
# define BCMCPU_IS_6345()	(bcm63xx_get_cpu_id() == BCM6345_CPU_ID)
#else
# define BCMCPU_IS_6345()	(0)
#endif

#ifdef CONFIG_BCM63XX_CPU_6348
# ifdef bcm63xx_get_cpu_id
#  undef bcm63xx_get_cpu_id
#  define bcm63xx_get_cpu_id()	__bcm63xx_get_cpu_id()
#  define BCMCPU_RUNTIME_DETECT
# else
#  define bcm63xx_get_cpu_id()	BCM6348_CPU_ID
# endif
# define BCMCPU_IS_6348()	(bcm63xx_get_cpu_id() == BCM6348_CPU_ID)
#else
# define BCMCPU_IS_6348()	(0)
#endif

#ifdef CONFIG_BCM63XX_CPU_6358
# ifdef bcm63xx_get_cpu_id
#  undef bcm63xx_get_cpu_id
#  define bcm63xx_get_cpu_id()	__bcm63xx_get_cpu_id()
#  define BCMCPU_RUNTIME_DETECT
# else
#  define bcm63xx_get_cpu_id()	BCM6358_CPU_ID
# endif
# define BCMCPU_IS_6358()	(bcm63xx_get_cpu_id() == BCM6358_CPU_ID)
#else
# define BCMCPU_IS_6358()	(0)
#endif

#ifndef bcm63xx_get_cpu_id
#error "No CPU support configured"
#endif

/*
 * While registers sets are (mostly) the same across 63xx CPU, base
 * address of these sets do change.
 */
enum bcm63xx_regs_set {
	RSET_DSL_LMEM = 0,
	RSET_PERF,
	RSET_TIMER,
	RSET_WDT,
	RSET_UART0,
	RSET_GPIO,
	RSET_SPI,
	RSET_UDC0,
	RSET_OHCI0,
	RSET_OHCI_PRIV,
	RSET_USBH_PRIV,
	RSET_MPI,
	RSET_PCMCIA,
	RSET_DSL,
	RSET_ENET0,
	RSET_ENET1,
	RSET_ENETDMA,
	RSET_EHCI0,
	RSET_SDRAM,
	RSET_MEMC,
	RSET_DDR,
};

#define RSET_DSL_LMEM_SIZE		(64 * 1024 * 4)
#define RSET_DSL_SIZE			4096
#define RSET_WDT_SIZE			12
#define RSET_ENET_SIZE			2048
#define RSET_ENETDMA_SIZE		2048
#define RSET_UART_SIZE			24
#define RSET_SPI_SIZE			256
#define RSET_UDC_SIZE			256
#define RSET_OHCI_SIZE			256
#define RSET_EHCI_SIZE			256
#define RSET_PCMCIA_SIZE		12

/*
 * 6338 register sets base address
 */

#define BCM_6338_PERF_BASE		(0xfffe0000)
#define BCM_6338_BB_BASE		(0xfffe0100) /* bus bridge registers */
#define BCM_6338_TIMER_BASE		(0xfffe0200)
#define BCM_6338_WDT_BASE		(0xfffe021c)
#define BCM_6338_UART0_BASE		(0xfffe0300)
#define BCM_6338_GPIO_BASE		(0xfffe0400)
#define BCM_6338_SPI_BASE		(0xfffe0c00)
#define BCM_6338_DSL_BASE		(0xfffe1000)
#define BCM_6338_SAR_BASE		(0xfffe2000)
#define BCM_6338_ENETDMA_BASE		(0xfffe2400)
#define BCM_6338_USBDMA_BASE		(0xfffe2400)
#define BCM_6338_ENET0_BASE		(0xfffe2800)
#define BCM_6338_UDC0_BASE		(0xfffe3000) /* USB_CTL_BASE */
#define BCM_6338_MEMC_BASE		(0xfffe3100)

/*
 * 6345 register sets base address
 */
#define BCM_6345_PERF_BASE		(0xfffe0000)
#define BCM_6345_TIMER_BASE		(0xfffe0200)
#define BCM_6345_WDT_BASE		(0xfffe021c)
#define BCM_6345_UART0_BASE		(0xfffe0300)
#define BCM_6345_GPIO_BASE		(0xfffe0400)

/*
 * 6348 register sets base address
 */
#define BCM_6348_DSL_LMEM_BASE		(0xfff00000)
#define BCM_6348_PERF_BASE		(0xfffe0000)
#define BCM_6348_BB_BASE		(0xfffe0100) /* bus bridge registers */
#define BCM_6348_TIMER_BASE		(0xfffe0200)
#define BCM_6348_WDT_BASE		(0xfffe021c)
#define BCM_6348_UART0_BASE		(0xfffe0300)
#define BCM_6348_GPIO_BASE		(0xfffe0400)
#define BCM_6348_SPI_BASE		(0xfffe0c00)
#define BCM_6348_UDC0_BASE		(0xfffe1000)
#define BCM_6348_USBDMA_BASE		(0xfffe1400)
#define BCM_6348_OHCI0_BASE		(0xfffe1b00)
#define BCM_6348_OHCI_PRIV_BASE		(0xfffe1c00)
#define BCM_6348_USBH_PRIV_BASE		(0xdeadbeef)
#define BCM_6348_MPI_BASE		(0xfffe2000)
#define BCM_6348_PCMCIA_BASE		(0xfffe2054)
#define BCM_6348_SDRAM_REGS_BASE	(0xfffe2300)
#define BCM_6348_DSL_BASE		(0xfffe3000)
#define BCM_6348_SAR_BASE		(0xfffe4000)
#define BCM_6348_UBUS_BASE		(0xfffe5000)
#define BCM_6348_ENET0_BASE		(0xfffe6000)
#define BCM_6348_ENET1_BASE		(0xfffe6800)
#define BCM_6348_ENETDMA_BASE		(0xfffe7000)
#define BCM_6348_EHCI0_BASE		(0xdeadbeef)
#define BCM_6348_SDRAM_BASE		(0xfffe2300)
#define BCM_6348_MEMC_BASE		(0xdeadbeef)
#define BCM_6348_DDR_BASE		(0xdeadbeef)

/*
 * 6358 register sets base address
 */
#define BCM_6358_DSL_LMEM_BASE		(0xfff00000)
#define BCM_6358_PERF_BASE		(0xfffe0000)
#define BCM_6358_TIMER_BASE		(0xfffe0040)
#define BCM_6358_WDT_BASE		(0xfffe005c)
#define BCM_6358_GPIO_BASE		(0xfffe0080)
#define BCM_6358_UART0_BASE		(0xfffe0100)
#define BCM_6358_UDC0_BASE		(0xfffe0400)
#define BCM_6358_SPI_BASE		(0xfffe0800)
#define BCM_6358_MPI_BASE		(0xfffe1000)
#define BCM_6358_PCMCIA_BASE		(0xfffe1054)
#define BCM_6358_OHCI0_BASE		(0xfffe1400)
#define BCM_6358_OHCI_PRIV_BASE		(0xdeadbeef)
#define BCM_6358_USBH_PRIV_BASE		(0xfffe1500)
#define BCM_6358_SDRAM_REGS_BASE	(0xfffe2300)
#define BCM_6358_DSL_BASE		(0xfffe3000)
#define BCM_6358_ENET0_BASE		(0xfffe4000)
#define BCM_6358_ENET1_BASE		(0xfffe4800)
#define BCM_6358_ENETDMA_BASE		(0xfffe5000)
#define BCM_6358_EHCI0_BASE		(0xfffe1300)
#define BCM_6358_SDRAM_BASE		(0xdeadbeef)
#define BCM_6358_MEMC_BASE		(0xfffe1200)
#define BCM_6358_DDR_BASE		(0xfffe12a0)


extern const unsigned long *bcm63xx_regs_base;

static inline unsigned long bcm63xx_regset_address(enum bcm63xx_regs_set set)
{
#ifdef BCMCPU_RUNTIME_DETECT
	return bcm63xx_regs_base[set];
#else
#ifdef CONFIG_BCM63XX_CPU_6338
	switch (set) {
	case RSET_PERF:
		return BCM_6338_PERF_BASE;
	case RSET_TIMER:
		return BCM_6338_TIMER_BASE;
	case RSET_WDT:
		return BCM_6338_WDT_BASE;
	case RSET_UART0:
		return BCM_6338_UART0_BASE;
	case RSET_GPIO:
		return BCM_6338_GPIO_BASE;
	case RSET_SPI:
		return BCM_6338_SPI_BASE;
	case RSET_MEMC:
		return BCM_6338_MEMC_BASE;
	}
#endif
#ifdef CONFIG_BCM63XX_CPU_6345
	switch (set) {
	case RSET_PERF:
		return BCM_6345_PERF_BASE;
	case RSET_TIMER:
		return BCM_6345_TIMER_BASE;
	case RSET_WDT:
		return BCM_6345_WDT_BASE;
	case RSET_UART0:
		return BCM_6345_UART0_BASE;
	case RSET_GPIO:
		return BCM_6345_GPIO_BASE;
	}
#endif
#ifdef CONFIG_BCM63XX_CPU_6348
	switch (set) {
	case RSET_DSL_LMEM:
		return BCM_6348_DSL_LMEM_BASE;
	case RSET_PERF:
		return BCM_6348_PERF_BASE;
	case RSET_TIMER:
		return BCM_6348_TIMER_BASE;
	case RSET_WDT:
		return BCM_6348_WDT_BASE;
	case RSET_UART0:
		return BCM_6348_UART0_BASE;
	case RSET_GPIO:
		return BCM_6348_GPIO_BASE;
	case RSET_SPI:
		return BCM_6348_SPI_BASE;
	case RSET_UDC0:
		return BCM_6348_UDC0_BASE;
	case RSET_OHCI0:
		return BCM_6348_OHCI0_BASE;
	case RSET_OHCI_PRIV:
		return BCM_6348_OHCI_PRIV_BASE;
	case RSET_USBH_PRIV:
		return BCM_6348_USBH_PRIV_BASE;
	case RSET_MPI:
		return BCM_6348_MPI_BASE;
	case RSET_PCMCIA:
		return BCM_6348_PCMCIA_BASE;
	case RSET_DSL:
		return BCM_6348_DSL_BASE;
	case RSET_ENET0:
		return BCM_6348_ENET0_BASE;
	case RSET_ENET1:
		return BCM_6348_ENET1_BASE;
	case RSET_ENETDMA:
		return BCM_6348_ENETDMA_BASE;
	case RSET_EHCI0:
		return BCM_6348_EHCI0_BASE;
	case RSET_SDRAM:
		return BCM_6348_SDRAM_BASE;
	case RSET_MEMC:
		return BCM_6348_MEMC_BASE;
	case RSET_DDR:
		return BCM_6348_DDR_BASE;
	}
#endif
#ifdef CONFIG_BCM63XX_CPU_6358
	switch (set) {
	case RSET_DSL_LMEM:
		return BCM_6358_DSL_LMEM_BASE;
	case RSET_PERF:
		return BCM_6358_PERF_BASE;
	case RSET_TIMER:
		return BCM_6358_TIMER_BASE;
	case RSET_WDT:
		return BCM_6358_WDT_BASE;
	case RSET_UART0:
		return BCM_6358_UART0_BASE;
	case RSET_GPIO:
		return BCM_6358_GPIO_BASE;
	case RSET_SPI:
		return BCM_6358_SPI_BASE;
	case RSET_UDC0:
		return BCM_6358_UDC0_BASE;
	case RSET_OHCI0:
		return BCM_6358_OHCI0_BASE;
	case RSET_OHCI_PRIV:
		return BCM_6358_OHCI_PRIV_BASE;
	case RSET_USBH_PRIV:
		return BCM_6358_USBH_PRIV_BASE;
	case RSET_MPI:
		return BCM_6358_MPI_BASE;
	case RSET_PCMCIA:
		return BCM_6358_PCMCIA_BASE;
	case RSET_ENET0:
		return BCM_6358_ENET0_BASE;
	case RSET_ENET1:
		return BCM_6358_ENET1_BASE;
	case RSET_ENETDMA:
		return BCM_6358_ENETDMA_BASE;
	case RSET_DSL:
		return BCM_6358_DSL_BASE;
	case RSET_EHCI0:
		return BCM_6358_EHCI0_BASE;
	case RSET_SDRAM:
		return BCM_6358_SDRAM_BASE;
	case RSET_MEMC:
		return BCM_6358_MEMC_BASE;
	case RSET_DDR:
		return BCM_6358_DDR_BASE;
	}
#endif
#endif
	/* unreached */
	return 0;
}

/*
 * SPI register layout is not compatible
 * accross CPU versions but it is software
 * compatible
 */

enum bcm63xx_regs_spi {
	SPI_CMD,
	SPI_INT_STATUS,
	SPI_INT_MASK_ST,
	SPI_INT_MASK,
	SPI_ST,
	SPI_CLK_CFG,
	SPI_FILL_BYTE,
	SPI_MSG_TAIL,
	SPI_RX_TAIL,
	SPI_MSG_CTL,
	SPI_MSG_DATA,
	SPI_RX_DATA,
};

extern const unsigned long *bcm63xx_regs_spi;

static inline unsigned long bcm63xx_spireg(enum bcm63xx_regs_spi reg)
{
#ifdef BCMCPU_RUNTIME_DETECT
        return bcm63xx_regs_spi[reg];
#else
#ifdef CONFIG_BCM63XX_CPU_6338
switch (reg) {
	case SPI_CMD:
		return SPI_BCM_6338_SPI_CMD;
	case SPI_INT_STATUS:
		return SPI_BCM_6338_SPI_INT_STATUS;
	case SPI_INT_MASK_ST:
		return SPI_BCM_6338_SPI_MASK_INT_ST;
	case SPI_INT_MASK:
		return SPI_BCM_6338_SPI_INT_MASK;
	case SPI_ST:
		return SPI_BCM_6338_SPI_ST;
	case SPI_CLK_CFG:
		return SPI_BCM_6338_SPI_CLK_CFG;
	case SPI_FILL_BYTE:
		return SPI_BCM_6338_SPI_FILL_BYTE;
	case SPI_MSG_TAIL:
		return SPI_BCM_6338_SPI_MSG_TAIL;
	case SPI_RX_TAIL:
		return SPI_BCM_6338_SPI_RX_TAIL;
	case SPI_MSG_CTL:
		return SPI_BCM_6338_SPI_MSG_CTL;
	case SPI_MSG_DATA:
		return SPI_BCM_6338_SPI_MSG_DATA;
	case SPI_RX_DATA:
		return SPI_BCM_6338_SPI_RX_DATA;
}
#endif
#ifdef CONFIG_BCM63XX_CPU_6348
switch (reg) {
	case SPI_CMD:
		return SPI_BCM_6348_SPI_CMD;
	case SPI_INT_MASK_ST:
		return SPI_BCM_6348_SPI_MASK_INT_ST;
	case SPI_INT_MASK:
		return SPI_BCM_6348_SPI_INT_MASK;
	case SPI_INT_STATUS:
		return SPI_BCM_6348_SPI_INT_STATUS;
	case SPI_ST:
		return SPI_BCM_6348_SPI_ST;
	case SPI_CLK_CFG:
		return SPI_BCM_6348_SPI_CLK_CFG;
	case SPI_FILL_BYTE:
		return SPI_BCM_6348_SPI_FILL_BYTE;
	case SPI_MSG_TAIL:
		return SPI_BCM_6348_SPI_MSG_TAIL;
	case SPI_RX_TAIL:
		return SPI_BCM_6348_SPI_RX_TAIL;
	case SPI_MSG_CTL:
		return SPI_BCM_6348_SPI_MSG_CTL;
	case SPI_MSG_DATA:
		return SPI_BCM_6348_SPI_MSG_DATA;
	case SPI_RX_DATA:
		return SPI_BCM_6348_SPI_RX_DATA;
}
#endif
#ifdef CONFIG_BCM63XX_CPU_6358
switch (reg) {
	case SPI_CMD:
		return SPI_BCM_6358_SPI_CMD;
	case SPI_INT_STATUS:
		return SPI_BCM_6358_SPI_INT_STATUS;
	case SPI_INT_MASK_ST:
		return SPI_BCM_6358_SPI_MASK_INT_ST;
	case SPI_INT_MASK:
		return SPI_BCM_6358_SPI_INT_MASK;
	case SPI_ST:
		return SPI_BCM_6358_SPI_STATUS;
	case SPI_CLK_CFG:
		return SPI_BCM_6358_SPI_CLK_CFG;
	case SPI_FILL_BYTE:
		return SPI_BCM_6358_SPI_FILL_BYTE;
	case SPI_MSG_TAIL:
		return SPI_BCM_6358_SPI_MSG_TAIL;
	case SPI_RX_TAIL:
		return SPI_BCM_6358_SPI_RX_TAIL;
	case SPI_MSG_CTL:
		return SPI_BCM_6358_MSG_CTL;
	case SPI_MSG_DATA:
		return SPI_BCM_6358_SPI_MSG_DATA;
	case SPI_RX_DATA:
		return SPI_BCM_6358_SPI_RX_DATA;
}
#endif
#endif
	return 0;
}

/*
 * IRQ number changes across CPU too
 */
enum bcm63xx_irq {
	IRQ_TIMER = 0,
	IRQ_UART0,
	IRQ_SPI,
	IRQ_DSL,
	IRQ_UDC0,
	IRQ_ENET0,
	IRQ_ENET1,
	IRQ_ENET_PHY,
	IRQ_OHCI0,
	IRQ_EHCI0,
	IRQ_PCMCIA0,
	IRQ_ENET0_RXDMA,
	IRQ_ENET0_TXDMA,
	IRQ_ENET1_RXDMA,
	IRQ_ENET1_TXDMA,
	IRQ_PCI,
	IRQ_PCMCIA,
};

/*
 * 6338 irqs
 */
#define BCM_6338_TIMER_IRQ		(IRQ_INTERNAL_BASE + 0)
#define BCM_6338_SPI_IRQ		(IRQ_INTERNAL_BASE + 1)
#define BCM_6338_UART0_IRQ		(IRQ_INTERNAL_BASE + 2)
#define BCM_6338_DG_IRQ			(IRQ_INTERNAL_BASE + 4)
#define BCM_6338_DSL_IRQ		(IRQ_INTERNAL_BASE + 5)
#define BCM_6338_ATM_IRQ		(IRQ_INTERNAL_BASE + 6)
#define BCM_6338_UDC0_IRQ		(IRQ_INTERNAL_BASE + 7)
#define BCM_6338_ENET0_IRQ		(IRQ_INTERNAL_BASE + 8)
#define BCM_6338_ENET_PHY_IRQ		(IRQ_INTERNAL_BASE + 9)
#define BCM_6338_SDRAM_IRQ		(IRQ_INTERNAL_BASE + 10)
#define BCM_6338_USB_CNTL_RX_DMA_IRQ	(IRQ_INTERNAL_BASE + 11)
#define BCM_6338_USB_CNTL_TX_DMA_IRQ	(IRQ_INTERNAL_BASE + 12)
#define BCM_6338_USB_BULK_RX_DMA_IRQ	(IRQ_INTERNAL_BASE + 13)
#define BCM_6338_USB_BULK_TX_DMA_IRQ	(IRQ_INTERNAL_BASE + 14)
#define BCM_6338_ENET0_RXDMA_IRQ	(IRQ_INTERNAL_BASE + 15)
#define BCM_6338_ENET0_TXDMA_IRQ	(IRQ_INTERNAL_BASE + 16)
#define BCM_6338_SDIO_IRQ		(IRQ_INTERNAL_BASE + 17)

/*
 * 6345 irqs
 */
#define BCM_6345_TIMER_IRQ		(IRQ_INTERNAL_BASE + 0)
#define BCM_6345_UART0_IRQ		(IRQ_INTERNAL_BASE + 2)
#define BCM_6345_DSL_IRQ		(IRQ_INTERNAL_BASE + 3)
#define BCM_6345_ATM_IRQ		(IRQ_INTERNAL_BASE + 4)
#define BCM_6345_USB_IRQ		(IRQ_INTERNAL_BASE + 5)
#define BCM_6345_ENET0_IRQ		(IRQ_INTERNAL_BASE + 8)
#define BCM_6345_ENET_PHY_IRQ		(IRQ_INTERNAL_BASE + 12)

/*
 * 6348 irqs
 */
#define BCM_6348_TIMER_IRQ		(IRQ_INTERNAL_BASE + 0)
#define BCM_6348_SPI_IRQ		(IRQ_INTERNAL_BASE + 1)
#define BCM_6348_UART0_IRQ		(IRQ_INTERNAL_BASE + 2)
#define BCM_6348_DSL_IRQ		(IRQ_INTERNAL_BASE + 4)
#define BCM_6348_UDC0_IRQ		(IRQ_INTERNAL_BASE + 6)
#define BCM_6348_ENET1_IRQ		(IRQ_INTERNAL_BASE + 7)
#define BCM_6348_ENET0_IRQ		(IRQ_INTERNAL_BASE + 8)
#define BCM_6348_ENET_PHY_IRQ		(IRQ_INTERNAL_BASE + 9)
#define BCM_6348_OHCI0_IRQ		(IRQ_INTERNAL_BASE + 12)
#define BCM_6348_USB_CNTL_RX_DMA	(IRQ_INTERNAL_BASE + 14)
#define BCM_6348_USB_CNTL_TX_DMA	(IRQ_INTERNAL_BASE + 15)
#define BCM_6348_USB_BULK_RX_DMA	(IRQ_INTERNAL_BASE + 16)
#define BCM_6348_USB_BULK_TX_DMA	(IRQ_INTERNAL_BASE + 17)
#define BCM_6348_USB_ISO_RX_DMA		(IRQ_INTERNAL_BASE + 18)
#define BCM_6348_USB_ISO_TX_DMA		(IRQ_INTERNAL_BASE + 19)
#define BCM_6348_ENET0_RXDMA_IRQ	(IRQ_INTERNAL_BASE + 20)
#define BCM_6348_ENET0_TXDMA_IRQ	(IRQ_INTERNAL_BASE + 21)
#define BCM_6348_ENET1_RXDMA_IRQ	(IRQ_INTERNAL_BASE + 22)
#define BCM_6348_ENET1_TXDMA_IRQ	(IRQ_INTERNAL_BASE + 23)
#define BCM_6348_PCMCIA_IRQ		(IRQ_INTERNAL_BASE + 24)
#define BCM_6348_PCI_IRQ		(IRQ_INTERNAL_BASE + 24)

/*
 * 6358 irqs
 */
#define BCM_6358_TIMER_IRQ		(IRQ_INTERNAL_BASE + 0)
#define BCM_6358_SPI_IRQ		(IRQ_INTERNAL_BASE + 1)
#define BCM_6358_UART0_IRQ		(IRQ_INTERNAL_BASE + 2)
#define BCM_6358_OHCI0_IRQ		(IRQ_INTERNAL_BASE + 5)
#define BCM_6358_ENET1_IRQ		(IRQ_INTERNAL_BASE + 6)
#define BCM_6358_ENET0_IRQ		(IRQ_INTERNAL_BASE + 8)
#define BCM_6358_ENET_PHY_IRQ		(IRQ_INTERNAL_BASE + 9)
#define BCM_6358_EHCI0_IRQ		(IRQ_INTERNAL_BASE + 10)
#define BCM_6358_ENET0_RXDMA_IRQ	(IRQ_INTERNAL_BASE + 15)
#define BCM_6358_ENET0_TXDMA_IRQ	(IRQ_INTERNAL_BASE + 16)
#define BCM_6358_ENET1_RXDMA_IRQ	(IRQ_INTERNAL_BASE + 17)
#define BCM_6358_ENET1_TXDMA_IRQ	(IRQ_INTERNAL_BASE + 18)
#define BCM_6358_DSL_IRQ		(IRQ_INTERNAL_BASE + 29)
#define BCM_6358_PCI_IRQ		(IRQ_INTERNAL_BASE + 31)
#define BCM_6358_PCMCIA_IRQ		(IRQ_INTERNAL_BASE + 24)

extern const int *bcm63xx_irqs;

static inline int bcm63xx_get_irq_number(enum bcm63xx_irq irq)
{
	return bcm63xx_irqs[irq];
}

/*
 * return installed memory size
 */
unsigned int bcm63xx_get_memory_size(void);

#endif /* !BCM63XX_CPU_H_ */
