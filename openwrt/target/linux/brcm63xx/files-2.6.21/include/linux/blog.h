#ifndef __BLOG_H_INCLUDED__
#define __BLOG_H_INCLUDED__

/*
<:copyright-gpl

 Copyright 2003 Broadcom Corp. All Rights Reserved.

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
 *******************************************************************************
 *
 * File Name  : blog.h
 *
 * Description: This file implements the interface for debug logging of protocol
 *              header information of a packet buffer as it traverses the Linux
 *              networking stack. The header information is logged into a
 *              buffer log 'Blog_t' object associated with the sk_buff that
 *              is managing the packet. Logging is only performed when a sk_buff
 *              has an associated Blog_t object.
 *
 *              When an sk_buff is cloned or copied, the associated Blog_t
 *              object is transfered to the new sk_buff.
 *
 *              Interface to log receive and transmit net_device specific 
 *              information is also provided.
 *
 *              Logging includes L3 IPv4 tuple information and L4 port number.
 *
 *              Mechanism to skip further logging, when non-interesting scenario
 *              such as, non-ipv4, ipv4 flow has associated helper, etc is also
 *              provided.
 *
 *              The three main interfaces are: blog_init(), blog_emit(), blog().
 *
 *              blog_init() will allocate and associate a Blog_t object with
 *              the skb, and the receive blog hook is invoked. If a NULL receive
 *              hook is configured, a Blog_t object is not associated with the
 *              sk_buff and no further logging occurs.
 *
 *              blog_emit() will invoke the transmit blog hook and subsequently
 *              dis-associate the Blog_t object from the skb, and recycle.
 *
 *              Physical Network devices may be instrumented as follows:
 *              - prior to netif_receive_skb() invoke blog_init().
 *              - in hard_start_xmit, prior to initiating a dma transfer,
 *                invoke blog_emit().
 *
 *              A receive and transmit blog hook is provided. These hooks
 *              may be initialized to do some basic processing when a packet
 *              is received or transmitted via a physical network device. By
 *              default, these hooks are NULL and hence no logging occurs.
 *              PS. blog_init() will associate a Blog_t only if the rx hook is
 *              defined.
 *              The receive hook may return an action to drop the packet, in
 *              which case no further processing of the skb may be done.
 *
 *              blog() may be inserted in various L2 protocol decoders/encoders
 *              to record the L2 header information. blog() may also be used to
 *              record the IP tuple.
 *
 *              While blog may be used as a simple tracing tool, a traffic
 *              generator to analyze the networking path may be envisioned,
 *              wherein one (or more) packets are recycled from tx to rx.
 *              A connection may be tracked by netfilter conntrack which
 *              expects packets to keep itself refreshed. If the traffic
 *              needs to hold on to the packets to insert a burst followed
 *              by very large idle periods, it should refresh the conntrack.
 *              Likewise a conntrack may be destroyed, and the traffic flow
 *              would be notified using a flow key saved in the conntrack.
 *
 * Engineering: The data structues are defined to pack each blog into 9 16byte
 *              cachelines. Layout organized for data locality.
 *
 *******************************************************************************
 */

#include <linux/autoconf.h>

#ifndef NULL_STMT
#define NULL_STMT                   do { /* NULL BODY */ } while (0)
#endif

#define BLOG_NONXTM                 0xFF
                                            /* First encapsulation type */
#define TYPE_ETH                    0x0000  /* LAN: ETH, WAN: EoA, MER, PPPoE */
#define TYPE_PPP                    0x0001  /*           WAN: PPPoA */
#define TYPE_IP                     0x0002  /*           WAN: IPoA */

#define TYPE_PPP_IP                 0x0021  /* IP in PPP */
#define TYPE_ETH_P_IP               0x0800  /* IP in Ethernet */
#define TYPE_ETH_P_8021Q            0x8100  /* VLAN in Ethernet */
#define TYPE_ETH_P_PPP_SES          0x8864  /* PPPoE in Ethernet */
#define TYPE_ETH_P_BCM              0x8874  /* BCM Switch Hdr */

#undef  BLOG_DECL
#define BLOG_DECL(x)                x,

typedef enum {
        BLOG_DECL(PKT_DONE)         /* packet consumed and freed */
        BLOG_DECL(PKT_NORM)         /* continue normal stack processing */
        BLOG_DECL(PKT_BLOG)         /* continue stack with blogging */
} BlogAction_t;

typedef enum {
        BLOG_DECL(DIR_RX)           /* Receive path */
        BLOG_DECL(DIR_TX)           /* Transmit path */
        BLOG_DECL(DIR_MAX)
} BlogDir_t;

