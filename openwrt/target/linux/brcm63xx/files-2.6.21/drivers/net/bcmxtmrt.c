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
 * File Name  : bcmxtmrt.c
 *
 * Description: This file implements BCM6368 ATM/PTM network device driver
 *              runtime processing - sending and receiving data.
 ***************************************************************************/


/* Defines. */
#define CARDNAME    "bcmxtmrt"
#define VERSION     "0.1"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__
#define INCLUDE_ATM_CRC32

/* Includes. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmnet.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include "bcmxtmrtimpl.h"
#include <asm/io.h>
#include <asm/r4kcache.h>
#include <asm/uaccess.h>
#include <linux/blog.h>     /* CONFIG_BLOG */


/* Externs. */
extern unsigned long getMemorySize(void);


/* Prototypes. */
static inline void cache_wbflush_region(void *addr, void *end);
static inline void cache_wbflush_len(void *addr, int len);
int __init bcmxtmrt_init( void );
static void bcmxtmrt_cleanup( void );
static int bcmxtmrt_open( struct net_device *dev );
static int bcmxtmrt_close( struct net_device *dev );
static void bcmxtmrt_timeout( struct net_device *dev );
static struct net_device_stats *bcmxtmrt_query(struct net_device *dev);
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd);
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx,void *useraddr);
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd,
    unsigned long arg);
static PBCMXTMRT_DEV_CONTEXT FindDevCtx( UINT16 usVpi, UINT16 usVci );
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc);
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc);
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb);
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan,struct sk_buff *skb);
static int bcmxtmrt_xmit( struct sk_buff *skb, struct net_device *dev);
static void AddRfc2684Hdr(struct sk_buff **ppskb,PBCMXTMRT_DEV_CONTEXT pDevCtx);
static void QueueAdd( UINT32 ulOfs, PATM_DMA_BD pBd );
static PATM_DMA_BD QueueRemove( UINT32 ulOfs );
static void bcmxtmrt_freeskbordata( PATM_DMA_BD pBd, struct sk_buff *skb,
    unsigned nFlag );
static irqreturn_t bcmxtmrt_isr(int nIrq, PBCMXTMRT_GLOBAL_INFO pGi);
static int bcmxtmrt_poll(struct net_device * dev, int * budget);
static UINT32 bcmxtmrt_rxtask( UINT32 ulBudget, UINT32 ulFreeQOfs,
    UINT32 ulRxQOfs, UINT32 ulQBufSizeExp );
static struct sk_buff *ProcessReceiveCellQueue( volatile PATM_DMA_BD pBd );
static struct sk_buff *ReassembleCell( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    UINT8 *pucCell );
static void MirrorPacket( struct sk_buff *skb, char *intfName );
static void bcmxtmrt_timer( PBCMXTMRT_GLOBAL_INFO pGi );
static int DoGlobInitReq( PXTMRT_GLOBAL_INIT_PARMS pGip );
static int DoCreateDeviceReq( PXTMRT_CREATE_NETWORK_DEVICE pCnd );
static int DoRegCellHdlrReq( PXTMRT_CELL_HDLR pCh );
static int DoUnregCellHdlrReq( PXTMRT_CELL_HDLR pCh );
static int DoLinkStsChangedReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoLinkUp( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoLinkDown( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoSetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static int DoUnsetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static int DoSendCellReq( PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC );
static int DoDeleteDeviceReq( PBCMXTMRT_DEV_CONTEXT pDevCtx );
static int bcmxtmrt_add_proc_files( void );
static int bcmxtmrt_del_proc_files( void );
static int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt, 
    int *eof, void *data);


/* Globals. */
static BCMXTMRT_GLOBAL_INFO g_GlobalInfo;
static struct atm_ioctl g_PppoAtmIoctlOps =
    {
        .ioctl    = bcmxtmrt_atm_ioctl,
    };
static struct ppp_channel_ops g_PppoAtmOps =
    {
        .start_xmit = bcmxtmrt_pppoatm_send
    };
static const struct atmdev_ops g_AtmDevOps =
    {
        .open       = bcmxtmrt_atmdev_open,
        .close      = bcmxtmrt_atmdev_close,
        .send       = bcmxtmrt_atmdev_send,
    };

/*
 *  Data integrity and cache management.
 *          - Prior to xmit, the modified portion of a data buffer that is
 *            to be transmitted needs to be wback flushed so that the data
 *            in the buffer is coherent with that in the L2 Cache.
 *            The region is demarcated by
 *                skb->data and skb->len
 *          - When a buffer is recycled, the buffer region that is handed to
 *            the DMA controller needs to be invalidated in L2 Cache.
 *            The region is demarcated by
 *                skb->head + RESERVED HEADROOM and skb->end + skb_shared_info
 *          - Given that the entire skb_shared_info is not accessed,
 *            e.g. the skb_frag_t array is only accessed if nr_frags is > 0,
 *            it is sufficient to only flush/invalidate a portion of the
 *            skb_shared_info that is placed after the skb->end.
 *
 *  Cache operations reworked so as to not perform any operation beyond
 *  the "end" demarcation.
 */

/*
 * Macros to round down and up, an address to a cachealigned address
 */
#define ROUNDDN(addr, align)  ( (addr) & ~((align) - 1) )
#define ROUNDUP(addr, align)  ( ((addr) + (align) - 1) & ~((align) - 1) )


/***************************************************************************
 * Function Name: cache_wbflush_region
 * Description  : Do MIPS flush cache operation on a buffer.
 *                if (addr == end) && (addr is cache aligned) no operation.
 *                if (addr == end) && (addr is not cache aligned) flush.
 *
 *                addr is rounded down to cache line.
 *                end is rounded up to cache line.
 *
 *                All cache lines from addr, NOT INCLUDING end are flushed. 
 * Returns      : None.
 ***************************************************************************/
static inline void cache_wbflush_region(void *addr, void *end)
{
    unsigned long dc_lsize = current_cpu_data.dcache.linesz;
    unsigned long a = ROUNDDN( (unsigned long)addr, dc_lsize );
    unsigned long e = ROUNDUP( (unsigned long)end, dc_lsize );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += dc_lsize;
    }
}


/***************************************************************************
 * Function Name: cache_wbflush_len
 * Description  : Do MIPS invalidate cache operation on a buffer.
 *                if (len == 0) && (addr is cache aligned) then no operation.
 *                if (len == 0) && (addr is not cache aligned) then flush !
 *                end = addr + len, then rounded up to cacheline
 *                addr is rounded down to cache line.
 *                All cache lines from addr, NOT INCLUDING end are flushed.
 * Returns      : None.
 ***************************************************************************/
static inline void cache_wbflush_len(void *addr, int len)
{
    unsigned long dc_lsize = current_cpu_data.dcache.linesz;
    unsigned long a = ROUNDDN( (unsigned long)addr, dc_lsize );
    unsigned long e = ROUNDUP( ((unsigned long)addr + len), dc_lsize );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += dc_lsize;
    }
}


/***************************************************************************
 * Function Name: bcmxtmrt_init
 * Description  : Called when the driver is loaded.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int __init bcmxtmrt_init( void )
{
    UINT16 usChipId  = (PERF->RevID & 0xFFFF0000) >> 16;
    UINT16 usChipRev = (PERF->RevID & 0xFF);

    printk(CARDNAME ": Broadcom BCM%X%X ATM Network Device ", usChipId,
        usChipRev);
    printk(VER_STR "\n");

    memset(&g_GlobalInfo, 0x00, sizeof(g_GlobalInfo));

    register_atm_ioctl(&g_PppoAtmIoctlOps);
    g_GlobalInfo.pAtmDev = atm_dev_register("bcmxtmrt_atmdev", &g_AtmDevOps,
        -1, NULL);
    if( g_GlobalInfo.pAtmDev )
    {
        g_GlobalInfo.pAtmDev->ci_range.vpi_bits = 12;
        g_GlobalInfo.pAtmDev->ci_range.vci_bits = 16;
    }

    g_GlobalInfo.ulNumExtBufs = NR_RX_BDS(getMemorySize());
    g_GlobalInfo.ulNumExtBufsRsrvd = g_GlobalInfo.ulNumExtBufs / 5;
    g_GlobalInfo.ulNumExtBufs90Pct = (g_GlobalInfo.ulNumExtBufs * 9) / 10;
    g_GlobalInfo.ulNumExtBufs50Pct = g_GlobalInfo.ulNumExtBufs / 2;

    bcmxtmrt_add_proc_files();

    return( 0 );
} /* bcmxtmrt_init */


/***************************************************************************
 * Function Name: bcmxtmrt_cleanup
 * Description  : Called when the driver is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_cleanup( void )
{
    bcmxtmrt_add_proc_files();
    deregister_atm_ioctl(&g_PppoAtmIoctlOps);
    if( g_GlobalInfo.pAtmDev )
    {
        atm_dev_deregister( g_GlobalInfo.pAtmDev );
        g_GlobalInfo.pAtmDev = NULL;
    }
} /* bcmxtmrt_cleanup */


/***************************************************************************
 * Function Name: bcmxtmrt_open
 * Description  : Called to make the device operational.  Called due to shell
 *                command, "ifconfig <device_name> up".
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_open( struct net_device *dev )
{
    int nRet = 0;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;

    netif_start_queue(dev);

    if( pDevCtx->ulAdminStatus == ADMSTS_UP )
        pDevCtx->ulOpenState = XTMRT_DEV_OPENED;
    else
        nRet = -EIO;

    return( nRet );
} /* bcmxtmrt_open */


/***************************************************************************
 * Function Name: bcmxtmrt_close
 * Description  : Called to stop the device.  Called due to shell command,
 *                "ifconfig <device_name> down".
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_close( struct net_device *dev )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;

    pDevCtx->ulOpenState = XTMRT_DEV_CLOSED;
    netif_stop_queue(dev);
    return 0;
} /* bcmxtmrt_close */


/***************************************************************************
 * Function Name: bcmxtmrt_timeout
 * Description  : Called when there is a transmit timeout. 
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_timeout( struct net_device *dev )
{
    dev->trans_start = jiffies;
    netif_wake_queue(dev);
} /* bcmxtmrt_timeout */


/***************************************************************************
 * Function Name: bcmxtmrt_query
 * Description  : Called to return device statistics. 
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static struct net_device_stats *bcmxtmrt_query(struct net_device *dev)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
    return( &pDevCtx->DevStats );
} /* bcmxtmrt_query */


