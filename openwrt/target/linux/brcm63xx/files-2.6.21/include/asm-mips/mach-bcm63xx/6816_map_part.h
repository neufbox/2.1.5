/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
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

#ifndef __BCM6816_MAP_H
#define __BCM6816_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#define PERF_BASE            0xb0000000  /* chip control registers */
#define TIMR_BASE            0xb0000040  /* timer registers */
#define GPIO_BASE            0xb0000080  /* gpio registers */
#define UART_BASE            0xb0000100  /* uart registers */
#define UART1_BASE           0xb0000120  /* uart registers */
#define SPI_BASE             0xb0000800  /* SPI master controller registers */
#define MPI_BASE             0xb0002000  /* MPI control registers */
#define USB_CTL_BASE         0xb0002400  /* USB 2.0 device control registers */
#define USB_EHCI_BASE        0x10002500  /* USB host registers */
#define USB_OHCI_BASE        0x10002600  /* USB host registers */
#define USBH_CFG_BASE        0xb0002700
#define MEMC_BASE            0xb0003000  /* Memory control registers */


/***** TBD. This is the BCM6368 definition.  Need BCM6816 definition. *****/
typedef struct MemoryControl
{
    uint32 Control;             /* (00) */
#define MEMC_SELF_REFRESH    (1<<6) // enable self refresh mode
#define MEMC_MRS             (1<<4) // generate a mode register select cycle
#define MEMC_PRECHARGE       (1<<3) // generate a precharge cycle
#define MEMC_REFRESH         (1<<2) // generate an auto refresh cycle
#define MEMC_SEQUENCE_ENABLE (1<<1) // enable memory controller sequencer
#define MEMC_MASTER_ENABLE   (1<<0) // enable accesses to external sdram

    uint32  Config;             /* (04) */
#define MEMC_EARLY_HDR_CNT_SHFT 25
#define MEMC_EARLY_HDR_CNT_MASK (0x7<<MEMC_EARLYHDRCNT_SHFT)
#define MEMC_USE_HDR_CNT        (1<<24)
#define MEMC_EN_FAST_REPLY      (1<<23)
#define MEMC_RR_ARB             (1<<22)
#define MEMC_SFX_NO_MRS2        (1<<21)
#define MEMC_SFX_NO_DLL_RST     (1<<20)
#define MEMC_LLMB_ONE_REQ       (1<<19)
#define MEMC_SYS_PORT_CMD_MODE  (1<<18)
#define MEMC_PAD_OP_MODE        (1<<17)
#define MEMC_DQS_GATE_EN        (1<<16)
#define MEMC_PRED_RD_STROBE_EN  (1<<15)
#define MEMC_PRED_RD_LATENCY_SEL (1<<14)
#define MEMC_UBUS_CLF_EN        (1<<8)

#define MEMC_ROW_SHFT           6
#define MEMC_ROW_MASK           (0x3<<MEMC_ROW_SHFT)
#define MEMC_11BIT_ROW          0
#define MEMC_12BIT_ROW          1
#define MEMC_13BIT_ROW          2
#define MEMC_14BIT_ROW          3

#define MEMC_COL_SHFT           3
#define MEMC_COL_MASK           (0x7<<MEMC_COL_SHFT)
#define MEMC_8BIT_COL           0
#define MEMC_9BIT_COL           1
#define MEMC_10BIT_COL          2
#define MEMC_11BIT_COL          3

#define MEMC_SEL_PRIORITY       (1<<2)

#define MEMC_WIDTH_SHFT         1
#define MEMC_WIDTH_MASK         (0x1<<MEMC_WIDTH_SHFT)
#define MEMC_32BIT_BUS          0
#define MEMC_16BIT_BUS          1

#define MEMC_MEMTYPE_SDR        (0<<0)
#define MEMC_MEMTYPE_DDR        (1<<0)

    uint32  RefreshPdControl;   /* (08) */ 
    uint32  BistStatus;         /* (0C) */
    uint32  ExtendedModeBuffer; /* (10) */
    uint32  BankClosingTimer;   /* (14) */
    uint32  PriorityInversionTimer; /* (18) */

    uint32  DramTiming;         /* (1c) */
#define MEMC_WR_NOP_RD        (1<<23)
#define MEMC_WR_NOP_WR        (1<<22)
#define MEMC_RD_NOP_WR        (1<<21)
#define MEMC_RD_NOP_RD        (1<<20)
#define MEMC_CAS_LATENCY_2    (0)
#define MEMC_CAS_LATENCY_2_5  (1)
#define MEMC_CAS_LATENCY_3    (2)

    uint32  IntStatus;          /* (20) */
    uint32  IntMask;            /* (24) */
#define MEMC_INT3             (1<<3)
#define MEMC_INT2             (2<<3)
#define MEMC_INT1             (1<<3)
#define MEMC_INT0             (0<<3)

    uint32  IntInfo;            /* (28) */
    uint8   unused5[0x50-0x2c]; /* (2c) */
    uint32  Barrier;            /* (50) */
    uint32  CoreId;             /* (54) */
} MemoryControl;