/*
 *------------------------------------------------------------------------------
 * RFC 2684 header logging.
 * CAUTION: 0'th enum corresponds to either header was stripped or zero length
 *          header. VC_MUX_PPPOA and VC_MUX_IPOA have 0 length RFC2684 header.
 *          PTM does not have an rfc2684 header.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(RFC2684_NONE)         /*                               */
        BLOG_DECL(LLC_SNAP_ETHERNET)    /* AA AA 03 00 80 C2 00 07 00 00 */
        BLOG_DECL(LLC_SNAP_ROUTE_IP)    /* AA AA 03 00 00 00 08 00       */
        BLOG_DECL(LLC_ENCAPS_PPP)       /* FE FE 03 CF                   */
        BLOG_DECL(VC_MUX_ETHERNET)      /* 00 00                         */
        BLOG_DECL(VC_MUX_IPOA)          /*                               */
        BLOG_DECL(VC_MUX_PPPOA)         /*                               */
        BLOG_DECL(PTM)                  /*                               */
} Rfc2684_t;

#if defined(CONFIG_BLOG)

#include <linux/types.h>

/* LAB ONLY: Design development */
// #define CC_CONFIG_BLOG_COLOR
// #define CC_CONFIG_BLOG_DEBUG

#define BLOG_VERSION                "v1.0"

#define BLOG_ENCAP_MAX              6       /* Maximum number of L2 encaps */
#define BLOG_HDRSZ_MAX              32      /* Maximum size of L2 encaps */

/* Maximum 400 blogs Ucast+Mcast */
#define BLOG_EXTEND_SIZE            25      /* Number of Blog_t per extension */
#define BLOG_EXTEND_MAX             16      /* Maximum extensions allowed */

#define BLOG_NULL                   ((Blog_t*)NULL)

/* Forward declarations */
struct sk_buff;
struct net_device;
struct nf_conn;
struct net_bridge_fdb_entry;

typedef uint16_t hProto_t;

struct bcmhdr {
    uint32_t brcm_tag;
    uint16_t h_proto;
} __attribute__((packed));


extern const uint8_t rfc2684HdrLength[];
extern const uint8_t rfc2684HdrData[][16];
extern const char    * strRfc2684[];    /* for debug printing */

/*
 *------------------------------------------------------------------------------
 * Layer 2 encapsulations logged.
 * Implementation constraint: max 8 proto types.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(BCM_XTM)          /* BRCM ATM|PTM */
        BLOG_DECL(BCM_SWC)          /* BRCM Switch Header */
        BLOG_DECL(ETH_802x)         /* Ethernet */
        BLOG_DECL(VLAN_8021Q)       /* Vlan 8021Q (incld stacked) */
        BLOG_DECL(PPPoE_2516)       /* PPPoE RFC 2516 */
        BLOG_DECL(PPP_1661)         /* PPP RFC 1661 */
        BLOG_DECL(USB_CDC11)        /* USB CDC1.1 */
        BLOG_DECL(WLAN_HDR)         /* WLAN ? */
        BLOG_DECL(PROTO_MAX)
} BlogEncap_t;


typedef struct {
    uint8_t             channel;        /* e.g. port number, txchannel, ... */
    uint8_t             count;          /* Number of L2 encapsulations */

    struct {
        uint8_t             reserved    :2;
        uint8_t             qdisc       :1;        /* traffic control flag */
        uint8_t             multicast   :1;        /* multicast flag */
        uint8_t             rfc2684     :4;        /* Type of rfc2684 header */
    };

    union {
        struct {
            uint8_t         WLAN_HDR    : 1;
            uint8_t         USB_CDC11   : 1;
            uint8_t         PPP_1661    : 1;
            uint8_t         PPPoE_2516  : 1;
            uint8_t         VLAN_8021Q  : 1;    /* also 8021Qad stacked */
            uint8_t         ETH_802x    : 1;
            uint8_t         BCM_SWC     : 1;
            uint8_t         BCM_XTM     : 1;    /* ATM/PTM type  */
        }               bmap;/* as per order of BlogEncap_t enums declaration */
        uint8_t         hdrs;
    };
} __attribute__((packed)) BlogInfo_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log IP Tuple.
 * Packed: 1 16byte cacheline.
 *------------------------------------------------------------------------------
 */
struct blogTuple_t {
    uint32_t        saddr;          /* IP header saddr */
    uint32_t        daddr;          /* IP header daddr */

    union {
        struct {
            uint16_t    source;     /* L4 source port */
            uint16_t    dest;       /* L4 dest port */
        }           port;
        uint32_t    ports;
    };

