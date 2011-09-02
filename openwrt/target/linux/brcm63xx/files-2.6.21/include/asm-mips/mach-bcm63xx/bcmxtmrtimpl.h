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
/**************************************************************************
 * File Name  : bcmxtmrtimpl.h
 *
 * Description: This file contains constant definitions and structure
 *              definitions for the BCM6358/BCM6348/BCM6338 ATM/PTM
 *              network device driver.
 ***************************************************************************/

#if !defined(_BCMXTMRTIMPL_H)
#define _BCMXTMRTIMPL_H


#define MAX_DEV_CTXS                16
#define MAX_RFC2684_HDR_SIZE        10
#define ENET_MTU_SIZE               1500
#define ENET_HDR_SIZE               14
#define ENET_8021Q_SIZE             4
#define ENET_CRC_SIZE               4
#define MAX_MTU_SIZE                ((ENET_MTU_SIZE + MAX_RFC2684_HDR_SIZE + \
                                     ENET_HDR_SIZE + ENET_8021Q_SIZE + \
                                     ENET_CRC_SIZE + 63) & ~63)
#define RXBUF_HEAD_RESERVE          ((176 + 0x0f) & ~0x0f)
#define RXBUF_NEEDED_SIZE           SKB_DATA_ALIGN(RXBUF_HEAD_RESERVE + \
                                        MAX_MTU_SIZE +                 \
                                        sizeof(struct skb_shared_info))
#define MAX_BUFMEM_BLOCKS           64
#define SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define RFC1626_MTU                 9180
#define CELL_Q_BUF_SIZE             64
#define DEFAULT_CELL_Q_SIZE         10
#if defined(CONFIG_BCM96358)
#define RXBUF_ALLOC_SIZE            RXBUF_NEEDED_SIZE
#define PKT_Q_BUF_SIZE_EXP          (MAX_MTU_SIZE / 64)
#define CELL_Q_BUF_SIZE_EXP         1
#else
#define RXBUF_ALLOC_SIZE            SKB_DATA_ALIGN(2048 + RXBUF_HEAD_RESERVE + sizeof(struct skb_shared_info))
#define PKT_Q_BUF_SIZE_EXP          11
#define CELL_Q_BUF_SIZE_EXP         6
#endif

#define SAR_RX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_0
#define SAR_TX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_4
#define SAR_TIMEOUT                 (HZ/20)
#define INVALID_VCID                0xff

#define CACHE_TO_NONCACHE(x)        ((unsigned)(x)|0xA0000000)
#define NONCACHE_TO_CACHE(x)        ((unsigned)(x)&0x9FFFFFFF)
#define CACHE_TO_PHYS(x)            ((unsigned)(x)&0x1FFFFFFF)
#define NONCACHE_TO_PHYS(x)         ((unsigned)(x)&0x1FFFFFFF)
#define PHYS_TO_CACHE(x)            ((unsigned)(x)|0x80000000)
#define PHYS_TO_NONCACHE(x)         ((unsigned)(x)|0xA0000000)

#define XTMRT_UNINITIALIZED         0
#define XTMRT_INITIALIZED           1
#define XTMRT_RUNNING               2

#define SKB_PROTO_ATM_CELL          0xf000
#define XTM_POLL_DONE               0x80000000

#define REG_IRQ_STATUS_OFS          0
#define REG_IRQ_MASK_OFS            1
#define REG_FREE_PKT_Q_OFS          0
#define REG_RX_PKT_Q_OFS            1
#define REG_FREE_CELL_Q_OFS         2
#define REG_RX_CELL_Q_OFS           3
#define REG_NUM_RX_FREE_QS          4

/* Flag bits for ATM_DMA_BD ulFlags_NextRxBd */
#define BD_FLAG_EOP                 0x80000000 // End Of Packet
#define BD_FLAG_CLP                 0x40000000 // Cell Loss Priority
#define BD_FLAG_CI                  0x20000000 // Congestion Indicator
#define BD_FLAG_NEG                 0x10000000 // Negative length

/* BD bit twiddling. */
#define BD_CT_SHIFT                 27
#define BD_FLAGS_SHIFT              27
#define BD_ADDR_MASK                0x07ffffff
#define BD_SET_ADDR(F,V)            F = ((F & ~BD_ADDR_MASK) | \
                                        VIRT_TO_PHY((UINT32) (V)))
#define BD_SET_CT(F,V)              F = ((F & BD_ADDR_MASK) | \
                                        ((UINT32) (V) << BD_CT_SHIFT))
#define BD_GET_CADDR(T,F) \
    ((F) & BD_ADDR_MASK) ? T (PHYS_TO_CACHE((F) & BD_ADDR_MASK)) : NULL
#define BD_GET_NCADDR(T,F) \
    ((F) & BD_ADDR_MASK) ? T (PHYS_TO_NONCACHE((F) & BD_ADDR_MASK)) : NULL