#define MEMC ((volatile MemoryControl * const) MEMC_BASE)

/***** TBD. This is the BCM6368 definition.  Need BCM6816 definition. *****/
typedef struct DDRControl {
    uint32    RevID;            /* 00 */
    uint32    PadSSTLMode;      /* 04 */
    uint32    CmdPadCntl;       /* 08 */
    uint32    DQPadCntl;        /* 0c */
    uint32    DQSPadCntl;       /* 10 */
    uint32    ClkPadCntl0;      /* 14 */
    uint32    MIPSDDRPLLCntl0;  /* 18 */
    uint32    MIPSDDRPLLCntl1;  /* 1c */
    uint32    MIPSDDRPLLConfig; /* 20 */
#define MIPSDDR_NDIV_SHFT       16
#define MIPSDDR_NDIV_MASK       (0x1ff<<MIPSDDR_NDIV_SHFT)
#define REF_MDIV_SHFT           8
#define REF_MDIV_MASK           (0xff<<REF_MDIV_SHFT)
#define MIPSDDR_P2_SHFT         4
#define MIPSDDR_P2_MASK         (0xf<<MIPSDDR_P2_SHFT)
#define MIPSDDR_P1_SHFT         0
#define MIPSDDR_P1_MASK         (0xf<<MIPSDDR_P1_SHFT)
    uint32    MIPSDDRPLLMDiv;   /* 24 */
#define DDR_MDIV_SHFT           8
#define DDR_MDIV_MASK           (0xff<<DDR_MDIV_SHFT)
#define MIPS_MDIV_SHFT          0
#define MIPS_MDIV_MASK          (0xff<<MIPS_MDIV_SHFT)
    uint32    DSLCorePhaseCntl; /* 28 */
#define	DSL_PHY_PI_CNTR_EN	(1 << 20)
#define	DSL_PHY_PI_CNTR_CYCLES_SHIFT	16
#define	DSL_PHY_PI_CNTR_CYCLES_MASK	(0xF << DSL_PHY_PI_CNTR_CYCLES_SHIFT)
    uint32    DSLCpuPhaseCntr;  /* 2c */
#define	DSL_CPU_PI_CNTR_EN	(1 << 20)
#define	DSL_CPU_PI_CNTR_CYCLES_SHIFT	16
#define	DSL_CPU_PI_CNTR_CYCLES_MASK	(0xF << DSL_CPU_PI_CNTR_CYCLES_SHIFT)
    uint32    MIPSPhaseCntl;    /* 30 */
#define	PH_CNTR_EN		(1 << 20)
    uint32    DDR1_2PhaseCntl0;  /* 34 */
    uint32    DDR3_4PhaseCntl0; /* 38 */
    uint32    VCDLPhaseCntl0;   /* 3c */
    uint32    VCDLPhaseCntl1;   /* 40 */
    uint32    WSliceCntl;       /* 44 */
    uint32    DeskewDLLCntl;    /* 48 */
    uint32    DeskewDLLReset;   /* 4c */
    uint32    DeskewDLLPhase;   /* 50 */
    uint32    AnalogTestCntl;   /* 54 */
    uint32    RdDQSGateCntl;    /* 58 */
    uint32    PLLTestReg;       /* 5c */
    uint32    Spare0;           /* 60 */
    uint32    Spare1;           /* 64 */
    uint32    Spare2;           /* 68 */
    uint32    CLBist;           /* 6c */
    uint32    LBistCRC;         /* 70 */
    uint32    UBUSPhaseCntl;    /* 74 */
    uint32    UBUSPIDeskewLLMB0; /* 78 */
    uint32    UBUSPIDeskewLLMB1; /* 7C */

} DDRControl;

#define DDR ((volatile DDRControl * const) DDR_BASE)