/***************************************************************************
 * Function Name: bcmxtmrt_ioctl
 * Description  : Driver IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
    int *data=(int*)Req->ifr_data;
    int status;
    MirrorCfg mirrorCfg;
    int nRet = 0;

    switch (nCmd)
    {
    case SIOCGLINKSTATE:
        if( pDevCtx->ulLinkState == LINK_UP )
            status = LINKSTATE_UP;
        else
            status = LINKSTATE_DOWN;
        if (copy_to_user((void*)data, (void*)&status, sizeof(int)))
            nRet = -EFAULT;
        break;

    case SIOCSCLEARMIBCNTR:
        memset(&pDevCtx->DevStats, 0, sizeof(struct net_device_stats));
        break;

    case SIOCMIBINFO:
        if (copy_to_user((void*)data, (void*)&pDevCtx->MibInfo,
            sizeof(pDevCtx->MibInfo)))
        {
            nRet = -EFAULT;
        }
        break;

    case SIOCPORTMIRROR:
        if(copy_from_user((void*)&mirrorCfg,data,sizeof(MirrorCfg)))
            nRet=-EFAULT;
        else
        {
            if( mirrorCfg.nDirection == MIRROR_DIR_IN )
            {
                if( mirrorCfg.nStatus == MIRROR_ENABLED )
                    strcpy(pDevCtx->szMirrorIntfIn, mirrorCfg.szMirrorInterface);
                else
                    memset(pDevCtx->szMirrorIntfIn, 0x00, MIRROR_INTF_SIZE);
            }
            else /* MIRROR_DIR_OUT */
            {
                if( mirrorCfg.nStatus == MIRROR_ENABLED )
                    strcpy(pDevCtx->szMirrorIntfOut, mirrorCfg.szMirrorInterface);
                else
                    memset(pDevCtx->szMirrorIntfOut, 0x00, MIRROR_INTF_SIZE);
            }
        }
        break;

    case SIOCETHTOOL:
        nRet = bcmxtmrt_ethtool_ioctl(pDevCtx, (void *) Req->ifr_data);
        break;

    default:
        nRet = -EOPNOTSUPP;    
        break;
    }

    return( nRet );
} /* bcmxtmrt_ioctl */


/***************************************************************************
 * Function Name: bcmxtmrt_ethtool_ioctl
 * Description  : Driver ethtool IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *useraddr)
{
    struct ethtool_drvinfo info;
    struct ethtool_cmd ecmd;
    unsigned long ethcmd;
    int nRet = 0;

    if( copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)) == 0 )
    {
        switch (ethcmd)
        {
        case ETHTOOL_GDRVINFO:
            info.cmd = ETHTOOL_GDRVINFO;
            strncpy(info.driver, CARDNAME, sizeof(info.driver)-1);
            strncpy(info.version, VERSION, sizeof(info.version)-1);
            if (copy_to_user(useraddr, &info, sizeof(info)))
                nRet = -EFAULT;
            break;

        case ETHTOOL_GSET:
            ecmd.cmd = ETHTOOL_GSET;
            ecmd.speed = pDevCtx->MibInfo.ulIfSpeed / (1024 * 1024);
            if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
                nRet = -EFAULT;
            break;

        default:
            nRet = -EOPNOTSUPP;    
            break;
        }
    }
    else
       nRet = -EFAULT;

    return( nRet );
}

/***************************************************************************
 * Function Name: bcmxtmrt_atm_ioctl
 * Description  : Driver ethtool IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd,
    unsigned long arg)
{
    struct atm_vcc *pAtmVcc = ATM_SD(sock);
    void __user *argp = (void __user *)arg;
    atm_backend_t b;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    int nRet = -ENOIOCTLCMD;

    switch( cmd )
    {
    case ATM_SETBACKEND:
        if( get_user(b, (atm_backend_t __user *) argp) == 0 )
        {
            switch (b)
            {
            case ATM_BACKEND_PPP_BCM:
                if( (pDevCtx = FindDevCtx((UINT16) pAtmVcc->vpi, (UINT16)
                    pAtmVcc->vci)) != NULL && pDevCtx->Chan.private == NULL )
                {
                    pDevCtx->Chan.private = pDevCtx->pDev;
                    pDevCtx->Chan.ops = &g_PppoAtmOps;
                    pDevCtx->Chan.mtu = 1500; /* TBD. Calc value. */
                    pAtmVcc->user_back = pDevCtx;
                    if( ppp_register_channel(&pDevCtx->Chan) == 0 )
                        nRet = 0;
                    else
                        nRet = -EFAULT;
                }
                else
                    nRet = (pDevCtx) ? 0 : -EFAULT;
                break;

            case ATM_BACKEND_PPP_BCM_DISCONN:
                /* This is a patch for PPP reconnection.
                 * ppp daemon wants us to send out an LCP termination request
                 * to let the BRAS ppp server terminate the old ppp connection.
                 */
                if( (pDevCtx = FindDevCtx((UINT16) pAtmVcc->vpi, (UINT16)
                    pAtmVcc->vci)) != NULL )
                {
                    struct sk_buff *skb;
                    int size = 6;
                    int eff  = (size+3) & ~3; /* align to word boundary */

                    while (!(skb = alloc_skb(eff, GFP_KERNEL)))
                        schedule();

                    skb->dev = NULL; /* for paths shared with net_device interfaces */
                    skb_put(skb, size);

                    skb->data[0] = 0xc0;  /* PPP_LCP == 0xc021 */
                    skb->data[1] = 0x21;
                    skb->data[2] = 0x05;  /* TERMREQ == 5 */
                    skb->data[3] = 0x02;  /* id == 2 */
                    skb->data[4] = 0x00;  /* HEADERLEN == 4 */
                    skb->data[5] = 0x04;

                    if (eff > size)
                        memset(skb->data+size,0,eff-size);

                    nRet = bcmxtmrt_xmit( skb, pDevCtx->pDev );
                }
                else
                    nRet = -EFAULT;
                break;

            case ATM_BACKEND_PPP_BCM_CLOSE_DEV:
                if( (pDevCtx = FindDevCtx((UINT16) pAtmVcc->vpi, (UINT16)
                    pAtmVcc->vci)) != NULL)
                {
                    bcmxtmrt_pppoatm_send(&pDevCtx->Chan, NULL);
                    ppp_unregister_channel(&pDevCtx->Chan);
                    pDevCtx->Chan.private = NULL;
                }
                nRet = 0;
                break;

            default:
                break;
            }
        }
        else
            nRet = -EFAULT;
        break;

    case PPPIOCGCHAN:
        if( (pDevCtx = FindDevCtx((UINT16) pAtmVcc->vpi, (UINT16)
            pAtmVcc->vci)) != NULL )
        {
            nRet = put_user(ppp_channel_index(&pDevCtx->Chan),
                (int __user *) argp) ? -EFAULT : 0;
        }
        else
            nRet = -EFAULT;
        break;

    case PPPIOCGUNIT:
        if( (pDevCtx = FindDevCtx((UINT16) pAtmVcc->vpi, (UINT16)
            pAtmVcc->vci)) != NULL )
        {
            nRet = put_user(ppp_unit_number(&pDevCtx->Chan),
                (int __user *) argp) ? -EFAULT : 0;
        }
        else
            nRet = -EFAULT;
        break;
    }

    return( nRet );
} /* bcmxtmrt_atm_ioctl */


/***************************************************************************
 * Function Name: FindDevCtx
 * Description  : Finds a device context structure for a VCC.
 * Returns      : Pointer to a device context structure or NULL.
 ***************************************************************************/
static PBCMXTMRT_DEV_CONTEXT FindDevCtx( UINT16 usVpi, UINT16 usVci )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( (pDevCtx = pGi->pDevCtxs[i]) != NULL )
        {
            if( pDevCtx->Addr.u.Vcc.usVpi == usVpi &&
                pDevCtx->Addr.u.Vcc.usVci == usVci )
            {
                break;
            }

            pDevCtx = NULL;
        }
    }

    return( pDevCtx );
} /* FindDevCtx */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_open
 * Description  : ATM device open
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc)
{
    set_bit(ATM_VF_READY,&pVcc->flags);
    return( 0 );
} /* bcmxtmrt_atmdev_open */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_close
 * Description  : ATM device open
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc)
{
    clear_bit(ATM_VF_READY,&pVcc->flags);
    clear_bit(ATM_VF_ADDR,&pVcc->flags);
} /* bcmxtmrt_atmdev_close */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_send
 * Description  : send data
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx =
        FindDevCtx((UINT16) pVcc->vpi, (UINT16) pVcc->vci);
    int nRet;

    if( pDevCtx )
        nRet = bcmxtmrt_xmit( skb, pDevCtx->pDev );
    else
        nRet = -EIO;

    return( nRet );
} /* bcmxtmrt_atmdev_send */


/***************************************************************************
 * Function Name: bcmxtmrt_pppoatm_send
 * Description  : Called by the PPP driver to send data.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan, struct sk_buff *skb)
{
    bcmxtmrt_xmit( skb, (struct net_device *) pChan->private);
    return(1);
} /* bcmxtmrt_pppoatm_send */


/***************************************************************************
 * Function Name: QueuePacket
 * Description  : Determines whether to queue a packet for transmission based
 *                on the number of total external (ie Ethernet) buffers and
 *                buffers already queued.
 * Returns      : 1 to queue packet, 0 to drop packet
 ***************************************************************************/
inline int QueuePacket( PBCMXTMRT_GLOBAL_INFO pGi, PTXQINFO pTqi )
{
    int nRet = 0; /* default to drop packet */

    if( pGi->ulNumTxQs == 1 )
    {
        /* One total transmit queue.  Allow up to 90% of external buffers to
         * be queued on this transmit queue.
         */
        if( pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs90Pct )
        {
            nRet = 1; /* queue packet */
            pGi->ulDbgQ1++;
        }
        else
            pGi->ulDbgD1++;
    }
    else
    {
        if(pGi->ulNumExtBufs - pGi->ulNumTxBufsQdAll > pGi->ulNumExtBufsRsrvd)
        {
            /* The available number of external buffers is greater than the
             * reserved value.  Allow up to 50% of external buffers to be
             * queued on this transmit queue.
             */
            if( pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs50Pct )
            {
                nRet = 1; /* queue packet */
                pGi->ulDbgQ2++;
            }
            else
                pGi->ulDbgD2++;
        }
        else
        {
            /* Divide the reserved number of external buffers evenly among all
             * of the transmit queues.
             */
            if(pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufsRsrvd / pGi->ulNumTxQs)
            {
                nRet = 1; /* queue packet */
                pGi->ulDbgQ3++;
            }
            else
                pGi->ulDbgD3++;
        }
    }

    return( nRet );
} /* QueuePacket */