    uint8_t         ttl;            /* IP header ttl */
    uint8_t         tos;            /* IP header tos */
    uint16_t        check;          /* checksum: rx tuple=l3, tx tuple=l4 */

} __attribute__((packed)) ____cacheline_aligned;
typedef struct blogTuple_t BlogTuple_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log Layer 2 and IP Tuple headers.
 * Packed: 4 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogHeader_t {

    BlogTuple_t         tuple;          /* L3+L4 IP Tuple log */

    union {
        struct net_device * dev_p;
        struct nf_conn    * ct_p;
    };

    union {
    	BlogInfo_t      info;
	    uint32_t        word;           /* channel, count, rfc2684, bmap */
    };

    union {
        uint8_t         reserved;
        uint8_t         nf_dir;
    };
    uint8_t             length;         /* L2 header total length */
    uint8_t /*BlogEncap_t*/ encap[ BLOG_ENCAP_MAX ];    /* All L2 header types */

    uint8_t             l2hdr[ BLOG_HDRSZ_MAX ];    /* Data of all L2 headers */

} __attribute__((packed)) ____cacheline_aligned;

typedef struct blogHeader_t BlogHeader_t;           /* L2 and L3+4 tuple */

/*
 *------------------------------------------------------------------------------
 * Buffer log structure.
 * Packed: 10 16 byte cachelines, 160bytes per blog.
 *------------------------------------------------------------------------------
 */
struct blog_t {

    union {
        void            * void_p;
        struct blog_t   * blog_p;       /* Free list of Blog_t */
        struct sk_buff  * skb_p;        /* Associated sk_buff */
    };

    uint8_t             hash;           /* Hash of Rx IP tuple */
    uint8_t             protocol;       /* IP protocol */
    uint16_t            nfct_events;    /* netfilter events */

    uint32_t            mark;           /* NF mark value on tx */
    uint32_t            priority;       /* Tx  priority */

    struct net_bridge_fdb_entry * fdb_src;
    struct net_bridge_fdb_entry * fdb_dst;
    uint32_t            reserved[2];

    BlogHeader_t        rx;             /* Receive path headers */
    BlogHeader_t        tx;             /* Transmit path headers */

} __attribute__((packed)) ____cacheline_aligned;
typedef struct blog_t Blog_t;


/*
 * -----------------------------------------------------------------------------
 *
 * Blog defines three hooks:
 *
 *  RX Hook: If this hook is defined then blog_init() will attach a Blog_t
 *           object to an skb and pass the the blogged skb to the bound hook.
 *  TX Hook: If this hook is defined then blog_emit() will check to see whether
 *           the skb has a Blog_t, and if so pass the blogged skb to the bound
 *           Tx hook.
 *  StopHook: When blog_stop is invoked, the bound hook is invoked.
 *           Use of Stop hook, to stop associated traffic flow when a conntrack
 *           is destroyed. 
 *
 * -----------------------------------------------------------------------------
 */
typedef BlogAction_t (* BlogHook_t)(struct sk_buff * skb_p, Blog_t * blog_p,
                                    uint16_t encap);
typedef void (* BlogStop_t)(struct net_bridge_fdb_entry * fdb_p,
                            struct nf_conn * ct_p );

extern void blog_bind(BlogHook_t rx_hook, BlogHook_t tx_hook,
                      BlogStop_t stop_hook);


/*
 * -----------------------------------------------------------------------------
 * Blog functional interface
 * -----------------------------------------------------------------------------
 */

/* Free a Blog_t */
void blog_put(Blog_t * blog_p);

/* Allocate a Blog_t and associate with sk_buff */
extern Blog_t * blog_skb(struct sk_buff * skb_p);

/* Clear association of Blog_t with sk_buff and free Blog_t object */
extern void blog_free(struct sk_buff * skb_p);

/* Dump a Blog_t object */
extern void blog_dump(Blog_t * blog_p);

/* Disable further logging. Dis-associate with skb and free Blog object */
extern void blog_skip(struct sk_buff * skb_p);

/* Clear association of Blog_t with sk_buff */
extern Blog_t * blog_null(struct sk_buff * skb_p);

/* Link a Blog_t object to an sk_buff */
extern void blog_link(struct sk_buff * skb_p, Blog_t * blog_p);

/*
 * Transfer association of a Blog_t object between two sk_buffs.
 * May be used to implement transfer of Blog_t object from one sk_buff to
 * another, e.g. to permit Blogging when sk_buffs are cloned. Currently,
 * when an sk_buff is cloned, any associated non-multicast blog-t object 
 * is cleared and freed explicitly !!!
 */
extern void blog_xfer(struct sk_buff * skb_p, const struct sk_buff * prev_p);

/*
 * Duplicate a Blog_t object for another skb.
 */
extern void blog_clone(struct sk_buff * skb_p, const struct blog_t * prev_p);