#define BD_SET_RX_RBL(BD,V) \
    BD->ucRxAalErrors_RblHigh = (BD->ucRxAalErrors_RblHigh & 0x8e) | ((V)>>3); \
    BD->ucRxRblLow  = (BD->ucRxRblLow  & 0x1f) | (((V) & 0x07) << 5)
#define BD_GET_RX_RBL(BD) \
    (((BD->ucRxAalErrors_RblHigh & 0x01) << 3) | ((BD->ucRxRblLow & 0xe0) >> 5)

#define BD_TX_PORT_ID_MASK          0x30
#define BD_TX_PORT_ID_SHIFT         4

#if defined(CONFIG_BCM96358)
#define BD_RX_VCID_MASK             0x0f
#define BD_RX_PORT_ID_MASK          0x30
#define BD_RX_PORT_ID_SHIFT         4
#elif defined(CONFIG_BCM96348)
#define BD_RX_VCID_MASK             0x0f
#define BD_RX_PORT_ID_MASK          0x10
#define BD_RX_PORT_ID_SHIFT         4
#elif defined(CONFIG_BCM96338)
#define BD_RX_VCID_MASK             0x03
#define BD_RX_PORT_ID_MASK          0x04
#define BD_RX_PORT_ID_SHIFT         2
#endif

#define BD_RX_AAL_ERROR_MASK        (0xf0 & ~RXAAL5AAL0_LENGTH_ERROR)

#define CT_TRANSPARENT              0x01
#define CT_OAM_F5_SEGMENT           0x04
#define CT_OAM_F5_END_TO_END        0x05
#define CT_AAL5                     0x07

/* ATM Error Indicators. */
#define RXATM_PORT_NOT_ENABLED      0x80
#define RXATM_HEC_ERROR             0x40
#define RXATM_PTI_ERROR             0x20
#define RXATM_RECEIVED_IDLE_CELL    0x10
#define RXATM_INVALID_VPI_VCI       0x08
#define RXATM_NOT_USED              0x04
#define RXATM_OAM_RM_CRC_ERROR      0x02
#define RXATM_GFC_ERROR             0x01

/* AAL5 and AAL0 Error Indicators. */
#define RXAAL5AAL0_CRC_ERROR        0x80
#define RXAAL5AAL0_SHORT_PKT_ERROR  0x40
#define RXAAL5AAL0_LENGTH_ERROR     0x20
#define RXAAL5AAL0_BIG_PKT_ERROR    0x10

/* Definitions for ulIrqStatus and ulIrqMask field. */
#define INTR_TX_QUEUE                       0x00000001
#define INTR_RCQ_ALMOST_FULL                0x00000002
#define INTR_FCQ_ALMOST_EMPTY               0x00000004
#define INTR_RPQ_ALMOST_FULL                0x00000008
#define INTR_FPQ_ALMOST_EMPTY               0x00000010
#define INTR_MIB_COUNTER_HALF_FULL          0x00000020
#define INTR_RCQ_WD_TIMER                   0x00000040
#define INTR_RPQ_WD_TIMER                   0x00000080
#define INTR_RCQ_IMMED_RSP                  0x00000100
#define INTR_RPQ_IMMED_RSP                  0x00000200
#define INTR_ERROR_RX_RTR_DROPPED_CELL      0x00000400
#define INTR_ERROR_VCAM_MULT_MATCH          0x00000800
#define INTR_TX_QUEUE_ENABLE_ALL            0x00ff0000

#define INTR_MASK (INTR_RCQ_ALMOST_FULL | INTR_RPQ_ALMOST_FULL | \
    INTR_RCQ_WD_TIMER | INTR_RPQ_WD_TIMER | INTR_RCQ_IMMED_RSP | \
    INTR_RPQ_IMMED_RSP | INTR_ERROR_VCAM_MULT_MATCH | INTR_TX_QUEUE_ENABLE_ALL)


/* ATM DMA Buffer Descriptor */
typedef struct AtmDmaBd
{
    UINT32 ulCt_BufPtr;
    UINT32 ulFlags_NextRxBd;
    UINT8 ucUui8;
    UINT8 ucRxPortId_Vcid;
    UINT16 usLength;
    union
    {
        UINT8 ucTxPortId_Gfc;
        UINT8 ucRxAtmErrors;
    } u1;
    UINT8 ucRxAalErrors_RblHigh;
    union
    {
        UINT8 ucRxRblLow;
        UINT8 ucFreeRbl;
    } u2;
    UINT8 ucReserved;
} ATM_DMA_BD, *PATM_DMA_BD;

/* Information about a DMA transmit channel. A device instance may use more
 * than one transmit DMA channel. A DMA channel corresponds to a transmit queue.
 */