/***************************************************************************
 * Function Name: bcmxtmrt_xmit
 * Description  : Check for transmitted packets to free and, if skb is
 *                non-NULL, transmit a packet.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_xmit( struct sk_buff *skb, struct net_device *dev)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
    PTXQINFO pTqi;
    UINT32 i;

    local_bh_disable();

    /* Free packets that have been transmitted. */
    for( i = 0; i < pDevCtx->ulTxQueuesSize; i++ )
    {
        pTqi = pDevCtx->pTxQueues[i];
        while( pTqi->ppSkbs[pTqi->ulCurrBdIdx] != NULL &&
            pTqi->ulCurrBdIdx != *pTqi->pucTailIdx )
        {
            local_bh_enable();
            dev_kfree_skb_any(pTqi->ppSkbs[pTqi->ulCurrBdIdx]);
            local_bh_disable();
            pTqi->ppSkbs[pTqi->ulCurrBdIdx] = NULL;

            if( ++pTqi->ulCurrBdIdx >= pTqi->ulQueueSize )
                pTqi->ulCurrBdIdx = 0;

            pTqi->ulNumTxBufsQdOne--;
            pGi->ulNumTxBufsQdAll--;
        }
    }

    pTqi = NULL;
    
    if( skb && pDevCtx->ulTxQueuesSize && pDevCtx->ulLinkState == LINK_UP )
    {
        /* Find a transmit queue to send on. */
        UINT32 ulPort = 0;

#ifdef CONFIG_NETFILTER
        /* bit 2-0 of the 32-bit nfmark is the subpriority (0 to 7) set by ebtables.
         * bit 3   of the 32-bit nfmark is the DSL latency, 0=PATH0, 1=PATH1
         * bit 4   of the 32-bit nfmark is the PTM priority, 0=LOW,  1=HIGH
         */
        UINT32 ulPriority = skb->mark & 0x07;

        ulPort = (skb->mark >> 3) & 0x1;  //DSL latency

        pTqi = pDevCtx->pTxPriorities[ulPort][ulPriority];
        
        /* If a transmit queue was not found, use the existing highest priority queue
         * that had been configured with the default DSL latency (port).
         */
        if (pTqi == NULL && ulPriority > 1)
        {
           UINT32 ulPortDflt;

           if (pDevCtx->pTxPriorities[0][0] == pDevCtx->pTxQueues[0])
              ulPortDflt = 0;
           else
              ulPortDflt = 1;

           for (i = ulPriority - 1; pTqi == NULL && i >= 1; i--)
              pTqi = pDevCtx->pTxPriorities[ulPortDflt][i];
        }
#endif

        /* If a transmit queue was not found, use the first one. */
        if( pTqi == NULL )
            pTqi = pDevCtx->pTxQueues[0];

        if( ((pTqi->ulHeadIdx + 1) % pTqi->ulQueueSize) != pTqi->ulCurrBdIdx &&
            QueuePacket(pGi, pTqi) )
        {
            volatile PATM_DMA_BD pBd;

            pTqi->ulNumTxBufsQdOne++;
            pGi->ulNumTxBufsQdAll++;

            if( pDevCtx->szMirrorIntfOut[0] != '\0' &&
                (pDevCtx->ulHdrType ==  HT_PTM ||
                 pDevCtx->ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
                 pDevCtx->ulHdrType ==  HT_VC_MUX_ETHERNET) )
            {
                MirrorPacket( skb, pDevCtx->szMirrorIntfOut );
            }

            /* Configure FlowCache stack and configure CMF. */
            blog_emit( skb, dev, (unsigned int)pTqi->ulDmaIndex,
                   0, pDevCtx->ulEncapType );

            switch( pDevCtx->ulHdrType )
            {
            case HT_LLC_SNAP_ROUTE_IP:
            case HT_VC_MUX_IPOA:
                /* Packet is passed to driver with MAC header for IPoA */
                skb_pull(skb, ETH_HLEN);
                break;

            case HT_LLC_SNAP_ETHERNET:
            case HT_VC_MUX_ETHERNET:
                if( skb->len < ETH_ZLEN &&
                    skb->protocol != SKB_PROTO_ATM_CELL )
                {
                    struct sk_buff *skb2 = skb_copy_expand(skb, 0, ETH_ZLEN -
                        skb->len, GFP_ATOMIC);
                    if( skb2 )
                    {
                        dev_kfree_skb(skb);
                        skb = skb2;
                        memset(skb->tail, 0, ETH_ZLEN - skb->len);
                        skb_put(skb, ETH_ZLEN - skb->len);
                    }
                }
                break;

            default:
                break;
            }

            if( HT_LEN(pDevCtx->ulHdrType) != 0 &&
                skb->protocol != SKB_PROTO_ATM_CELL )
            {
                AddRfc2684Hdr( &skb, pDevCtx );
            }

            /* Track sent SKB, so we can release them later */
            pTqi->ppSkbs[pTqi->ulHeadIdx] = skb;

            pBd = &pTqi->pQBase[pTqi->ulHeadIdx];

            if( skb->protocol == SKB_PROTO_ATM_CELL )
                pBd->ulCt_BufPtr = CT_TRANSPARENT << BD_CT_SHIFT;
            else
                pBd->ulCt_BufPtr = CT_AAL5 << BD_CT_SHIFT;

            BD_SET_ADDR(pBd->ulCt_BufPtr, skb->data);
            pBd->usLength = (UINT16) skb->len;
            pBd->ucRxPortId_Vcid = pDevCtx->ucTxVcid;
            pBd->ucUui8 = 0;
            pBd->u1.ucTxPortId_Gfc = pTqi->ulPort << BD_TX_PORT_ID_SHIFT;
            pBd->ulFlags_NextRxBd |= BD_FLAG_EOP;
            pBd->ucReserved = (UINT8) ulPriority - 1;

            if( ++pTqi->ulHeadIdx == pTqi->ulQueueSize )
                pTqi->ulHeadIdx = 0;

            flush_dcache_line((UINT32) pBd);

            cache_wbflush_len(skb->data, skb->len);

            /* Updating the DMA transmit queue head index causes the DMA
             * transfer to start.
             */
            *pTqi->pucHeadIdx = (UINT8) pTqi->ulHeadIdx;

            /* Transmitted bytes are counted in hardware. */
            pDevCtx->DevStats.tx_packets++;
            pDevCtx->DevStats.tx_bytes += skb->len;
            pDevCtx->pDev->trans_start = jiffies;

        }
        else
        {
            /* Transmit queue is full.  Free the socket buffer.  Don't call
             * netif_stop_queue because this device may use more than one queue.
             */
            local_bh_enable();
            dev_kfree_skb_any(skb);
            pDevCtx->DevStats.tx_errors++;
            local_bh_disable();
        }
    }
    else
    {
        if( skb )
        {
            local_bh_enable();
            dev_kfree_skb_any(skb);
            pDevCtx->DevStats.tx_dropped++;
            local_bh_disable();
        }
    }

    local_bh_enable();

    return 0;
} /* bcmxtmrt_xmit */


/***************************************************************************
 * Function Name: AddRfc2684Hdr
 * Description  : Adds the RFC2684 header to an ATM packet before transmitting
 *                it.
 * Returns      : None.
 ***************************************************************************/
static void AddRfc2684Hdr(struct sk_buff **ppskb, PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
    const UINT32 ulMinPktSize = 60;
    UINT8 ucHdrs[][16] =
        {{},
         {0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00},
         {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00},
         {0xFE, 0xFE, 0x03, 0xCF},
         {0x00, 0x00}};
    int minheadroom = HT_LEN(pDevCtx->ulHdrType);
    struct sk_buff *skb = *ppskb;
    int headroom = skb_headroom(skb);

    if (headroom < minheadroom)
    {
        struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);

        dev_kfree_skb(skb);
        if (skb2 == NULL)
            skb = NULL;
        else
            skb = skb2;
    }
    if( skb )
    {
        int skblen;

        skb_push(skb, minheadroom);
        memcpy(skb->data, ucHdrs[HT_TYPE(pDevCtx->ulHdrType)], minheadroom);
        skblen = skb->len;
        if (skblen < ulMinPktSize)
        {
            struct sk_buff *skb2=skb_copy_expand(skb, 0, ulMinPktSize -
                skb->len, GFP_ATOMIC);

            dev_kfree_skb(skb);
            if (skb2 == NULL)
                skb = NULL;
            else
            {
                skb = skb2;
                memset(skb->tail, 0, ulMinPktSize - skb->len);
                skb_put(skb, ulMinPktSize - skb->len);
            }
        }
    }

    *ppskb = skb;;
} /* AddRfc2684Hdr */


/***************************************************************************
 * Function Name: QueueAdd
 * Description  : Adds an element to a ATM Free or Receive queue.
 * Returns      : None.
 ***************************************************************************/
static void QueueAdd( UINT32 ulOfs, PATM_DMA_BD pBd )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PRXFREEQUEUES pQueue = &pGi->pRxFreeQs[ulOfs];
    UINT32 *pulQBase = pGi->pulRxFreeQAddrs[ulOfs];
    UINT32 ulHeadIdx = pQueue->ulQHead;
    UINT32 ulNextHeadIdx = (ulHeadIdx + 1) % (pQueue->ulQLen + 1);
    UINT32 ulFlags;

    local_save_flags(ulFlags);
    local_irq_disable();

    if( ulNextHeadIdx != pQueue->ulQTail )
    {
        flush_dcache_line((UINT32) pBd);
        pulQBase[ulHeadIdx] = (UINT32) VIRT_TO_PHY(pBd);
        pQueue->ulQHead = ulNextHeadIdx;
    }

    local_irq_restore(ulFlags);
} /* QueueAdd */


/***************************************************************************
 * Function Name: QueueRemove
 * Description  : Removes an element to a ATM Free or Receive queue.
 * Returns      : Pointer to BD or NULL
 ***************************************************************************/
static PATM_DMA_BD QueueRemove( UINT32 ulOfs )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PRXFREEQUEUES pQueue = &pGi->pRxFreeQs[ulOfs];
    PATM_DMA_BD pBd;
    UINT32 ulTailIdx = pQueue->ulQTail;
    UINT32 ulFlags;

    local_save_flags(ulFlags);
    local_irq_disable();

    if( pQueue->ulQHead != ulTailIdx )
    {
        UINT32 *pulQBase = pGi->pulRxFreeQAddrs[ulOfs];
        pBd = (PATM_DMA_BD) KSEG0ADDR(pulQBase[ulTailIdx]);
        ulTailIdx = (ulTailIdx + 1) % (pQueue->ulQLen + 1);
        pQueue->ulQTail = ulTailIdx;
    }
    else
        pBd = NULL;

    local_irq_restore(ulFlags);

    return( pBd );
} /* QueueRemove */


/***************************************************************************
 * Function Name: bcmxtmrt_freeskbordata
 * Description  : Put socket buffer header back onto the free list or a data
 *                buffer back on to the BD ring. 
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_freeskbordata( PATM_DMA_BD pBd, struct sk_buff *skb,
    unsigned nFlag )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    local_bh_disable();

    if( nFlag & RETFREEQ_SKB )
    {
        skb->retfreeq_context = pGi->pFreeRxSkbList;
        pGi->pFreeRxSkbList = skb;
    }
    else
    {
        /* Compute region to writeback flush invalidate */
        UINT8 * pEnd = skb->end + offsetof(struct skb_shared_info, frags);
        if ( unlikely( skb_shinfo(skb)->nr_frags ))
            pEnd += ( sizeof(skb_frag_t) * MAX_SKB_FRAGS );

        /* DMA never sees RXBUF_HEAD_RESERVE at head of pucData */
        cache_wbflush_region(skb->head + RXBUF_HEAD_RESERVE, pEnd);

        pBd->u2.ucFreeRbl = PKT_Q_BUF_SIZE_EXP;
        QueueAdd( REG_FREE_PKT_Q_OFS, pBd );
    }

    local_bh_enable();
} /* bcmxtmrt_freeskbordata */


/***************************************************************************
 * Function Name: bcmxtmrt_isr
 * Description  : Interrupt service routine that is called when there is an
 *                ATM interrupt.
 * Returns      : IRQ_HANDLED
 ***************************************************************************/
