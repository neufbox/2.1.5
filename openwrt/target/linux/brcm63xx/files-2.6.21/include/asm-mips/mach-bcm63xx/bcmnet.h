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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcmnet.h                                                 */
/*   DATE:    05/16/02                                                 */
/*   PURPOSE: network interface ioctl definition                       */
/*                                                                     */
/***********************************************************************/
#ifndef _IF_NET_H_
#define _IF_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LINKSTATE_DOWN      0
#define LINKSTATE_UP        1

#if defined(CONFIG_BCM96368) || defined(CHIP_6368)
#define NR_RX_BDS_MAX           400
#define NR_RX_BDS(x)            NR_RX_BDS_MAX
#define NR_TX_BDS               (NR_RX_BDS_MAX-20)
#elif defined(CONFIG_BCM96358) || defined(CHIP_6358)
//#define NR_RX_BDS_MAX           520  /* IPv6Ready- conformance test */
#define NR_RX_BDS_MAX           200
#define NR_RX_BDS_MIN           80
#define NR_RX_BDS(dram_size)    ((dram_size > 0x800000) ? NR_RX_BDS_MAX: NR_RX_BDS_MIN)
#define NR_TX_BDS               280
#else
#define NR_RX_BDS_MAX           120
#define NR_RX_BDS_MIN           80
#define NR_RX_BDS(dram_size)    ((dram_size > 0x800000) ? NR_RX_BDS_MAX: NR_RX_BDS_MIN)
#define NR_TX_BDS               200
#endif

/*---------------------------------------------------------------------*/
/* Ethernet Switch Type                                                */
/*---------------------------------------------------------------------*/
#define ESW_TYPE_UNDEFINED                  0
#define ESW_TYPE_BCM5325M                   1
#define ESW_TYPE_BCM5325E                   2
#define ESW_TYPE_BCM5325F                   3
#define ESW_TYPE_BCM5397                    4

/*
 * Ioctl definitions.
 */
/* reserved SIOCDEVPRIVATE */
enum {
    SIOCGLINKSTATE = SIOCDEVPRIVATE + 1,
    SIOCSCLEARMIBCNTR,
    SIOCMIBINFO,
    SIOCSDUPLEX,	/* 0: auto 1: full 2: half */
    SIOCSSPEED,		/* 0: auto 1: 100mbps 2: 10mbps */
    SIOCGENABLEVLAN,
    SIOCGDISABLEVLAN,
    SIOCGQUERYNUMVLANPORTS,
    SIOCGSWITCHTYPE,
    SIOCGQUERYNUMPORTS,
    SIOCGMACTOPORT,
    SIOCGLINKSTATUS,
    SIOCSWANPORT,
    SIOCPORTMIRROR,
    SIOCLAST
};

#define SPEED_10MBIT        10000000
#define SPEED_100MBIT       100000000

typedef struct IoctlMibInfo
{
    unsigned long ulIfLastChange;
    unsigned long ulIfSpeed;
} IOCTL_MIB_INFO, *PIOCTL_MIB_INFO;


#define MIRROR_INTF_SIZE    32
#define MIRROR_DIR_IN       0
#define MIRROR_DIR_OUT      1
#define MIRROR_DISABLED     0
#define MIRROR_ENABLED      1

typedef struct _MirrorCfg
{
    char szMonitorInterface[MIRROR_INTF_SIZE];
    char szMirrorInterface[MIRROR_INTF_SIZE];
    int nDirection;
    int nStatus;
} MirrorCfg ;

#ifdef __cplusplus
}
#endif

#endif /* _IF_NET_H_ */