/*
** Peripheral Controller
*/
typedef struct PerfControl {
     uint32        RevID;             /* (00) word 0 */
     uint32        blkEnables;        /* (04) word 1 */
#define ACP_A_CLK_EN     (1 << 25)
#define ACP_B_CLK_EN     (1 << 24)
#define NTP_CLK_EN       (1 << 23)
#define PCM_CLK_EN       (1 << 22)
#define BMU_CLK_EN       (1 << 21)
#define PCIE_CLK_EN      (1 << 20)
#define GPON_SER_CLK_EN  (1 << 19)
#define IPSEC_CLK_EN     (1 << 18)
#define NAND_CLK_EN      (1 << 17)
#define DISABLE_GLESS    (1 << 16)
#define USBH_CLK_EN      (1 << 15)
#define APM_CLK_EN       (1 << 14)
//#define UTOPIA_CLK_EN    (1 << 13)
#define ROBOSW_CLK_EN    (1 << 12)
#define USBD_CLK_EN      (1 << 10)
#define SPI_CLK_EN       (1 << 9)
#define SWPKT_SAR_CLK_EN (1 << 8)
#define SWPKT_USB_CLK_EN (1 << 7)
#define GPON_CLK_EN      (1 << 6)
   
     uint32        pll_control;       /* (08) word 2 */
#define SOFT_RESET              0x00000001      // 0

     uint32        deviceTimeoutEn;   /* (0c) word 3 */
     uint32        softResetB;        /* (10) word 4 */
#define SOFT_RST_SPI         0x00000001
#define SOFT_RST_ACP         0x00000002
#define SOFT_RST_EMAC        0x00000004
#define SOFT_RST_PCIE        0x00000008
#define SOFT_RST_FPM         0x00000020
#define SOFT_RST_HMAC        0x00000080
#define SOFT_RST_EMAC1       0x00000200
#define SOFT_RST_EMAC2       0x00000400
#define SOFT_RST_USBS        0x00000800
#define SOFT_RST_APM         0x00001000
#define SOFT_RST_PCM         0x00002000
#define SOFT_RST_HVG         0x00004000
#define SOFT_RST_BMU         0x00008000

     uint32        diagControl;            /* (14) word 5 */
#define DIAG_UBUSARB            1
#define DIAG_USBH               3
#define DIAG_CLKRST             4
#define DIAG_APM                5
#define DIAG_MPI                6
#define DIAG_PERIPH             7
#define DIAG_USBS               8
#define DIAG_GPON               9
#define DIAG_GMAC               10
#define DIAG_MIPS               11
#define DIAG_MEMC               12
#define DIAG_MOCA               13
#define DIAG_GPON_SERDES        14
#define DIAG_PCIE               15
//#define DIAG_DHIF_FEC           16
#define DIAG_UBUSARB_VDSL       17
#define DIAG_IPSEC              18
#define DIAG_ARB_BRIDGE         19

#define DIAG_LO_EN_SHFT         7
#define DIAG_HI_EN_SHFT         7
#define DIAG_LO_CLK_SHFT        12
#define DIAG_HI_CLK_SHFT        12
#define DIAG_SWAP_DATA_EN       0x00010000

    uint32        ExtIrqCfg;          /* (18) word 6*/
#define EI_SENSE_SHFT   0
#define EI_STATUS_SHFT  4
#define EI_CLEAR_SHFT   8
#define EI_MASK_SHFT    12
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   20

    uint32        ExtIrqCfg1;         /* (1c) word 7 */
#define EPHY_RESET_N         0x00000100
#define DSPHY_BYP		0x00000002

     uint32        IrqMask_high;      /* (20) word 8 */
     uint32        IrqMask;           /* (24) word 9 */
     uint32        IrqStatus_high;    /* (28) word 10 */
#define APM_DMA_IRQ5            0x00200000
#define APM_DMA_IRQ4            0x00100000
#define APM_DMA_IRQ3            0x00080000
#define APM_DMA_IRQ2            0x00040000
#define APM_DMA_IRQ1            0x00020000
#define APM_DMA_IRQ0            0x00010000

     uint32        IrqStatus;              /* (2c) word 11 */
//#define PCI_IRQ                 0x80000000
#define APM_IRQ                 0x00001000
#define DS_IRQ2                 0x00200000
#define DS_IRQ1                 0x00100000
#define SAR_IRQ                 0x00080000
#define USB_IRQ_EP_OUT_IRQ     (1<<31)         // usb bulk rx dma
#define USB_IRQ_EP_IN_IRQ      (1<<30)         // usb bulk tx dma
#define USB_BULK_EP_OUT_IRQ    (1<<28)         // usb bulk rx dma
#define USB_BULK_EP_IN_IRQ     (1<<29)         // usb bulk tx dma
#define USB_CNTL_EP_OUT_IRQ    (1<<27)         // usb cntl rx dma
#define USB_CNTL_EP_IN_IRQ     (1<<26)         // usb cntl tx dma

#define TC_IRQ                  0x00000400
#define EPHY_IRQ                0x00000200
#define ETHIRQ                  0x00000100      // emac
#define USB_IRQ                 0x00000100
#define EMAC2_IRQ               0x00000040
#define US_IRQ                  0x00000020      // adsl
#define UART1IRQ                0x00000008
#define UART0IRQ                0x00000004
#define SPIIRQ                  0x00000002
#define TIMRIRQ                 0x00000001

     uint32        IrqMask1_high;     /* (30) word 13 */
     uint32        IrqMask1;          /* (34) word 14*/   
     uint32        IrqStatus1_high;   /* (38) word 15 */
     uint32        IrqStatus1;        /* (3c) word 16 */
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)