static irqreturn_t bcmxtmrt_isr(int nIrq, PBCMXTMRT_GLOBAL_INFO pGi)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    UINT32 ulIrqSts = *pGi->pulIrqStatus;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( (pDevCtx = pGi->pDevCtxs[i]) != NULL &&
            pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
        {
            netif_rx_schedule(pDevCtx->pDev);
            break;
        }
    }

    /* If no network devices have been created, remove all packets from receive
     * queues and put them on free queues.
     */
    if( i == MAX_DEV_CTXS )
    {
        volatile PATM_DMA_BD pBd;

        while( (pBd = QueueRemove( REG_RX_CELL_Q_OFS )) != NULL )
        {
            pBd->u2.ucFreeRbl = CELL_Q_BUF_SIZE_EXP;
            QueueAdd( REG_FREE_CELL_Q_OFS, pBd );
        }

        while( (pBd = QueueRemove( REG_RX_PKT_Q_OFS )) != NULL )
        {
            pBd->u2.ucFreeRbl = PKT_Q_BUF_SIZE_EXP;
            QueueAdd( REG_FREE_PKT_Q_OFS, pBd );
        }

        /* Clear interrupt bits and reenable ATM interrupt. */
        *pGi->pulIrqStatus = ulIrqSts;
        BcmHalInterruptEnable( INTERRUPT_ID_ATM );
    }

    return( IRQ_HANDLED );
} /* bcmxtmrt_isr */


/***************************************************************************
 * Function Name: bcmxtmrt_poll
 * Description  : Hardware interrupt that is called when a packet is received
 *                on one of the receive queues.
 * Returns      : IRQ_HANDLED
 ***************************************************************************/
static int bcmxtmrt_poll(struct net_device * dev, int * budget)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 ulIrqSts = *pGi->pulIrqStatus;
    UINT32 work_to_do = min(dev->quota, *budget);
    UINT32 work_done = 0;
    UINT32 ret_done = 0;
    volatile PRXFREEQUEUES pQ;

    pQ = &pGi->pRxFreeQs[REG_RX_CELL_Q_OFS];
    if( pQ->ulQHead != pQ->ulQTail )
    {
        work_done = bcmxtmrt_rxtask(work_to_do, REG_FREE_CELL_Q_OFS,
            REG_RX_CELL_Q_OFS, CELL_Q_BUF_SIZE_EXP);
        ret_done = work_done & XTM_POLL_DONE;
        work_done &= ~XTM_POLL_DONE;
        *budget -= work_done;
        dev->quota -= work_done;
    }

    if (work_done < work_to_do && ret_done != XTM_POLL_DONE)
    {
        work_done = bcmxtmrt_rxtask(work_to_do, REG_FREE_PKT_Q_OFS,
            REG_RX_PKT_Q_OFS, PKT_Q_BUF_SIZE_EXP);
        ret_done = work_done & XTM_POLL_DONE;
        work_done &= ~XTM_POLL_DONE;
        *budget -= work_done;
        dev->quota -= work_done;
        if (work_done < work_to_do && ret_done != XTM_POLL_DONE)
        {
            /* Did as much as could, but we are not done yet */
            return 1;
        }
    }

    /* We are done */
    netif_rx_complete(dev);

    /* Clear interrupt bits and reenable ATM interrupt. */
    *pGi->pulIrqStatus = ulIrqSts;
    BcmHalInterruptEnable( INTERRUPT_ID_ATM );

    return 0;
} /* bcmxtmrt_poll */


/***************************************************************************
 * Function Name: bcmxtmrt_rxtask
 * Description  : Linux task that processes an ATM interrupt.
 * Returns      : None.
 ***************************************************************************/
static UINT32 bcmxtmrt_rxtask( UINT32 ulBudget, UINT32 ulFreeQOfs,
    UINT32 ulRxQOfs, UINT32 ulQBufSizeExp )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    UINT8 ucVcid;
    volatile PATM_DMA_BD pBd;
    volatile PRXFREEQUEUES pQ;
    struct sk_buff *skb;
    UINT32 ulRxPktGood = 0;
    UINT32 ulRxPktProcessed = 0;
    UINT32 ulRxPktMax = ulBudget + (ulBudget / 2);

    pQ = &pGi->pRxFreeQs[ulRxQOfs];
    while( (pBd = QueueRemove( ulRxQOfs )) != NULL )
    {
        ucVcid = pBd->ucRxPortId_Vcid & BD_RX_VCID_MASK;
        if( ucVcid <  MAX_DEV_CTXS )
            pDevCtx = pGi->pDevCtxsByVcid[ucVcid];
        else
            pDevCtx = NULL;

        if( pDevCtx == NULL || (pBd->ulFlags_NextRxBd & BD_FLAG_EOP) == 0 )
        {
            /* Chained BDs are not supported.  Discard the packet. */
            PATM_DMA_BD pBd2;
            UINT32 ulEop;
            do
            {
                pBd2 = BD_GET_CADDR((ATM_DMA_BD *), pBd->ulFlags_NextRxBd);
                ulEop = pBd->ulFlags_NextRxBd & BD_FLAG_EOP;

                pBd->u2.ucFreeRbl = ulQBufSizeExp;
                QueueAdd( ulFreeQOfs, pBd );
                ulRxPktProcessed++;

                pBd = pBd2;
            } while( ulEop == 0 );

            if( pDevCtx )
                pDevCtx->DevStats.rx_dropped++;

            if( ulRxPktProcessed >= ulRxPktMax )
                break;
        }
        else
        {
            UINT8 ucCt = (UINT8) (pBd->ulCt_BufPtr >> BD_CT_SHIFT);
            if( ucCt == CT_AAL5 && pBd->u1.ucRxAtmErrors == 0 &&
                (pBd->ucRxAalErrors_RblHigh & BD_RX_AAL_ERROR_MASK) == 0 )
            {
                UINT8 *pucData = BD_GET_CADDR((UINT8 *), pBd->ulCt_BufPtr);

                /* Get an skb to return to the network stack. */
                skb = pGi->pFreeRxSkbList;
                pGi->pFreeRxSkbList =
                    pGi->pFreeRxSkbList->retfreeq_context;

                if( pDevCtx->ulHdrType == HT_TYPE_LLC_SNAP_ETHERNET &&
                    pucData[7] == 0x01 )
                {
                    pBd->usLength -= 4; /* strip FCS */
                }

                /* Remove RFC2684 header. */
                pucData += HT_LEN(pDevCtx->ulHdrType);
                pBd->usLength -= HT_LEN(pDevCtx->ulHdrType);

                if( pBd->usLength < ETH_ZLEN &&
                    (pDevCtx->ulHdrType == HT_LLC_SNAP_ETHERNET ||
                     pDevCtx->ulHdrType == HT_VC_MUX_ETHERNET) )
                {
                    pBd->usLength = ETH_ZLEN;
                }

                skb_hdrinit(RXBUF_HEAD_RESERVE, pBd->usLength, skb,pucData,
                    (void *) bcmxtmrt_freeskbordata, pBd, FROM_WAN);

                __skb_trim(skb, pBd->usLength);

                skb->dev = pDevCtx->pDev;
            }
            else
            {
                /* ATM cell error, AAL5 packet error or AAL5 packet to
                 * be reassembled.  Process cell if it is not an AAL5
                 * packet error.
                 */
                if( ucCt != CT_AAL5 ||
                    (pBd->ucRxAalErrors_RblHigh & BD_RX_AAL_ERROR_MASK) == 0 )
                {
                    if( (skb = ProcessReceiveCellQueue( pBd )) != NULL )
                        pDevCtx = (PBCMXTMRT_DEV_CONTEXT) skb->dev->priv;
                }
                else
                {
                    pDevCtx->DevStats.rx_dropped++;
                    skb = NULL;
                    if( ++ulRxPktProcessed >= ulRxPktMax )
                        break;
                }

                pBd->u2.ucFreeRbl = ulQBufSizeExp;
                QueueAdd( ulFreeQOfs, pBd );
            }

            if( skb )
            {
                pDevCtx->pDev->last_rx = jiffies;
                pDevCtx->DevStats.rx_packets++;
                pDevCtx->DevStats.rx_bytes += skb->len;

                ulRxPktProcessed++;
                ulRxPktGood++;

                ulBudget--;

                if( pDevCtx->szMirrorIntfIn[0] != '\0' &&
                    (pDevCtx->ulHdrType ==  HT_PTM ||
                     pDevCtx->ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
                     pDevCtx->ulHdrType ==  HT_VC_MUX_ETHERNET) )
                {
                    MirrorPacket( skb, pDevCtx->szMirrorIntfIn );
                }

                /* CONFIG_BLOG */
                if ( blog_init( skb, skb->dev,
                                0,
                                0,
                                pDevCtx->ulEncapType ) == PKT_DONE )
                {   /* if packet consumed, proceed to next packet */
                    continue;   /* next packet */
                }

                switch( pDevCtx->ulHdrType )
                {
                case HT_LLC_SNAP_ROUTE_IP:
                case HT_VC_MUX_IPOA:
                    /* IPoA */
                    skb->protocol = htons(ETH_P_IP);
                    skb->mac.raw = skb->data;

                    /* Give the received packet to the network stack. */
                    netif_receive_skb(skb);
                    break;

                case HT_LLC_ENCAPS_PPP:
                case HT_VC_MUX_PPPOA:

                    ppp_input(&pDevCtx->Chan, skb);

                    break;

                default:
                    /* bridge, MER, PPPoE */
                    skb->protocol = eth_type_trans(skb,pDevCtx->pDev);

                    /* Give the received packet to the network stack. */
                    netif_receive_skb(skb);
                    break;
                }

                if( ulBudget == 0 )
                    break;
            }
        }
    }

    if( pBd == NULL )
        ulRxPktGood |= XTM_POLL_DONE;

    return( ulRxPktGood );
} /* bcmxtmrt_rxtask */


/***************************************************************************
 * Function Name: ProcessReceiveCellQueue
 * Description  : This function processes cells received on the receive
 *                cell queue.
 * Returns      : socket buffer address or NULL
 ***************************************************************************/
static struct sk_buff *ProcessReceiveCellQueue( volatile PATM_DMA_BD pBd )
{
    const UINT32 ulAtmHdrSize = 4; /* no HEC */
    const UINT32 ulUserData  = (0x03 << 1);
    const UINT32 ulOamF5Segment  = (0x04 << 1);
    const UINT32 ulOamF5EndToEnd = (0x05 << 1);
    const UINT16 usOamF4VciSeg = 3;
    const UINT16 usOamF4VciEnd = 4;

    struct sk_buff *pRetSkb = NULL;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    UINT8 ucPti, ucCt, ucVcid, *pucData;
    XTMRT_CELL Cell;
    UINT32 ulAtmHdr;
    UINT16 usVpi, usVci;
    int nProcessOam = 1;