typedef struct TxQInfo
{
    UINT32 ulPort;
    UINT32 ulPtmPriority;
    UINT32 ulSubPriority;
    UINT32 ulQueueSize;
    UINT32 ulDmaIndex;
    UINT32 ulUseCount;

    UINT8 *pMemBuf;
    volatile PATM_DMA_BD pQBase;
    volatile UINT32 *pulQHtl;
    volatile UINT8 *pucHeadIdx;
    volatile UINT8 *pucTailIdx;
    UINT32 ulHeadIdx;
    UINT32 ulLength;
    UINT32 ulCurrBdIdx;
    UINT8 ucNumBds;
    struct sk_buff **ppSkbs;
    UINT32 ulNumTxBufsQdOne;
    UINT32 ulFreeBds;
    UINT32 ulHead;
} TXQINFO, *PTXQINFO;

/* The definition of the driver control structure */
typedef struct bcmxtmrt_dev_context
{
    /* Linux structures. */
    struct net_device *pDev;        
    struct net_device_stats DevStats;
    IOCTL_MIB_INFO MibInfo;
    struct ppp_channel Chan;
    struct sk_buff *pRxSoftSarSkb;
    UINT32 ulRxSoftSarCrc;

    /* ATM/PTM fields. */
    XTM_ADDR Addr;
    UINT32 ulLinkState;
    UINT32 ulOpenState;
    UINT32 ulAdminStatus;
    UINT32 ulHdrType;
    UINT32 ulEncapType; /* IPoA, PPPoA, or EoA[bridge,MER,PPPoE] */
    UINT32 ulFlags;

    /* Transmit fields. */
    UINT8 ucTxVcid;
    UINT32 ulTxQueuesSize;
    PTXQINFO pTxQueues[MAX_TRANSMIT_QUEUES];
    PTXQINFO pTxPriorities[MAX_PHY_PORTS][MAX_SUB_PRIORITIES];

    /*Port Mirroring fields*/
    char szMirrorIntfIn[MIRROR_INTF_SIZE];
    char szMirrorIntfOut[MIRROR_INTF_SIZE];
} BCMXTMRT_DEV_CONTEXT, *PBCMXTMRT_DEV_CONTEXT;

typedef struct RxFreeQueues
{
    UINT32 ulQAddr;
    UINT32 ulQHead;
    UINT32 ulQTail;
    UINT32 ulQLen;
} RXFREEQUEUES, *PRXFREEQUEUES;

/* Information that is global to all network device instances. */
typedef struct bcmxtmrt_global_info
{
    /* Linux structures. */
    PBCMXTMRT_DEV_CONTEXT pDevCtxs[MAX_DEV_CTXS];
    PBCMXTMRT_DEV_CONTEXT pDevCtxsByVcid[MAX_DEV_CTXS];
    struct tasklet_struct Tasklet;
    struct timer_list Timer;
    struct atm_dev *pAtmDev;

    /* DMA, BD and buffer fields. */
    struct sk_buff *pFreeRxSkbList;
    UINT8 *pRxBdMem;
    UINT8 *pBufMem[MAX_BUFMEM_BLOCKS];
    UINT8 *pRxCellMem;
    TXQINFO TxQInfos[MAX_TRANSMIT_QUEUES];
    volatile UINT32 *pulIrqStatus;
    volatile UINT32 *pulIrqMask;
    volatile PRXFREEQUEUES pRxFreeQs;
    volatile UINT32 *pulTxQAddrBase;
    volatile UINT32 *pulTxQHtlBase;
    UINT32 *pulRxFreeQAddrs[REG_NUM_RX_FREE_QS];

    /* Global transmit queue fields. */
    UINT32 ulNumExtBufs;
    UINT32 ulNumExtBufsRsrvd;
    UINT32 ulNumExtBufs90Pct;
    UINT32 ulNumExtBufs50Pct;
    UINT32 ulNumTxQs;
    UINT32 ulNumTxBufsQdAll;
    UINT32 ulDbgQ1;
    UINT32 ulDbgQ2;
    UINT32 ulDbgQ3;
    UINT32 ulDbgD1;
    UINT32 ulDbgD2;
    UINT32 ulDbgD3;

    /* Callback functions. */
    XTMRT_CALLBACK pfnOamHandler;
    void *pOamContext;
    XTMRT_CALLBACK pfnAsmHandler;
    void *pAsmContext;

    /* Everything else. */
    UINT32 ulDrvState;
} BCMXTMRT_GLOBAL_INFO, *PBCMXTMRT_GLOBAL_INFO;

#if defined(INCLUDE_ATM_CRC32)
#define Aal5UpdateCrc(crc,b)    ((crc)<<8) ^ g_AtmCrc32Table[((crc)>>24) ^ (b)]
static UINT32 g_AtmCrc32Table[256] =
{
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005, 
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, 
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
    0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75, 
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
    0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD, 
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D, 
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
    0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95, 
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
    0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
    0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072, 
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
    0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02, 
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
    0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA, 
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692, 
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
    0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A, 
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
    0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A, 
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
    0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB, 
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
    0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53, 
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
    0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B, 
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
    0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B, 
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
    0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3, 
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
    0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
    0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3, 
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
    0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24, 
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC, 
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
    0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
    0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C, 
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
    0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C, 
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
    0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};
#endif /* defined(INCLUDE_ATM_CRC32) */


#endif /* _BCMXTMRTIMPL_H */