/*
 *------------------------------------------------------------------------------
 * BLOG to Netfilter Conntrack interface for flows tracked by Netfilter.
 * Associate a nf_conn with an skb's blog object.
 * Associate a traffic flow key with each direction of a blogged conntrack.
 * - Down call blog_stop() invoked when a conntrack is destroyed.
 * - Up   call blog_time() invoke to refresh a conntrack.
 *------------------------------------------------------------------------------
 */

/* Log a Netfilter Conntrack and events into a blog on nf_conntrack_in */
extern void blog_nfct(struct sk_buff * skb_p, struct nf_conn * nfct_p);
extern void blog_ctev(const struct sk_buff * skb_p, uint16_t event);

/* Log a bridge forward info into a blog at br_handle_frame_finish */
extern void blog_br_fdb(struct sk_buff * skb_p, 
                        struct net_bridge_fdb_entry* fdb_src,
                        struct net_bridge_fdb_entry* fdb_dst);
/*
 * Bind a traffic flow to blogged conntrack using a 16bit traffic flow key.
 * if the conntrack gets destroyed the associated traffic flow(s) are notified
 * via the downcall blog_stop.
 * Either Conntrack destroy or bridge delete fdb entry, flows are unbound 
 * using blog_stop()
 */
extern void blog_flow(Blog_t * blog_p, uint32_t key);
extern void blog_stop(struct net_bridge_fdb_entry * fdb_p,
                      struct nf_conn * nfct_p);

/* Refresh a blogged conntrack on behalf of associated traffic flow */
extern void blog_time(Blog_t * blog_p, uint32_t jiffies);
typedef void (*blog_refresh_t)(struct nf_conn * nfct, uint32_t ctinfo,
                               struct sk_buff * skb_p,
                               uint32_t jiffies, int do_acct);
extern blog_refresh_t blog_refresh_fn;

/* Refresh a blogged bridge forward entry on behalf of associated flow */
extern void blog_refresh_br( Blog_t * blog_p );

/*
 *------------------------------------------------------------------------------
 * If rx hook is defined, allocate and associate a Blog_t object with the
 * sk_buff and invoke the rx hook
 *------------------------------------------------------------------------------
 */
extern BlogAction_t blog_init(struct sk_buff * skb_p, struct net_device * dev_p,
                              uint32_t channel, uint8_t xtmHdr, uint16_t encap);

/*
 *------------------------------------------------------------------------------
 * If tx hook is defined, invoke tx hook, dis-associate and free Blog_t
 *------------------------------------------------------------------------------
 */
extern BlogAction_t blog_emit(struct sk_buff * skb_p, struct net_device * dev_p,
                              uint32_t channel, uint8_t xtmHdr, uint16_t encap);

/* Logging of L2|L3 headers */
extern void blog(struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,  
                 size_t len, void * data_p);

#define BLOG(skb_p, dir, encap, len, hdr_p)                         \
        do {                                                        \
            if ( skb_p->blog_p )                                    \
                blog( skb_p, dir, encap, len, hdr_p );              \
        } while(0)

/* Debug display of enums */
extern const char * strBlogAction[];
extern const char * strBlogEncap[];

#else   /* else ! defined(CONFIG_BLOG) */

#define blog_put(blog)                          NULL_STMT
#define blog_skb(skb)                           NULL
#define blog_free(skb)                          NULL_STMT
#define blog_dump(blog)                         NULL_STMT
#define blog_skip(skb)                          NULL_STMT

#define blog_null(skb)                          NULL
#define blog_link(skb, blog)                    NULL_STMT
#define blog_xfer(skb, prev)                    NULL_STMT
#define blog_clone(skb, prev)                   NULL_STMT

#define blog_nfct(skb, nfct)                    NULL_STMT
#define blog_ctev(skb, event)                   NULL_STMT
#define blog_refresh_br(blog)                   NULL_STMT
#define blog_br_fdb(skb, fdb_src, fdb_dst)      NULL_STMT
#define blog_flow(blog, key)                    NULL_STMT
#define blog_stop(fdb,nfct)                     NULL_STMT
#define blog_time(blog, time)                   NULL_STMT

#define blog_hash(blog)                         NULL_STMT

#define blog_init(skb, dev, ch, xtm, encap)     PKT_NORM
#define blog_emit(skb, dev, ch, xtm, encap)     NULL_STMT

#define blog(skb, dir, encap, len, hdr)         NULL_STMT
#define BLOG(skb, dir, encap, len, hdr)         NULL_STMT

#define blog_bind(rx_hook, tx_hook, stop_hook)  NULL_STMT
#endif  /* ! defined(CONFIG_BLOG) */

#endif /* defined(__BLOG_H_INCLUDED__) */