    pucData = BD_GET_CADDR((UINT8 *), pBd->ulCt_BufPtr);
    switch( pBd->u1.ucRxAtmErrors )
    {
    case RXATM_INVALID_VPI_VCI:
        ulAtmHdr = *(UINT32 *) pucData;
        usVpi = (UINT16) ((ulAtmHdr >> 20) & 0x00ff);
        usVci = (UINT16) ((ulAtmHdr >>  4) & 0xffff);
        ucPti = (UINT8) ulAtmHdr & 0x0e;
        pDevCtx = FindDevCtx(usVpi, usVci);

        if( pDevCtx && ucPti <= ulUserData )
        {
            /* If the cell is on a VCC from one of this driver's network
             * devices then reassemble it into an AAL5 packet.  (This will
             * only be needed on a BCM6338.)
             */
            pRetSkb = ReassembleCell(pDevCtx, pucData);
        }
        else
        {
            if( usVci == usOamF4VciSeg )
                Cell.ucCircuitType = CTYPE_OAM_F4_SEGMENT;
            else
                if( usVci == usOamF4VciEnd )
                    Cell.ucCircuitType = CTYPE_OAM_F4_END_TO_END;
                else
                    if( pDevCtx && ucPti == ulOamF5Segment )
                        Cell.ucCircuitType = CTYPE_OAM_F5_SEGMENT;
                    else
                        if( pDevCtx && ucPti == ulOamF5EndToEnd )
                            Cell.ucCircuitType = CTYPE_OAM_F5_END_TO_END;
                        else
                            nProcessOam = 0;

            if( nProcessOam == 1 )
            {
                /* Process OAM cell. */
                Cell.ConnAddr.u.Vcc.ulPortMask = (pBd->ucRxPortId_Vcid &
                    BD_RX_PORT_ID_MASK) >> BD_RX_PORT_ID_SHIFT;
                Cell.ConnAddr.u.Vcc.ulPortMask = 
                    PORT_TO_PORTID(Cell.ConnAddr.u.Vcc.ulPortMask);
                Cell.ConnAddr.u.Vcc.usVpi = usVpi;
                Cell.ConnAddr.u.Vcc.usVci = usVci;
                memcpy(Cell.ucData, pucData, pBd->usLength);
                pDevCtx = pGi->pDevCtxs[0];
            }
        }
        break;

    case 0: /* no error */
        /* Process OAM F5 cell. */
        ucCt = (UINT8) (pBd->ulCt_BufPtr >> BD_CT_SHIFT);
        ucVcid = pBd->ucRxPortId_Vcid & BD_RX_VCID_MASK;
        if( (ucCt == CT_OAM_F5_SEGMENT || ucCt == CT_OAM_F5_END_TO_END) &&
            ucVcid < MAX_DEV_CTXS )
        {
            pDevCtx = pGi->pDevCtxsByVcid[ucVcid];
        }

        if( pDevCtx )
        {
            memcpy(&Cell.ConnAddr, &pDevCtx->Addr, sizeof(XTM_ADDR));
            Cell.ucCircuitType = (ucCt == CT_OAM_F5_SEGMENT)
                ? CTYPE_OAM_F5_SEGMENT : CTYPE_OAM_F5_END_TO_END;
            memcpy(Cell.ucData + ulAtmHdrSize, pucData, pBd->usLength);
            *(UINT32 *) &Cell.ucData[0] = (Cell.ConnAddr.u.Vcc.usVpi << 20) |
                (Cell.ConnAddr.u.Vcc.usVci<<4) | ((ucCt==CT_OAM_F5_SEGMENT)
                ?  ulOamF5Segment : ulOamF5EndToEnd);
        }
        else
            nProcessOam = 0;
        break;

    default:
        nProcessOam = 0;
        break;
    }

    if( pDevCtx && nProcessOam && pGi->pfnOamHandler )
    {
        (*pGi->pfnOamHandler) ((XTMRT_HANDLE)pDevCtx, XTMRTCB_CMD_CELL_RECEIVED,
            &Cell, pGi->pOamContext);
    }

    return( pRetSkb );
} /* ProcessReceiveCellQueue */


/***************************************************************************
 * Function Name: ReassembleCell
 * Description  : Reassembles an ATM cell into an AAL5 packet.  Used by
 *                BCM6338 to support more tha four PVCs.
 * Returns      : socket buffer address or NULL
 ***************************************************************************/
static struct sk_buff *ReassembleCell( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    UINT8 *pucCell )
{

    const UINT32 kAtmHdrPtiOfs        = 3;
    const UINT32 kAtmCellDataSize     = 48;
    const UINT32 kAtmCellHdrSizeNoHec = 4;
    const UINT32 kAtmHdrPtiNoCongSDU1 = 1;
    const UINT32 kAtmHdrPtiCongSDU1   = 3;
    const UINT32 kAal5TrailerLength   = 8;
    const UINT32 kAal5CrcInit         = 0xffffffff;
    const UINT32 kAal5GoodCrc         = 0xC704DD7B;

    struct sk_buff *pRetSkb = NULL;
    struct sk_buff *pSkb = pDevCtx->pRxSoftSarSkb;

    if( pSkb == NULL )
    {
        pSkb = dev_alloc_skb(RXBUF_HEAD_RESERVE+MAX_MTU_SIZE+kAtmCellDataSize);
        if( pSkb )
        {
            pDevCtx->pRxSoftSarSkb = pSkb;
            pDevCtx->ulRxSoftSarCrc = kAal5CrcInit;
            skb_pull(pSkb, RXBUF_HEAD_RESERVE);
        }
    }

    if( pSkb )
    {
        UINT8 ucPti = (pucCell[kAtmHdrPtiOfs] & 0x0f) >> 1;
        UINT32 i;

        if( pSkb->tail + kAtmCellDataSize > pSkb->end )
        {
            /* The received packet is too big.  This is probably caused by
             * losing an EOP cell.  Reset the socket buffer.  When an EOP
             * cell is received, the packet will be dropped due to a bad CRC.
             */
            pDevCtx->DevStats.rx_errors++;
            pSkb->data = pSkb->head + RXBUF_HEAD_RESERVE;
            pSkb->tail = pSkb->data;
            pSkb->len = 0;
            pDevCtx->ulRxSoftSarCrc = kAal5CrcInit;
        }

        pucCell += kAtmCellHdrSizeNoHec;
        memcpy(pSkb->data + pSkb->len, pucCell, kAtmCellDataSize);
        pSkb->len += kAtmCellDataSize;
        pSkb->tail += kAtmCellDataSize;

        for( i = 0; i < kAtmCellDataSize; i++ )
        {
            pDevCtx->ulRxSoftSarCrc =
                Aal5UpdateCrc( pDevCtx->ulRxSoftSarCrc, *pucCell );
            pucCell++;
        }

        if( ucPti == kAtmHdrPtiNoCongSDU1 || ucPti == kAtmHdrPtiCongSDU1 )
        {
            UINT32 ulPadLen, ulPktLen = 0;
            if( (pSkb->len & 0x01) == 0 )
                ulPktLen = *(UINT16 *) (pSkb->tail - 6);
            ulPadLen = pSkb->len - ulPktLen;

            if( kAal5GoodCrc == pDevCtx->ulRxSoftSarCrc &&
                ulPadLen <= kAtmCellDataSize + kAal5TrailerLength )
            {
                /* Complete AAL5 packet successfully received. Set length
                 * to AAL5 packet length and then remove the RFC2684 header.
                 */
                __skb_trim(pSkb, ulPktLen);
                skb_pull(pSkb, HT_LEN(pDevCtx->ulHdrType));
                pSkb->dev = pDevCtx->pDev;
                pRetSkb = pSkb;
                pDevCtx->pRxSoftSarSkb = NULL;
            }
            else
            {
                /* Error, reset socket buffer. */
                pDevCtx->DevStats.rx_errors++;
                pSkb->data = pSkb->head + RXBUF_HEAD_RESERVE;
                pSkb->tail = pSkb->data;
                pSkb->len = 0;
                pDevCtx->ulRxSoftSarCrc = kAal5CrcInit;
            }
        }
    }

    return( pRetSkb );
} /* ReassembleCell */


/***************************************************************************
 * Function Name: MirrorPacket
 * Description  : This function sends a sent or received packet to a LAN port.
 *                The purpose is to allow packets sent and received on the WAN
 *                to be captured by a protocol analyzer on the Lan for debugging
 *                purposes.
 * Returns      : None.
 ***************************************************************************/
static void MirrorPacket( struct sk_buff *skb, char *intfName )
{
    struct sk_buff *skbClone;
    struct net_device *netDev;

    if( (skbClone = skb_clone(skb, GFP_ATOMIC)) != NULL )
    {
        if( (netDev = __dev_get_by_name(intfName)) != NULL )
        {
            unsigned long flags;

            blog_xfer(skb, skbClone);
            skbClone->dev = netDev;
            skbClone->protocol = htons(ETH_P_802_3);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(skbClone) ;
            local_irq_restore(flags);
        }
        else
            dev_kfree_skb(skbClone);
    }
} /* MirrorPacket */


/***************************************************************************
 * Function Name: bcmxtmrt_timer
 * Description  : Periodic timer that calls the send function to free packets
 *                that have been transmitted.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_timer( PBCMXTMRT_GLOBAL_INFO pGi )
{
    UINT32 i;
    volatile PRXFREEQUEUES pRxPktQ = &pGi->pRxFreeQs[REG_RX_PKT_Q_OFS];

    /* Free transmitted buffers. */
    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( pGi->pDevCtxs[i] )
        {
            PBCMXTMRT_DEV_CONTEXT pDevCtx = pGi->pDevCtxs[i];

            bcmxtmrt_xmit( NULL, pDevCtx->pDev );

            /* On the BCM6338, the "Receive Packet Queue Almost Full" interrupt
             * does not interrupt at the last queue index.  Therefore, schedule
             * receive when the packet at that index has been received.  There
             * should be no harm to do this check on BCM6348 and BCM6358.
             */
            if( pRxPktQ->ulQTail == pRxPktQ->ulQLen )
            {
                if( pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
                    netif_rx_schedule(pDevCtx->pDev);
            }
        }
    }

    /* Restart the timer. */
    pGi->Timer.expires = jiffies + SAR_TIMEOUT;
    add_timer(&pGi->Timer);
} /* bcmxtmrt_timer */


/***************************************************************************
 * Function Name: bcmxtmrt_request
 * Description  : Request from the bcmxtmcfg driver.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int bcmxtmrt_request( XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParm )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = (PBCMXTMRT_DEV_CONTEXT) hDev;
    int nRet = 0;

    switch( ulCommand )
    {
    case XTMRT_CMD_GLOBAL_INITIALIZATION:
        nRet = DoGlobInitReq( (PXTMRT_GLOBAL_INIT_PARMS) pParm );
        break;

    case XTMRT_CMD_CREATE_DEVICE:
        nRet = DoCreateDeviceReq( (PXTMRT_CREATE_NETWORK_DEVICE) pParm );
        break;

    case XTMRT_CMD_GET_DEVICE_STATE:
        *(UINT32 *) pParm = pDevCtx->ulOpenState;
        break;

    case XTMRT_CMD_SET_ADMIN_STATUS:
        pDevCtx->ulAdminStatus = (UINT32) pParm;
        break;

    case XTMRT_CMD_REGISTER_CELL_HANDLER:
        nRet = DoRegCellHdlrReq( (PXTMRT_CELL_HDLR) pParm );
        break;

    case XTMRT_CMD_UNREGISTER_CELL_HANDLER:
        nRet = DoUnregCellHdlrReq( (PXTMRT_CELL_HDLR) pParm );
        break;

    case XTMRT_CMD_LINK_STATUS_CHANGED:
        nRet = DoLinkStsChangedReq(pDevCtx, (PXTMRT_LINK_STATUS_CHANGE)pParm);
        break;

    case XTMRT_CMD_SEND_CELL:
        nRet = DoSendCellReq( pDevCtx, (PXTMRT_CELL) pParm );
        break;

    case XTMRT_CMD_DELETE_DEVICE:
        nRet = DoDeleteDeviceReq( pDevCtx );
        break;

    case XTMRT_CMD_SET_TX_QUEUE:
        nRet = DoSetTxQueue( pDevCtx, (PXTMRT_TRANSMIT_QUEUE_ID) pParm );
        break;

    case XTMRT_CMD_UNSET_TX_QUEUE:
        nRet = DoUnsetTxQueue( pDevCtx, (PXTMRT_TRANSMIT_QUEUE_ID) pParm );
        break;

    default:
        nRet = -EINVAL;
        break;
    }

    return( nRet );
} /* bcmxtmrt_request */