/*
** Timer
*/
typedef struct Timer {
    uint16        TimerStatus;
#define TMR_WDCR_STATUS 0x0001
    byte          TimerMask;
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    byte          TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
#define WATCHDOG        0x10
    uint32        TimerCtl0;
    uint32        TimerCtl1;
    uint32        TimerCtl2;
#define TIMERENABLE     0x80000000
#define RSTCNTCLR       0x40000000      
    uint32        TimerCnt0;
    uint32        TimerCnt1;
    uint32        TimerCnt2;
    uint32        WatchDogDefCount;

    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 40-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

/*
** UART
*/
typedef struct UartChannel {
    byte          unused0;
    byte          control;
#define BRGEN           0x80    /* Control register bit defs */
#define TXEN            0x40
#define RXEN            0x20
#define LOOPBK          0x10
#define TXPARITYEN      0x08
#define TXPARITYEVEN    0x04
#define RXPARITYEN      0x02
#define RXPARITYEVEN    0x01

    byte          config;
#define XMITBREAK       0x40
#define BITS5SYM        0x00
#define BITS6SYM        0x10
#define BITS7SYM        0x20
#define BITS8SYM        0x30
#define ONESTOP         0x07
#define TWOSTOP         0x0f
    /* 4-LSBS represent STOP bits/char
     * in 1/8 bit-time intervals.  Zero
     * represents 1/8 stop bit interval.
     * Fifteen represents 2 stop bits.
     */
    byte          fifoctl;
#define RSTTXFIFOS      0x80
#define RSTRXFIFOS      0x40
    /* 5-bit TimeoutCnt is in low bits of this register.
     *  This count represents the number of characters 
     *  idle times before setting receive Irq when below threshold
     */
    uint32        baudword;
    /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
     */

    byte          txf_levl;       /* Read-only fifo depth */
    byte          rxf_levl;       /* Read-only fifo depth */
    byte          fifocfg;        /* Upper 4-bits are TxThresh, Lower are
                                   *      RxThreshold.  Irq can be asserted
                                   *      when rx fifo> thresh, txfifo<thresh
                                   */
    byte          prog_out;       /* Set value of DTR (Bit0), RTS (Bit1)
                                   *  if these bits are also enabled to GPIO_o
                                   */
#define DTREN   0x01
#define RTSEN   0x02

    byte          unused1;
    byte          DeltaIPEdgeNoSense;     /* Low 4-bits, set corr bit to 1 to 
                                           * detect irq on rising AND falling 
                                           * edges for corresponding GPIO_i
                                           * if enabled (edge insensitive)
                                           */
    byte          DeltaIPConfig_Mask;     /* Upper 4 bits: 1 for posedge sense
                                           *      0 for negedge sense if
                                           *      not configured for edge
                                           *      insensitive (see above)
                                           * Lower 4 bits: Mask to enable change
                                           *  detection IRQ for corresponding
                                           *  GPIO_i
                                           */
    byte          DeltaIP_SyncIP;         /* Upper 4 bits show which bits
                                           *  have changed (may set IRQ).
                                           *  read automatically clears bit
                                           * Lower 4 bits are actual status
                                           */

    uint16        intMask;                /* Same Bit defs for Mask and status */
    uint16        intStatus;
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000

    uint16        unused2;
    uint16        Data;                   /* Write to TX, Read from RX */
                                          /* bits 11:8 are BRK,PAR,FRM errors */

    uint32        unused3;
    uint32        unused4;
} Uart;

#define UART ((volatile Uart * const) UART_BASE)

/*
** Gpio Controller
*/

typedef struct GpioControl {
    uint32      GPIODir_high;               /* bits 36:32 */
    uint32      GPIODir;                    /* bits 31:00 */
    uint32      GPIOio_high;                /* bits 36:32 */
    uint32      GPIOio;                     /* bits 31:00 */
    uint32      LEDCtrl;
#define LED_ALL_STROBE          0x0f000000
#define LED3_STROBE             0x08000000
#define LED2_STROBE             0x04000000
#define LED1_STROBE             0x02000000
#define LED0_STROBE             0x01000000
#define LED_TEST                0x00010000
#define DISABLE_LINK_ACT_ALL    0x0000f000
#define DISABLE_LINK_ACT_3      0x00008000
#define DISABLE_LINK_ACT_2      0x00004000
#define DISABLE_LINK_ACT_1      0x00002000
#define DISABLE_LINK_ACT_0      0x00001000
#define LED_INTERVAL_SET_MASK   0x00000f00
#define LED_INTERVAL_SET_1280MS 0x00000700
#define LED_INTERVAL_SET_640MS  0x00000600
#define LED_INTERVAL_SET_320MS  0x00000500
#define LED_INTERVAL_SET_160MS  0x00000400
#define LED_INTERVAL_SET_80MS   0x00000300
#define LED_INTERVAL_SET_40MS   0x00000200
#define LED_INTERVAL_SET_20MS   0x00000100
#define LED_ON_ALL              0x000000f0
#define LED_ON_3                0x00000080
#define LED_ON_2                0x00000040
#define LED_ON_1                0x00000020
#define LED_ON_0                0x00000010
#define LED_ENABLE_ALL          0x0000000f
#define LED_ENABLE_3            0x00000008
#define LED_ENABLE_2            0x00000004
#define LED_ENABLE_1            0x00000002
#define LED_ENABLE_0            0x00000001
    uint32      SpiSlaveCfg;                /* 14 */
    uint32      GPIOMode;                   /* 18 */
/* TBD. Need BCM6816 definitions for this field. */
#if 0
#define GPIO_MODE_SPI_SSN5          (1<<31)
#define GPIO_MODE_SPI_SSN4          (1<<30)
#define GPIO_MODE_SPI_SSN3          (1<<29)
#define GPIO_MODE_SPI_SSN2          (1<<28)
#define GPIO_MODE_EBI_CS3           (1<<27)
#define GPIO_MODE_EBI_CS2           (1<<26)
#define GPIO_MODE_PCMCIA_VS2        (1<<25)
#define GPIO_MODE_PCMCIA_VS1        (1<<24)
#define GPIO_MODE_PCMCIA_CD2        (1<<23)
#define GPIO_MODE_PCMCIA_CD1        (1<<22)
#define GPIO_MODE_PCI_GNT0          (1<<20)
#define GPIO_MODE_PCI_REQ0          (1<<19)
#define GPIO_MODE_PCI_INTB          (1<<18)
#define GPIO_MODE_PCI_GNT1          (1<<17)
#define GPIO_MODE_PCI_REQ1          (1<<16)
#define GPIO_MODE_USBD_LED          (1<<14)
#define GPIO_MODE_ROBOSW_LED1       (1<<13)
#define GPIO_MODE_ROBOSW_LED0       (1<<12)
#define GPIO_MODE_ROBOSW_LED_CLK    (1<<11)
#define GPIO_MODE_ROBOSW_LED_DATA   (1<<10)
#define GPIO_MODE_EPHY3_LED         (1<<9)
#define GPIO_MODE_EPHY2_LED         (1<<8)
#define GPIO_MODE_EPHY1_LED         (1<<7)
#define GPIO_MODE_EPHY0_LED         (1<<6)
#define GPIO_MODE_INET_LED          (1<<5)
#define GPIO_MODE_SERIAL_LED_CLK    (1<<4)
#define GPIO_MODE_SERIAL_LED_DATA   (1<<3)
#define GPIO_MODE_SYS_IRQ           (1<<2)
#define GPIO_MODE_ANALOG_AFE_1      (1<<1)
#define GPIO_MODE_ANALOG_AFE_0      (1<<0)
#endif

    uint32      VregConfig;                 /* 1C */
    uint32      AuxLedInterval;             /* 20 */
#define AUX_LED_IN_7            0x80000000
#define AUX_LED_IN_6            0x40000000
#define AUX_LED_IN_5            0x20000000
#define AUX_LED_IN_4            0x10000000
#define AUX_LED_IN_MASK         0xf0000000
#define LED_IN_3                0x08000000
#define LED_IN_2                0x04000000
#define LED_IN_1                0x02000000
#define LED_IN_0                0x01000000
#define AUX_LED_TEST            0x00400000
#define USE_NEW_INTV            0x00200000
#define LED7_LNK_ORAND          0x00100000
#define LED7_LNK_MASK           0x000f0000
#define LED7_LNK_MASK_SHFT      16
#define LED7_ACT_MASK           0x0000f000
#define LED7_ACT_MASK_SHFT      12
#define AUX_FLASH_INTV          0x00000fc0
#define AUX_FLASH_INTV_SHFT     6
#define AUX_BLINK_INTV          0x0000003f
    uint32      AuxLedCtrl;                 /* 24 */
#define AUX_HW_DISAB_7          0x80000000
#define AUX_STROBE_7            0x40000000
#define AUX_MODE_7              0x30000000
#define AUX_MODE_SHFT_7         28
#define AUX_HW_DISAB_6          0x08000000
#define AUX_STROBE_6            0x04000000
#define AUX_MODE_6              0x03000000
#define AUX_MODE_SHFT_6         24
#define AUX_HW_DISAB_5          0x00800000
#define AUX_STROBE_5            0x00400000
#define AUX_MODE_5              0x00300000
#define AUX_MODE_SHFT_5         20
#define AUX_HW_DISAB_4          0x00080000
#define AUX_STROBE_4            0x00040000
#define AUX_MODE_4              0x00030000
#define AUX_MODE_SHFT_4         16
#define AUX_HW_DISAB_3          0x00008000
#define AUX_STROBE_3            0x00004000
#define AUX_MODE_3              0x00003000
#define AUX_MODE_SHFT_3         12
#define AUX_HW_DISAB_2          0x00000800
#define AUX_STROBE_2            0x00000400
#define AUX_MODE_2              0x00000300
#define AUX_MODE_SHFT_2         8
#define AUX_HW_DISAB_1          0x00000080
#define AUX_STROBE_1            0x00000040
#define AUX_MODE_1              0x00000030
#define AUX_MODE_SHFT_1         4
#define AUX_HW_DISAB_0          0x00000008
#define AUX_STROBE_0            0x00000004
#define AUX_MODE_0              0x00000003
#define AUX_MODE_SHFT_0         0

#define LED_STEADY_OFF          0x0
#define LED_FLASH               0x1
#define LED_BLINK               0x2
#define LED_STEADY_ON           0x3

    uint32      TestControl;                /* 28 */

    uint32	OscControl; //2C:
    uint32    RSWLed;     //30:
    uint32	unused0;	//34:   Read Only Register --> serial shift reg from rsw_ledclk and rsw_leddata
    uint32	base_mode;	//38:
    uint32	ephy_ctl;	//3C:
    uint32	unused1;	//40:
    uint32	unused2;	//44:
    uint32	unused4;	//48:   Ring OSC -->   do not program
    uint32	unused5;	//4C:   Ring OSC -->   do not program

    uint32      SerialLed;                  /* 50 */
    uint32      SerialLedCtrl;              /* 54 */
#define SER_LED_BUSY            (1<<3)
#define SER_LED_POLARITY        (1<<2)
#define SER_LED_DIV_1           0
#define SER_LED_DIV_2           1
#define SER_LED_DIV_4           2
#define SER_LED_DIV_8           3
#define SER_LED_DIV_MASK        0x3
#define SER_LED_DIV_SHIFT       0
    uint32	serial_led_blink;               /* 58 */
    uint32    unused6[3];                   /* 5c */
    uint32    DieRevID;                     /* 68 */
    uint32    DiagMemStatus;                /* 6c */
    uint32    DiagSelControl;               /* 70 */
    uint32    DiagReadBack;                 /* 74 */
    uint32    DiagReadBackHi;               /* 78 */
    uint32    DiagMiscControl;              /* 7c */
#define EPHY_SA_RESET_N           0x00000300
#define EPHY_SA_TESTEN            0x00000500
#define EPHY_SA_CLOCK_RESET       0x0000d900
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_TOTAL_BITS_MASK        0x3f
#define GPIO_NUM_MAX_BITS_MASK          0x1f
#define GPIO_NUM_TO_MASK(X)             ( (((X) & GPIO_NUM_TOTAL_BITS_MASK) < 32) ? (1 << ((X) & GPIO_NUM_MAX_BITS_MASK)) : (0) )

/* Number to mask conversion macro used for GPIODir_high and GPIOio_high */
#define GPIO_NUM_MAX_BITS_MASK_HIGH     0x07
#define GPIO_NUM_TO_MASK_HIGH(X)        ( (((X) & GPIO_NUM_TOTAL_BITS_MASK) >= 32) ? (1 << ((X-32) & GPIO_NUM_MAX_BITS_MASK_HIGH)) : (0) )

/*
** Spi Controller
*/

/***** TBD. This is the BCM6368 definition.  Need BCM6816 definition. *****/
typedef struct SpiControl {
  uint16        spiMsgCtl;              /* (0x0) control byte */
#define FULL_DUPLEX_RW                  0
#define HALF_DUPLEX_W                   1
#define HALF_DUPLEX_R                   2
#define SPI_MSG_TYPE_SHIFT              14
#define SPI_BYTE_CNT_SHIFT              0
  byte          spiMsgData[0x21e];      /* (0x02 - 0x21f) msg data */
  byte          unused0[0x1e0];
  byte          spiRxDataFifo[0x220];   /* (0x400 - 0x61f) rx data */
  byte          unused1[0xe0];

  uint16        spiCmd;                 /* (0x700): SPI command */
#define SPI_CMD_NOOP                    0
#define SPI_CMD_SOFT_RESET              1
#define SPI_CMD_HARD_RESET              2
#define SPI_CMD_START_IMMEDIATE         3

#define SPI_CMD_COMMAND_SHIFT           0
#define SPI_CMD_COMMAND_MASK            0x000f

#define SPI_CMD_DEVICE_ID_SHIFT         4
#define SPI_CMD_PREPEND_BYTE_CNT_SHIFT  8
#define SPI_CMD_ONE_BYTE_SHIFT          11
#define SPI_CMD_ONE_WIRE_SHIFT          12
#define SPI_DEV_ID_0                    0
#define SPI_DEV_ID_1                    1
#define SPI_DEV_ID_2                    2
#define SPI_DEV_ID_3                    3

  byte          spiIntStatus;           /* (0x702): SPI interrupt status */
  byte          spiMaskIntStatus;       /* (0x703): SPI masked interrupt status */

  byte          spiIntMask;             /* (0x704): SPI interrupt mask */
#define SPI_INTR_CMD_DONE               0x01
#define SPI_INTR_RX_OVERFLOW            0x02
#define SPI_INTR_INTR_TX_UNDERFLOW      0x04
#define SPI_INTR_TX_OVERFLOW            0x08
#define SPI_INTR_RX_UNDERFLOW           0x10
#define SPI_INTR_CLEAR_ALL              0x1f

  byte          spiStatus;              /* (0x705): SPI status */
#define SPI_RX_EMPTY                    0x02
#define SPI_CMD_BUSY                    0x04
#define SPI_SERIAL_BUSY                 0x08

  byte          spiClkCfg;              /* (0x706): SPI clock configuration */
#define SPI_CLK_0_391MHZ                1
#define SPI_CLK_0_781MHZ                2 /* default */
#define SPI_CLK_1_563MHZ                3
#define SPI_CLK_3_125MHZ                4
#define SPI_CLK_6_250MHZ                5
#define SPI_CLK_12_50MHZ                6
#define SPI_CLK_MASK                    0x07
#define SPI_SSOFFTIME_MASK              0x38
#define SPI_SSOFFTIME_SHIFT             3
#define SPI_BYTE_SWAP                   0x80

  byte          spiFillByte;            /* (0x707): SPI fill byte */
  byte          unused2; 
  byte          spiMsgTail;             /* (0x709): msgtail */
  byte          unused3; 
  byte          spiRxTail;              /* (0x70B): rxtail */
} SpiControl;

#define SPI ((volatile SpiControl * const) SPI_BASE)

#define IUDMA_MAX_CHANNELS          32

/*
** DMA Channel Configuration (1 .. 32)
*/
typedef struct DmaChannelCfg {
  uint32        cfg;                    /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
  uint32        intStat;                /* (04) interrupts control and status */
  uint32        intMask;                /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
  uint32        maxBurst;               /* (0C) max burst length permitted */
} DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct DmaStateRam {
  uint32        baseDescPtr;            /* (00) descriptor ring start address */
  uint32        state_data;             /* (04) state/bytes done/ring offset */
  uint32        desc_len_status;        /* (08) buffer descriptor status and len */
  uint32        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} DmaStateRam;


/*
** DMA Registers
*/
typedef struct DmaRegs {
    uint32 controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32 flowctl_ch1_thresh_lo;           /* 004 */
    uint32 flowctl_ch1_thresh_hi;           /* 008 */
    uint32 flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32 flowctl_ch3_thresh_lo;           /* 010 */
    uint32 flowctl_ch3_thresh_hi;           /* 014 */
    uint32 flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32 flowctl_ch5_thresh_lo;           /* 01C */
    uint32 flowctl_ch5_thresh_hi;           /* 020 */
    uint32 flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32 flowctl_ch7_thresh_lo;           /* 028 */
    uint32 flowctl_ch7_thresh_hi;           /* 02C */
    uint32 flowctl_ch7_alloc;               /* 030 */

    uint32 ctrl_channel_reset;              /* 034 */
    uint32 ctrl_channel_debug;              /* 038 */
    uint32 reserved1;                       /* 03C */
    uint32 ctrl_global_interrupt_status;    /* 040 */
    uint32 ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8 reserved2[0x200-0x48];

    // Per channel registers/state ram
    DmaChannelCfg chcfg[IUDMA_MAX_CHANNELS];/* (200-3FF) Channel configuration */
    union {
        DmaStateRam     s[IUDMA_MAX_CHANNELS];
        uint32          u32[4 * IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-5FF) state ram */
} DmaRegs;

/*
** DMA Buffer 
*/
typedef struct DmaDesc {
  uint16        length;                 /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
  uint16        status;                 /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet

  uint32        address;                /* address of data */
} DmaDesc;


/*
** External Bus Interface
*/

/* TBD - Is it 8K to 256M or 4K to 128M? */
typedef struct EbiChipSelect {
    uint32    base;                         /* base address in upper 24 bits */
#define EBI_SIZE_8K         0
#define EBI_SIZE_16K        1
#define EBI_SIZE_32K        2
#define EBI_SIZE_64K        3
#define EBI_SIZE_128K       4
#define EBI_SIZE_256K       5
#define EBI_SIZE_512K       6
#define EBI_SIZE_1M         7
#define EBI_SIZE_2M         8
#define EBI_SIZE_4M         9
#define EBI_SIZE_8M         10
#define EBI_SIZE_16M        11
#define EBI_SIZE_32M        12
#define EBI_SIZE_64M        13
#define EBI_SIZE_128M       14
#define EBI_SIZE_256M       15
    uint32    config;
#define EBI_ENABLE          0x00000001      /* .. enable this range */
#define EBI_WAIT_STATES     0x0000000e      /* .. mask for wait states */
#define EBI_WTST_SHIFT      1               /* .. for shifting wait states */
#define EBI_WORD_WIDE       0x00000010      /* .. 16-bit peripheral, else 8 */
#define EBI_WREN            0x00000020      /* enable posted writes */
#define EBI_POLARITY        0x00000040      /* .. set to invert something, 
                                        **    don't know what yet */
#define EBI_TS_TA_MODE      0x00000080      /* .. use TS/TA mode */
#define EBI_TS_SEL          0x00000100      /* .. drive tsize, not bs_b */
#define EBI_FIFO            0x00000200      /* .. use fifo */
#define EBI_RE              0x00000400      /* .. Reverse Endian */
#define EBI_SETUP_SHIFT     16
#define EBI_HOLD_SHIFT      20
#define EBI_SETUP_STATES    0x0f0000
#define EBI_HOLD_STATES     0xf00000
} EbiChipSelect;

typedef struct MpiRegisters {
  EbiChipSelect cs[8];                          /* size chip select configuration */
  uint32        ebi_config;     /* 40 */        /* configuration */
#define EBI_MASTER_ENABLE       0x80000000      /* allow external masters */
#define EBI_EXT_MAST_PRIO       0x40000000      /* maximize ext master priority */
#define EBI_CTRL_ENABLE         0x20000000
#define EBI_TA_ENABLE           0x10000000
  uint32        reserved[4];
  uint32        pcmcia_gpio;    /*  54 */
  uint32        reserved2[1];
  uint32        pcmcia_cntrl;   /*  5c */ 
#define PCMCIA_GPIO_ENABLE      0x00002000
  uint32        reserved3[44];   
  uint32        ctrl_sp1_remap;
  uint32        reserved4[14];   
  uint32        pci_gpio;       /* 14c */
#define EN_PCI_GPIO 1
} MpiRegisters;

#define MPI ((volatile MpiRegisters * const) MPI_BASE)

/* PCI configuration address space start offset 0x40 */
#define BRCM_PCI_CONFIG_TIMER               0x40
#define BRCM_PCI_CONFIG_TIMER_RETRY_MASK	0x0000FF00
#define BRCM_PCI_CONFIG_TIMER_TRDY_MASK     0x000000FF

/***** TBD. This is the BCM6368 definition.  Need BCM6816 definition. *****/
typedef struct USBControl {
    uint32 BrtControl1;
    uint32 BrtControl2;
    uint32 BrtStatus1;
    uint32 BrtStatus2;
    uint32 UTMIControl1;
    uint32 TestPortControl;
    uint32 PllControl1;
    uint32 SwapControl;
#define EHCI_LOGICAL_ADDRESS_EN (1<<5)
#define EHCI_ENDIAN_SWAP        (1<<4)
#define EHCI_DATA_SWAP          (1<<3)
#define OHCI_LOGICAL_ADDRESS_EN (1<<2)
#define OHCI_ENDIAN_SWAP        (1<<1)
#define OHCI_DATA_SWAP          (1<<0)
    uint32 unused1;
    uint32 FrameAdjustValue;
    uint32 Setup;
#define USBH_IOC				(1<<4)
    uint32 MDIO;
    uint32 MDIO32;
    uint32 USBSimControl;
} USBControl;

#define USBH ((volatile USBControl * const) USBH_CFG_BASE)

#ifdef __cplusplus
}
#endif

#endif