/***************************************************************************
 * Function Name: DoGlobInitReq
 * Description  : Processes an XTMRT_CMD_GLOBAL_INITIALIZATION command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoGlobInitReq( PXTMRT_GLOBAL_INIT_PARMS pGip )
{
    volatile PRXFREEQUEUES pQ;

    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PATM_DMA_BD pBd, pBdBase, *ppFreeQ, *ppRxQ;
    UINT32 ulBufsToAlloc;
    UINT32 ulAllocAmt;
    UINT8 *p;
    UINT32 i, j, ulSize;

    /* Save registers. */
    pGi->pulIrqStatus = pGip->pulIrqBase + REG_IRQ_STATUS_OFS;
    pGi->pulIrqMask = pGip->pulIrqBase + REG_IRQ_MASK_OFS;
    pGi->pRxFreeQs = (PRXFREEQUEUES) pGip->pulRxQueueBase;
    pGi->pulTxQAddrBase = pGip->pulTxQueueAddrBase;
    pGi->pulTxQHtlBase = pGip->pulTxQueueHtlBase;

    /* Use the first configured receive queue size for the packet queue and
     * the second queue size for the cell queue.  Only packet queue needs
     * socket buffers.
     */
    ulBufsToAlloc = pGip->ulReceiveQueueSizes[0] - 1;

    if( pGi->ulDrvState != XTMRT_UNINITIALIZED )
        nRet = -EPERM;

    /* Allocate free packet queue, receive packet queue and BDs. */
    ulSize = ((ulBufsToAlloc + 1) * (sizeof(UINT32) * 2)) +
        (ulBufsToAlloc * sizeof(ATM_DMA_BD)) + 0x30;
    if( nRet == 0 && (pGi->pRxBdMem = (UINT8 *)
        kmalloc(ulSize, GFP_KERNEL)) != NULL )
    {
        p = pGi->pRxBdMem;
        ppFreeQ = (PATM_DMA_BD *) (((UINT32) p + 0x0f) & ~0x0f);
        p = (UINT8 *) ((UINT32) ppFreeQ + ((ulBufsToAlloc+1)*sizeof(UINT32)));
        ppRxQ = (PATM_DMA_BD *) (((UINT32) p + 0x0f) & ~0x0f);
        p = (UINT8 *) ((UINT32) ppRxQ + ((ulBufsToAlloc+1) * sizeof(UINT32)));
        pBdBase = (PATM_DMA_BD) (((UINT32) p + 0x0f) & ~0x0f);
        p = pGi->pRxBdMem;
        cache_wbflush_len(p, ulSize);

        pGi->pulRxFreeQAddrs[REG_FREE_PKT_Q_OFS]=(UINT32 *)KSEG1ADDR(ppFreeQ);
        pQ = &pGi->pRxFreeQs[REG_FREE_PKT_Q_OFS];
        pQ->ulQAddr = VIRT_TO_PHY(ppFreeQ);
        pQ->ulQHead = 0;
        pQ->ulQTail = 0;
        pQ->ulQLen = ulBufsToAlloc;

        pGi->pulRxFreeQAddrs[REG_RX_PKT_Q_OFS] = (UINT32 *) KSEG1ADDR(ppRxQ);
        pQ = &pGi->pRxFreeQs[REG_RX_PKT_Q_OFS];
        pQ->ulQAddr = VIRT_TO_PHY(ppRxQ);
        pQ->ulQHead = 0;
        pQ->ulQTail = 0;
        pQ->ulQLen = ulBufsToAlloc;
    }
    else
        nRet = -ENOMEM;

    /* Allocate receive socket buffers and data buffers. */
    if( nRet == 0 )
    {
        const UINT32 ulRxAllocSize = SKB_ALIGNED_SIZE + RXBUF_ALLOC_SIZE;
        const UINT32 ulBlockSize = (64 * 1024);
        const UINT32 ulBufsPerBlock = ulBlockSize / ulRxAllocSize;

        /* Allocate one additional socket buffer so the socket buffer chain is
         * never empty.
         */
        if( (p = kmalloc(SKB_ALIGNED_SIZE, GFP_KERNEL)) != NULL )
        {
            memset(p, 0x00, SKB_ALIGNED_SIZE);
            ((struct sk_buff *) p)->retfreeq_context = pGi->pFreeRxSkbList;
            pGi->pFreeRxSkbList = (struct sk_buff *) p;
        }

        j = 0;
        pBd = pBdBase;
        while( ulBufsToAlloc )
        {
            ulAllocAmt = (ulBufsPerBlock < ulBufsToAlloc)
                ? ulBufsPerBlock : ulBufsToAlloc;

            ulSize = ulAllocAmt * ulRxAllocSize;
            if( j < MAX_BUFMEM_BLOCKS &&
                (p = kmalloc(ulSize, GFP_KERNEL)) != NULL )
            {
                UINT8 *p2;

                memset(p, 0x00, ulSize);
                pGi->pBufMem[j++] = p;
                cache_wbflush_len(p, ulSize);
                p = (UINT8 *) (((UINT32) p + 0x0f) & ~0x0f);
                for( i = 0; i < ulAllocAmt; i++ )
                {
                    BD_SET_ADDR(pBd->ulCt_BufPtr, p + RXBUF_HEAD_RESERVE);
                    pBd->u2.ucFreeRbl = PKT_Q_BUF_SIZE_EXP;
                    QueueAdd( REG_FREE_PKT_Q_OFS, pBd );
                    pBd++;

                    p2 = p + RXBUF_ALLOC_SIZE;
                    ((struct sk_buff *) p2)->retfreeq_context =
                        pGi->pFreeRxSkbList;
                    pGi->pFreeRxSkbList = (struct sk_buff *) p2;

                    p += ulRxAllocSize;
                }
                ulBufsToAlloc -= ulAllocAmt;
            }
            else
            {
                nRet = -ENOMEM;
                break;
            }
        }
    }

    /* Allocate all cell queue related information. */
    ulBufsToAlloc = (pGip->ulReceiveQueueSizes[1])
        ? pGip->ulReceiveQueueSizes[1] - 1 : DEFAULT_CELL_Q_SIZE - 1;
    ulSize = ((ulBufsToAlloc + 1) * (sizeof(UINT32) * 2)) +
        (ulBufsToAlloc * (sizeof(ATM_DMA_BD) + CELL_Q_BUF_SIZE)) + 0x40;

    if( nRet == 0 && (pGi->pRxCellMem = (UINT8 *)
        kmalloc(ulSize, GFP_KERNEL)) != NULL )
    {
        p = pGi->pRxCellMem;
        ppFreeQ = (PATM_DMA_BD *) (((UINT32) p + 0x0f) & ~0x0f);
        p = (UINT8 *) ((UINT32) ppFreeQ + ((ulBufsToAlloc+1)*sizeof(UINT32)));
        ppRxQ = (PATM_DMA_BD *) (((UINT32) p + 0x0f) & ~0x0f);
        p = (UINT8 *) ((UINT32) ppRxQ + ((ulBufsToAlloc+1) * sizeof(UINT32)));
        pBdBase = (PATM_DMA_BD) (((UINT32) p + 0x0f) & ~0x0f);
        p = pGi->pRxCellMem;
        cache_wbflush_len(p, ulSize);

        pGi->pulRxFreeQAddrs[REG_FREE_CELL_Q_OFS]=(UINT32 *)KSEG1ADDR(ppFreeQ);
        pQ = &pGi->pRxFreeQs[REG_FREE_CELL_Q_OFS];
        pQ->ulQAddr = VIRT_TO_PHY(ppFreeQ);
        pQ->ulQHead = 0;
        pQ->ulQTail = 0;
        pQ->ulQLen = ulBufsToAlloc;

        pGi->pulRxFreeQAddrs[REG_RX_CELL_Q_OFS] = (UINT32 *) KSEG1ADDR(ppRxQ);
        pQ = &pGi->pRxFreeQs[REG_RX_CELL_Q_OFS];
        pQ->ulQAddr = VIRT_TO_PHY(ppRxQ);
        pQ->ulQHead = 0;
        pQ->ulQTail = 0;
        pQ->ulQLen = ulBufsToAlloc;

        p = (UINT8 *) ((UINT32) pBdBase + (ulBufsToAlloc*sizeof(ATM_DMA_BD)));
        p = (UINT8 *) (((UINT32) p + 0x0f) & ~0x0f);
        for( i = 0, pBd = pBdBase; i < ulBufsToAlloc; i++, pBd++ )
        {
            BD_SET_ADDR(pBd->ulCt_BufPtr, p);
            pBd->u2.ucFreeRbl = CELL_Q_BUF_SIZE_EXP;
            QueueAdd( REG_FREE_CELL_Q_OFS, pBd );
            p += CELL_Q_BUF_SIZE;
        }
    }
    else
        nRet = -ENOMEM;

    if( nRet == 0 )
    {
        BcmHalInterruptDisable( INTERRUPT_ID_ATM );
        BcmHalMapInterrupt((FN_HANDLER) bcmxtmrt_isr, (UINT32) pGi,
            INTERRUPT_ID_ATM);
        BcmHalInterruptEnable( INTERRUPT_ID_ATM );
        *pGi->pulIrqMask = ~INTR_MASK;

        /* Initialize a timer function to free transmit buffers. */
        init_timer(&pGi->Timer);
        pGi->Timer.data = (unsigned long) pGi;
        pGi->Timer.function = (void *) bcmxtmrt_timer;

        pGi->ulDrvState = XTMRT_INITIALIZED;
    }
    else
    {
        if( pGi->pRxBdMem )
        {
            kfree(pGi->pRxBdMem);
            pGi->pRxBdMem = NULL;
        }

        for( i = 0; i < MAX_BUFMEM_BLOCKS; i++ )
            if( pGi->pBufMem[i] )
            {
                kfree(pGi->pBufMem[i]);
                pGi->pBufMem[i] = NULL;
            }

        pGi->pFreeRxSkbList = NULL;

        if( pGi->pRxCellMem )
        {
            kfree(pGi->pRxCellMem);
            pGi->pRxCellMem = NULL;
        }
    }

    return( nRet );
} /* DoGlobInitReq */


/***************************************************************************
 * Function Name: DoCreateDeviceReq
 * Description  : Processes an XTMRT_CMD_CREATE_DEVICE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoCreateDeviceReq( PXTMRT_CREATE_NETWORK_DEVICE pCnd )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    struct net_device *dev = NULL;

    if( pGi->ulDrvState != XTMRT_UNINITIALIZED &&
        (dev = alloc_netdev( sizeof(BCMXTMRT_DEV_CONTEXT),
         pCnd->szNetworkDeviceName, ether_setup )) != NULL )
    {
        dev_alloc_name(dev, dev->name);
        SET_MODULE_OWNER(dev);

        pDevCtx = (PBCMXTMRT_DEV_CONTEXT) dev->priv;
        memset(pDevCtx, 0x00, sizeof(BCMXTMRT_DEV_CONTEXT));
        memcpy(&pDevCtx->Addr, &pCnd->ConnAddr, sizeof(XTM_ADDR));
        pDevCtx->ulHdrType = pCnd->ulHeaderType;
        pDevCtx->ulFlags = pCnd->ulFlags;
        pDevCtx->pDev = dev;
        pDevCtx->ulAdminStatus = ADMSTS_UP;
        pDevCtx->ucTxVcid = INVALID_VCID;

        /* Setup the callback functions. */
        dev->open               = bcmxtmrt_open;
        dev->stop               = bcmxtmrt_close;
        dev->hard_start_xmit    = bcmxtmrt_xmit;
        dev->tx_timeout         = bcmxtmrt_timeout;
        dev->watchdog_timeo     = SAR_TIMEOUT;
        dev->get_stats          = bcmxtmrt_query;
        dev->set_multicast_list = NULL;
        dev->do_ioctl           = &bcmxtmrt_ioctl;
        dev->poll               = bcmxtmrt_poll;
        dev->weight             = 64;

        /* identify as a WAN interface to block WAN-WAN traffic */
        dev->priv_flags |= IFF_WANDEV;

        switch( pDevCtx->ulHdrType )
        {
        case HT_LLC_SNAP_ROUTE_IP:
        case HT_VC_MUX_IPOA:
            /* IPoA */
            dev->type = ARPHRD_PPP;
            dev->hard_header_len = HT_LEN_LLC_SNAP_ROUTE_IP;
            dev->mtu = RFC1626_MTU;
            dev->addr_len = 0;
            dev->tx_queue_len = 100;
            pDevCtx->ulEncapType = TYPE_IP;     /* IPoA */
            dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
            break;

        case HT_LLC_ENCAPS_PPP:
        case HT_VC_MUX_PPPOA:
            pDevCtx->ulEncapType = TYPE_PPP;    /*PPPoA*/
            break;

        default: /* bridge, MER, PPPoE */
            /* Read and display the MAC address. */
            dev->dev_addr[0] = 0xff;
            kerSysGetMacAddress( dev->dev_addr,((UINT32)pDevCtx & 0x00ffffff) |
                0x10000000 );
            if( (dev->dev_addr[0] & 0x01) == 0x01 )
            {
                printk( KERN_ERR CARDNAME": Unable to read MAC address from "
                    "persistent storage.  Using default address.\n" );
                memcpy( dev->dev_addr, "\x02\x10\x18\x02\x00\x01", 6 );
            }
            printk( CARDNAME": MAC address: %2.2x %2.2x %2.2x %2.2x %2.2x "
                "%2.2x\n",dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
                dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5] );

            pDevCtx->ulEncapType = TYPE_ETH;    /* bridge, MER, PPPoE, ATM */
            dev->flags = IFF_MULTICAST;
            break;
        }
     
        /* Don't reset or enable the device yet. "Open" does that. */
        nRet = register_netdev(dev);
        if (nRet == 0) 
        {
            UINT32 i;
            for( i = 0; i < MAX_DEV_CTXS; i++ )
                if( pGi->pDevCtxs[i] == NULL )
                {
                    pGi->pDevCtxs[i] = pDevCtx;
                    break;
                }

            pCnd->hDev = (XTMRT_HANDLE) pDevCtx;
        }
        else
        {
            printk(KERN_ERR CARDNAME": register_netdev failed\n");
            free_netdev(dev);
        }

        if( nRet != 0 )
            kfree(pDevCtx);
    }
    else
    {
        printk(KERN_ERR CARDNAME": alloc_netdev failed\n");
        nRet = -ENOMEM;
    }

    return( nRet );
} /* DoCreateDeviceReq */


/***************************************************************************
 * Function Name: DoRegCellHdlrReq
 * Description  : Processes an XTMRT_CMD_REGISTER_CELL_HANDLER command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoRegCellHdlrReq( PXTMRT_CELL_HDLR pCh )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    switch( pCh->ulCellHandlerType )
    {
    case CELL_HDLR_OAM:
        if( pGi->pfnOamHandler == NULL )
        {
            pGi->pfnOamHandler = pCh->pfnCellHandler;
            pGi->pOamContext = pCh->pContext;
        }
        else
            nRet = -EEXIST;
        break;

    case CELL_HDLR_ASM:
        if( pGi->pfnAsmHandler == NULL )
        {
            pGi->pfnAsmHandler = pCh->pfnCellHandler;
            pGi->pAsmContext = pCh->pContext;
        }
        else
            nRet = -EEXIST;
        break;
    }

    return( nRet );
} /* DoRegCellHdlrReq */


/***************************************************************************
 * Function Name: DoUnregCellHdlrReq
 * Description  : Processes an XTMRT_CMD_UNREGISTER_CELL_HANDLER command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnregCellHdlrReq( PXTMRT_CELL_HDLR pCh )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    switch( pCh->ulCellHandlerType )
    {
    case CELL_HDLR_OAM:
        if( pGi->pfnOamHandler == pCh->pfnCellHandler )
        {
            pGi->pfnOamHandler = NULL;
            pGi->pOamContext = NULL;
        }
        else
            nRet = -EPERM;
        break;

    case CELL_HDLR_ASM:
        if( pGi->pfnAsmHandler == pCh->pfnCellHandler )
        {
            pGi->pfnAsmHandler = NULL;
            pGi->pAsmContext = NULL;
        }
        else
            nRet = -EPERM;
        break;
    }

    return( nRet );
} /* DoUnregCellHdlrReq */


/***************************************************************************
 * Function Name: DoLinkStsChangedReq
 * Description  : Processes an XTMRT_CMD_LINK_STATUS_CHANGED command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkStsChangedReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc )
{
    int nRet = -EPERM;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] == pDevCtx )
        {
            if( pLsc->ulLinkState == LINK_UP )
                nRet = DoLinkUp( pDevCtx, pLsc );
            else
                nRet = DoLinkDown( pDevCtx, pLsc );

            pDevCtx->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;
            pDevCtx->MibInfo.ulIfSpeed = pLsc->ulLinkUsRate;
            break;
        }

    return( nRet );
} /* DoLinkStsChangedReq */


/***************************************************************************
 * Function Name: DoLinkUp
 * Description  : Processes a "link up" condition.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkUp( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId;
    UINT32 i;
    int nInSoftIrq = in_softirq();

    if( !nInSoftIrq )
        local_bh_disable();

    /* Initialize transmit DMA channel information. */
    pDevCtx->ucTxVcid = pLsc->ucTxVcid;
    pDevCtx->ulLinkState = pLsc->ulLinkState;
    pDevCtx->ulTxQueuesSize = 0;

    /* Use each Rx vcid as an index into an array of bcmxtmrt devices
     * context structures.
     */
    for( i = 0; i < pLsc->ulRxVcidsSize; i++ )
        pGi->pDevCtxsByVcid[pLsc->ucRxVcids[i]] = pDevCtx;

    for( i = 0, pTxQId = pLsc->TransitQueueIds;
         i < pLsc->ulTransmitQueueIdsSize && nRet == 0; i++, pTxQId++ )
    {
        nRet = DoSetTxQueue(pDevCtx, pTxQId );
    }

    if( nRet == 0 )
    {
        /* If it is not already there, put the driver into a "ready to send and
         * receive state".
         */
        if( pGi->ulDrvState == XTMRT_INITIALIZED )
        {
            pGi->Timer.expires = jiffies + SAR_TIMEOUT;
            add_timer(&pGi->Timer);

            if( pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
                netif_start_queue(pDevCtx->pDev);

            pGi->ulDrvState = XTMRT_RUNNING;
        }

        if( !nInSoftIrq )
            local_bh_enable();
    }
    else
    {
        /*Memory allocation error. Free memory that was previously allocated.*/
        PTXQINFO pTxQInfo;

        if( !nInSoftIrq )
            local_bh_enable();

        for( i = 0; i < pDevCtx->ulTxQueuesSize; i++ )
        {
            pTxQInfo = pDevCtx->pTxQueues[i];
            if( pTxQInfo->pMemBuf && --pTxQInfo->ulUseCount == 0 )
            {
                kfree(pTxQInfo->pMemBuf);
                pTxQInfo->pMemBuf = NULL;
                memset(&pGi->TxQInfos[pTxQInfo->ulDmaIndex], 0x00,
                    sizeof(TXQINFO));
            }
        }
        pDevCtx->ulTxQueuesSize = 0;
    }

    return( nRet );
} /* DoLinkUp */


/***************************************************************************
 * Function Name: DoLinkDown
 * Description  : Processes a "link down" condition.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkDown( PBCMXTMRT_DEV_CONTEXT pDevCtx,
 PXTMRT_LINK_STATUS_CHANGE pLsc )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PTXQINFO pTxQInfo;
    UINT32 i, ulStopRunning;
    int nInSoftIrq = in_softirq();

    if( !nInSoftIrq )
        netif_stop_queue(pDevCtx->pDev);

    /* Free transmitted packets. */
    bcmxtmrt_xmit( NULL, pDevCtx->pDev );

    /* Free memory used for transmit queues. */
    for(i = 0; i < pDevCtx->ulTxQueuesSize; i++ )
    {
        pTxQInfo = pDevCtx->pTxQueues[i];
        if( pTxQInfo->pMemBuf && --pTxQInfo->ulUseCount == 0 )
        {
            kfree(pTxQInfo->pMemBuf);
            pTxQInfo->pMemBuf = NULL;
            pTxQInfo->ulNumTxBufsQdOne = 0;
            memset(&pGi->TxQInfos[pTxQInfo->ulDmaIndex], 0x00,
                sizeof(TXQINFO));
        }
    }

    /* Zero transmit related data structures. */
    pDevCtx->ulTxQueuesSize = 0;
    memset(pDevCtx->pTxQueues, 0x00, sizeof(pDevCtx->pTxQueues));
    memset(pDevCtx->pTxPriorities, 0x00, sizeof(pDevCtx->pTxPriorities));
    pDevCtx->ulLinkState = pLsc->ulLinkState;
    pDevCtx->ucTxVcid = INVALID_VCID;
    pGi->ulNumTxBufsQdAll = 0;

    /* Zero receive vcids. */
    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxsByVcid[i] == pDevCtx )
            pGi->pDevCtxsByVcid[i] = NULL;

    /* Return the software SAR receive skb. */
    if( pDevCtx->pRxSoftSarSkb )
    {
        dev_kfree_skb_any(pDevCtx->pRxSoftSarSkb);
        pDevCtx->pRxSoftSarSkb = NULL;
    }

    /* If all links are down, put the driver into an "initialized" state. */
    for( i = 0, ulStopRunning = 1; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] && pGi->pDevCtxs[i]->ulLinkState == LINK_UP )
        {
            ulStopRunning = 0;
            break;
        }

    if( ulStopRunning )
    {
        del_timer_sync(&pGi->Timer);
        pGi->ulDrvState = XTMRT_INITIALIZED;
    }

    if( !nInSoftIrq )
        netif_wake_queue(pDevCtx->pDev);

    return( nRet );
} /* DoLinkDown */


/***************************************************************************
 * Function Name: DoSetTxQueue
 * Description  : Allocate memory for and initialize a transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PTXQINFO pTxQInfo =  &pGi->TxQInfos[pTxQId->ulQueueIndex];
    UINT32 ulPort = PORTID_TO_PORT(pTxQId->ulPortId);
    UINT32 ulQueueSize, ulSize;
    UINT8 *p;

    /* Set every transmit queue size to the number of external buffers.
     * The QueuePacket function will control how many packets are queued.
     * The maximum transmit queue size is 256.
     */
    ulQueueSize = (pGi->ulNumExtBufs < 250) ? pGi->ulNumExtBufs : 250;

    if( pTxQInfo->ulUseCount++ == 0 )
    {
        ulSize = (ulQueueSize * (sizeof(struct sk_buff *) +
            sizeof(ATM_DMA_BD))) + 0x20; /* 0x20 for alignment */

        if( (pTxQInfo->pMemBuf = kmalloc(ulSize, GFP_ATOMIC)) != NULL )
        {
            memset(pTxQInfo->pMemBuf, 0x00, ulSize);
            cache_wbflush_len(pTxQInfo->pMemBuf, ulSize);

            if( ulPort < MAX_PHY_PORTS &&
                pTxQId->ulSubPriority < MAX_SUB_PRIORITIES )
            {
                UINT8 *pucHtl;

                p = (UINT8 *) (((UINT32) pTxQInfo->pMemBuf + 0x0f) & ~0x0f);
                pTxQInfo->ulPort = ulPort;
                pTxQInfo->ulSubPriority = pTxQId->ulSubPriority;
                pTxQInfo->ulQueueSize = ulQueueSize;
                pTxQInfo->ulDmaIndex = pTxQId->ulQueueIndex;
                pGi->pulTxQAddrBase[pTxQInfo->ulDmaIndex] = (UINT32) p;
                pTxQInfo->pQBase = (volatile PATM_DMA_BD) p;
                pucHtl = (UINT8 *) &pGi->pulTxQHtlBase[pTxQInfo->ulDmaIndex];
                pTxQInfo->pucHeadIdx = pucHtl + 1;
                pTxQInfo->pucTailIdx = pucHtl + 2;
                *pTxQInfo->pucHeadIdx = *pTxQInfo->pucTailIdx = 0;
                pucHtl[3] = pTxQInfo->ulQueueSize - 1;
                pTxQInfo->ulHeadIdx = pTxQInfo->ulCurrBdIdx = 0;
                p += ((sizeof(ATM_DMA_BD) * ulQueueSize) + 0x0f) & ~0x0f;
                pTxQInfo->ppSkbs = (struct sk_buff **) p;
                pTxQInfo->ulNumTxBufsQdOne = 0;
            }
            else
            {
                printk(CARDNAME ": Invalid transmit queue port/priority\n");
                nRet = -EFAULT;
            }
        }
        else
            nRet = -ENOMEM;
    }

    if( nRet == 0 )
    {
        UINT32 j, ulTxQs;

        pDevCtx->pTxQueues[pDevCtx->ulTxQueuesSize++] = pTxQInfo;
        pDevCtx->pTxPriorities[ulPort][pTxQInfo->ulSubPriority] = pTxQInfo;

        /* Count the total number of transmit queues used across all device
         * interfaces.
         */
        for( j = 0, ulTxQs = 0; j < MAX_TRANSMIT_QUEUES; j++ )
            if( pGi->TxQInfos[j].pMemBuf )
                ulTxQs++;
        pGi->ulNumTxQs = ulTxQs;
    }

    return( nRet );
} /* DoSetTxQueue */


/***************************************************************************
 * Function Name: DoUnsetTxQueue
 * Description  : Frees memory for a transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnsetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    int nRet = 0;
    UINT32 i, j, ulTxQs;
    PTXQINFO pTxQInfo;

    for( i = 0; i < pDevCtx->ulTxQueuesSize; i++ )
    {
        pTxQInfo = pDevCtx->pTxQueues[i];
        if( pTxQId->ulQueueIndex == pTxQInfo->ulDmaIndex )
        {
            PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
            UINT32 ulPort = PORTID_TO_PORT(pTxQId->ulPortId);

            pDevCtx->pTxPriorities[ulPort][pTxQInfo->ulSubPriority] = NULL;

            /* Shift remaining array elements down by one element. */
            memmove(&pDevCtx->pTxQueues[i], &pDevCtx->pTxQueues[i+1], (pDevCtx->ulTxQueuesSize - i - 1) *
                sizeof(PTXQINFO));
            pDevCtx->ulTxQueuesSize--;

            if( --pTxQInfo->ulUseCount == 0 )
            {
                pGi->pulTxQAddrBase[pTxQInfo->ulDmaIndex] = 0;
                pGi->pulTxQHtlBase[pTxQInfo->ulDmaIndex] = 0;

                if( pTxQInfo->pMemBuf )
                {
                    kfree(pTxQInfo->pMemBuf);
                    pTxQInfo->pMemBuf = NULL;
                }

                memset(&pGi->TxQInfos[pTxQInfo->ulDmaIndex], 0x00,
                    sizeof(TXQINFO));
            }

            /* Count the total number of transmit queues used across all device
             * interfaces.
             */
            for( j = 0, ulTxQs = 0; j < MAX_TRANSMIT_QUEUES; j++ )
                if( pGi->TxQInfos[j].pMemBuf )
                    ulTxQs++;
            pGi->ulNumTxQs = ulTxQs;
            break;
        }
    }

    return( nRet );
} /* DoUnsetTxQueue */


/***************************************************************************
 * Function Name: DoSendCellReq
 * Description  : Processes an XTMRT_CMD_SEND_CELL command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSendCellReq( PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC )
{
    const UINT32 ulAtmHdrSize = 4; /* no HEC */
    const UINT32 ulOamF5Segment  = (0x04 << 1);
    const UINT32 ulOamF5EndToEnd = (0x05 << 1);
    const UINT32 ulVciOamF4Seg = 3;
    const UINT32 ulVciOamF4End = 4;
    int nRet = 0;

    if( pDevCtx->ulLinkState == LINK_UP )
    {
        struct sk_buff *skb = dev_alloc_skb(ulAtmHdrSize + CELL_PAYLOAD_SIZE);

        if( skb )
        {
            UINT32 i, ulVpi, ulVci = 0, ulPti = 0;
            UINT32 ulPort = (pC->ConnAddr.ulTrafficType == TRAFFIC_TYPE_ATM)
                ? pC->ConnAddr.u.Vcc.ulPortMask
                : pC->ConnAddr.u.Flow.ulPortMask;

            /* A network device instance can potentially have transmit queues
             * on different ports. Find a transmit queue for the port specified
             * in the cell structure.  The cell structure should only specify
             * one port.
             */
            for( i = 0; i < MAX_SUB_PRIORITIES; i++ )
            {
                if( pDevCtx->pTxPriorities[ulPort][i] )
                {
                    skb->mark = i;
                    break;
                }
            }

            skb->dev = pDevCtx->pDev;
            __skb_put(skb, ulAtmHdrSize + CELL_PAYLOAD_SIZE);
            memcpy(skb->data + ulAtmHdrSize, pC->ucData, CELL_PAYLOAD_SIZE);

            ulVpi = pC->ConnAddr.u.Vcc.usVpi;
            switch( pC->ucCircuitType )
            {
            case CTYPE_OAM_F5_SEGMENT:
                ulVci = pC->ConnAddr.u.Vcc.usVci;
                ulPti = ulOamF5Segment;
                break;

            case CTYPE_OAM_F5_END_TO_END:
                ulVci = pC->ConnAddr.u.Vcc.usVci;
                ulPti = ulOamF5EndToEnd;
                break;

            case CTYPE_OAM_F4_SEGMENT:
                ulVci = ulVciOamF4Seg;
                ulPti = 0;
                break;

            case CTYPE_OAM_F4_END_TO_END:
                ulVci = ulVciOamF4End;
                ulPti = 0;
                break;
            }

            skb->protocol = SKB_PROTO_ATM_CELL;
            *(UINT32 *) &skb->data[0] = (ulVpi << 20) | (ulVci <<  4) | ulPti;

            bcmxtmrt_xmit( skb, pDevCtx->pDev);
        }
        else
            nRet = -ENOMEM;
    }
    else
        nRet = -EPERM;

    return( nRet );
} /* DoSendCellReq */


/***************************************************************************
 * Function Name: DoDeleteDeviceReq
 * Description  : Processes an XTMRT_CMD_DELETE_DEVICE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoDeleteDeviceReq( PBCMXTMRT_DEV_CONTEXT pDevCtx )
{
    int nRet = -EPERM;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] == pDevCtx )
        {
            pGi->pDevCtxs[i] = NULL;

            kerSysReleaseMacAddress( pDevCtx->pDev->dev_addr );

            unregister_netdev( pDevCtx->pDev );
            free_netdev( pDevCtx->pDev );

            nRet = 0;
            break;
        }

    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxsByVcid[i] == pDevCtx )
            pGi->pDevCtxsByVcid[i] = NULL;

    return( nRet );
} /* DoDeleteDeviceReq */


/***************************************************************************
 * Function Name: bcmxtmrt_add_proc_files
 * Description  : Adds proc file system directories and entries.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_add_proc_files( void )
{
    proc_mkdir ("driver/xtm", NULL);
    create_proc_read_entry("driver/xtm/txdmainfo", 0, NULL, ProcDmaTxInfo, 0);

    return(0);
} /* bcmxtmrt_add_proc_files */


/***************************************************************************
 * Function Name: bcmxtmrt_del_proc_files
 * Description  : Deletes proc file system directories and entries.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_del_proc_files( void )
{
    remove_proc_entry("driver/xtm/txdmainfo", NULL);
    remove_proc_entry("driver/xtm", NULL);

    return(0);
} /* bcmxtmrt_del_proc_files */


/***************************************************************************
 * Function Name: ProcDmaTxInfo
 * Description  : Displays information about transmit DMA channels for all
 *                network interfaces.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt, 
    int *eof, void *data)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PTXQINFO pTqi;
    UINT32 i;
    int sz = 0;

    for( i = 0; i < MAX_TRANSMIT_QUEUES; i++ )
        if( pGi->TxQInfos[i].pMemBuf )
        {
            pTqi = &pGi->TxQInfos[i];
            sz += sprintf(page + sz, "tx_chan_size: %lu, tx_chan_filled: %lu\n",
                pTqi->ulQueueSize, pTqi->ulNumTxBufsQdOne);
        }

    sz += sprintf(page + sz, "\next_buf_size: %lu, reserve_buf_size: %lu, tx_"
        "total_filled: %lu\n\n", pGi->ulNumExtBufs, pGi->ulNumExtBufsRsrvd,
        pGi->ulNumTxBufsQdAll);

    sz += sprintf(page + sz, "queue_condition: %lu %lu %lu, drop_condition: "
        "%lu %lu %lu\n\n", pGi->ulDbgQ1, pGi->ulDbgQ2, pGi->ulDbgQ3,
        pGi->ulDbgD1, pGi->ulDbgD2, pGi->ulDbgD3);

    *eof = 1;
    return( sz );
} /* ProcDmaTxInfo */


/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init(bcmxtmrt_init);
module_exit(bcmxtmrt_cleanup);
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(bcmxtmrt_request);

