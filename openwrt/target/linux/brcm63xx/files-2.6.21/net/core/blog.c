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
 * File Name  : blog.c
 * Description: Implements the tracing of L2 and L3 modifications to a packet
 * 		buffer while it traverses the Linux networking stack.
 *******************************************************************************
 */

#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#include <linux/blog.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include "../bridge/br_private.h"

#ifdef CC_CONFIG_BLOG_COLOR
#define COLOR(clr_code)     clr_code
#else
#define COLOR(clr_code)
#endif
#define CLRb                COLOR("\e[0;34m")       /* blue */
#define CLRc                COLOR("\e[0;36m")       /* cyan */
#define CLRn                COLOR("\e[0m")          /* normal */
#define CLRerr              COLOR("\e[0;33;41m")    /* yellow on red */
#define CLRN                CLRn"\n"                /* normal newline */

/* Debug macros */
#if defined(CC_CONFIG_BLOG_DEBUG)
#define blog_print(fmt, arg...)                                         \
    printk( CLRc "BLOG %s :" fmt CLRN, __FUNCTION__, ##arg )
#define blog_assertv(cond)                                              \
    if ( !cond ) {                                                      \
        printk( CLRerr "BLOG ASSERT %s : " #cond CLRN, __FUNCTION__ );  \
        return;                                                         \
    }
#define blog_assertr(cond, rtn)                                         \
    if ( !cond ) {                                                      \
        printk( CLRerr "BLOG ASSERT %s : " #cond CLRN, __FUNCTION__ );  \
        return rtn;                                                     \
    }
#define BLOG_DBG(debug_code)    do { debug_code } while(0)
#else
#define blog_print(fmt, arg...) NULL_STMT
#define blog_assertv(cond)      NULL_STMT
#define blog_assertr(cond, rtn) NULL_STMT
#define BLOG_DBG(debug_code)    NULL_STMT
#endif

#define blog_error(fmt, arg...)                                         \
    printk( CLRerr "BLOG ERROR %s :" fmt CLRN, __FUNCTION__, ##arg)

#undef  BLOG_DECL
#define BLOG_DECL(x)        #x,         /* string declaration */

/*--- globals ---*/

/*
 * Traffic flow generator, keep conntrack alive during idle traffic periods
 * by refreshing the conntrack. Dummy sk_buff passed to nf_conn.
 * Netfilter may not be statically loaded.
 */
blog_refresh_t blog_refresh_fn;
struct sk_buff * nfskb_p = (struct sk_buff*) NULL;

/*----- Constant string representation of enums for print -----*/
const char * strBlogAction[] =
{
    BLOG_DECL(PKT_DONE)
    BLOG_DECL(PKT_NORM)
    BLOG_DECL(PKT_BLOG)
};

const char * strBlogEncap[] =
{
    BLOG_DECL(BCM_XTM)
    BLOG_DECL(BCM_SWC)
    BLOG_DECL(ETH_802x)
    BLOG_DECL(VLAN_8021Q)
    BLOG_DECL(PPPoE_2516)
    BLOG_DECL(PPP_1661)
    BLOG_DECL(USB_CDC11)
    BLOG_DECL(WLAN_TBD)
    BLOG_DECL(PROTO_MAX)
};

const char * strIpctDir[] = {   /* in reference to enum ip_conntrack_dir */
    BLOG_DECL(DIR_ORIG)
    BLOG_DECL(DIR_RPLY)
    BLOG_DECL(DIR_UNKN)
};

const char * strIpctStatus[] =  /* in reference to enum ip_conntrack_status */
{
    BLOG_DECL(EXPECTED)
    BLOG_DECL(SEEN_REPLY)
    BLOG_DECL(ASSURED)
    BLOG_DECL(CONFIRMED)
    BLOG_DECL(SRC_NAT)
    BLOG_DECL(DST_NAT)
    BLOG_DECL(SEQ_ADJUST)
    BLOG_DECL(SRC_NAT_DONE)
    BLOG_DECL(DST_NAT_DONE)
    BLOG_DECL(DYING)
    BLOG_DECL(FIXED_TIMEOUT)
    BLOG_DECL(BLOG)
};

const char * strIpctEvents[] =  /* in reference to enum ip_conntrack_events */
{ 
    BLOG_DECL(NEW)
    BLOG_DECL(RELATED)
    BLOG_DECL(DESTROY)
    BLOG_DECL(REFRESH)
    BLOG_DECL(STATUS)
    BLOG_DECL(PROTOINFO)
    BLOG_DECL(PROTOINFO_VOLATILE)
    BLOG_DECL(HELPER)
    BLOG_DECL(HELPINFO)
    BLOG_DECL(HELPINFO_VOLATILE)
    BLOG_DECL(NATINFO)
    BLOG_DECL(COUNTER_FILLING)
    BLOG_DECL(MARK)
    BLOG_DECL(BLOG)
    BLOG_DECL(UNKNOWN)
};

#ifdef CC_CONFIG_BLOG_DEBUG
/* Convert an event value to its corresponding event name */
const char * ct_evtname(uint32_t evt_value)
{
    switch( evt_value )
    {
        case IPCT_NEW:      return strIpctEvents[IPCT_NEW_BIT];
        case IPCT_RELATED:  return strIpctEvents[IPCT_RELATED_BIT];
        case IPCT_DESTROY:  return strIpctEvents[IPCT_DESTROY_BIT];
        case IPCT_REFRESH:  return strIpctEvents[IPCT_REFRESH_BIT];
        case IPCT_STATUS:   return strIpctEvents[IPCT_STATUS_BIT];
        case IPCT_PROTOINFO:return strIpctEvents[IPCT_PROTOINFO_BIT];
        case IPCT_PROTOINFO_VOLATILE:
                            return strIpctEvents[IPCT_PROTOINFO_VOLATILE_BIT];
        case IPCT_HELPER:   return strIpctEvents[IPCT_HELPER_BIT];
        case IPCT_HELPINFO: return strIpctEvents[IPCT_HELPINFO_BIT];
        case IPCT_HELPINFO_VOLATILE:
                            return strIpctEvents[IPCT_HELPINFO_VOLATILE_BIT];
        case IPCT_NATINFO:  return strIpctEvents[IPCT_NATINFO_BIT];
        case IPCT_COUNTER_FILLING:
                            return strIpctEvents[IPCT_COUNTER_FILLING_BIT];
        case IPCT_MARK:     return strIpctEvents[IPCT_MARK_BIT];
        case IPCT_BLOG:     return strIpctEvents[IPCT_BLOG_BIT];
        default:            return strIpctEvents[IPCT_BLOG_BIT + 1];
    }
}
#else
#define ct_evtname(evt)       "ct_event"
#endif

/*
 *------------------------------------------------------------------------------
 * Support for RFC 2684 headers logging.
 *------------------------------------------------------------------------------
 */
const char * strRfc2684[] =
{
        BLOG_DECL(RFC2684_NONE)         /*                               */
        BLOG_DECL(LLC_SNAP_ETHERNET)    /* AA AA 03 00 80 C2 00 07 00 00 */
        BLOG_DECL(LLC_SNAP_ROUTE_IP)    /* AA AA 03 00 00 00 08 00       */
        BLOG_DECL(LLC_ENCAPS_PPP)       /* FE FE 03 CF                   */
        BLOG_DECL(VC_MUX_ETHERNET)      /* 00 00                         */
        BLOG_DECL(VC_MUX_IPOA)          /*                               */
        BLOG_DECL(VC_MUX_PPPOA)         /*                               */
        BLOG_DECL(PTM)                  /*                               */
};

const uint8_t rfc2684HdrLength[] =
{
     0, /* header was already stripped. :                               */
    10, /* LLC_SNAP_ETHERNET            : AA AA 03 00 80 C2 00 07 00 00 */
     8, /* LLC_SNAP_ROUTE_IP            : AA AA 03 00 00 00 08 00       */
     4, /* LLC_ENCAPS_PPP               : FE FE 03 CF                   */
     2, /* VC_MUX_ETHERNET              : 00 00                         */
     0, /* VC_MUX_IPOA                  :                               */
     0, /* VC_MUX_PPPOA                 :                               */
     0, /* PTM                          :                               */
};

const uint8_t rfc2684HdrData[][16] =
{
    {}, /* 0'th index implies no header or VC_MUX_IPOA, VC_MUX_PPPOA */
    { 0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00 },
    { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00 },
    { 0xFE, 0xFE, 0x03, 0xCF },
    { 0x00, 0x00 },
    {},
    {},
    {}
};

/*
 *------------------------------------------------------------------------------
 * Default Rx and Tx hooks.
 *------------------------------------------------------------------------------
 */
static BlogHook_t blog_rx_hook_g = (BlogHook_t)NULL;
static BlogHook_t blog_tx_hook_g = (BlogHook_t)NULL;
static BlogStop_t blog_xx_hook_g = (BlogStop_t)NULL;

/*
 *------------------------------------------------------------------------------
 * Network Utilities  : Network Order IP Address access (in Big Endian) format
 *------------------------------------------------------------------------------
 */
#define _IP_(ip)                ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], \
                                ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

/*
 *------------------------------------------------------------------------------
 * Function     : _read32_align16
 * Description  : Read a 32bit value from a 16 byte aligned data stream
 *------------------------------------------------------------------------------
 */
static inline uint32_t _read32_align16( uint16_t * from )
{
    return (__force uint32_t)( (from[0] << 16) | (from[1]) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _write32_align16
 * Description  : Write a 32bit value to a 16bit aligned data stream
 *------------------------------------------------------------------------------
 */
static inline void _write32_align16( uint16_t * to, uint32_t from )
{
    to[0] = (__force uint16_t)(from >> 16);
    to[1] = (__force uint16_t)(from >>  0);
}


/*
 *------------------------------------------------------------------------------
 * Blog_t Free Pool Management.
 * The free pool of Blog_t is self growing (extends upto an engineered
 * value). Could have used a kernel slab cache. 
 *------------------------------------------------------------------------------
 */

/* Global pointer to the free pool of Blog_t */
static Blog_t * blog_list_gp = BLOG_NULL;

static int blog_extends = 0;        /* Extension of Pool on depletion */
#if defined(CC_CONFIG_BLOG_DEBUG)
static int blog_cnt_free = 0;       /* Number of Blog_t free */
static int blog_cnt_used = 0;       /* Number of in use Blog_t */
static int blog_cnt_hwm  = 0;       /* In use high water mark for engineering */
static int blog_cnt_fails = 0;
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : blog_construct
 * Description: Create a pool of Blog_t objects. When a pool is exhausted
 *              this function may be invoked to extend the pool. The pool is
 *              identified by a global pointer, blog_list_gp. All objects in
 *              the pool chained together in a single linked list.
 * Parameters :
 *   num      : Number of Blog_t objects to be allocated.
 * Returns    : Number of Blog_t objects allocated in pool.
 *------------------------------------------------------------------------------
 */
static uint32_t blog_construct( uint32_t num )
{
    register int i;
    register Blog_t * list_p;

    blog_print( "blog_construct %u", num );

    list_p = (Blog_t *) kmalloc( num * sizeof(Blog_t), GFP_ATOMIC);
    if ( list_p == BLOG_NULL )
    {
#if defined(CC_CONFIG_BLOG_DEBUG)
        blog_cnt_fails++;
#endif
        blog_print( "WARNING: Failure to initialize %d Blog_t", num );
        return 0;
    }
    blog_extends++;

    /* memset( (void *)list_p, 0, (sizeof(Blog_t) * num ); */
    for ( i = 0; i < num; i++ )
        list_p[i].blog_p = &list_p[i+1];

    local_bh_disable();
    list_p[num-1].blog_p = blog_list_gp; /* chain last Blog_t object */
    blog_list_gp = list_p;  /* Head of list */
    local_bh_enable();

    BLOG_DBG( blog_cnt_free += num; );

    return num;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clr
 * Description  : Clear the data of a Blog_t
 *------------------------------------------------------------------------------
 */
static inline void blog_clr( Blog_t * blog_p )
{
    BLOG_DBG( memset( (void*)blog_p, 0, sizeof(Blog_t) ); );

    /* clear rfc2684, count, bmap, and channel */
    blog_p->rx.word = 0;
    blog_p->tx.word = 0;

    blog_p->nfct_events = 0;
    blog_p->rx.ct_p = (struct nf_conn*)NULL;
    blog_p->fdb_src = (struct net_bridge_fdb_entry*)NULL;
    blog_p->fdb_dst = (struct net_bridge_fdb_entry*)NULL;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_get
 * Description  : Allocate a Blog_t from the free list
 * Returns      : Pointer to an Blog_t or NULL, on depletion.
 *------------------------------------------------------------------------------
 */
static Blog_t * blog_get( void )
{
    register Blog_t * blog_p;

    if ( blog_list_gp == BLOG_NULL )
    {
#ifdef SUPPORT_BLOG_EXTEND
        if ( (blog_extends >= BLOG_EXTEND_MAX)  /* Try extending free pool */
          || (blog_construct( BLOG_EXTEND_SIZE ) != BLOG_EXTEND_SIZE) )
        {
            blog_print( "WARNING: free list exhausted" );
            return BLOG_NULL;
        }
#else
        if ( blog_construct( BLOG_EXTEND_SIZE ) == 0 )
        {
            blog_print( "WARNING: out of memory" );
            return BLOG_NULL;
        }
#endif
    }
    BLOG_DBG(
        blog_cnt_free--;
        if ( ++blog_cnt_used > blog_cnt_hwm )
            blog_cnt_hwm = blog_cnt_used;
        );

    local_bh_disable();
    blog_p = blog_list_gp;
    blog_list_gp = blog_list_gp->blog_p;
    local_bh_enable();

    blog_clr( blog_p );     /* quickly clear the contents */

    return blog_p;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_put
 * Description  : Release a Blog_t back into the free pool
 * Parameters   :
 *  blog_p      : Pointer to a non-null Blog_t to be freed.
 *------------------------------------------------------------------------------
 */
void blog_put( Blog_t * blog_p )
{
    blog_assertv( (blog_p != BLOG_NULL) );
    BLOG_DBG( blog_cnt_used--; blog_cnt_free++; );
    blog_clr( blog_p );
    local_bh_disable();
    blog_p->blog_p = blog_list_gp;
    blog_list_gp = blog_p;  /* link into free pool */
    local_bh_enable();
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_skb
 * Description  : Allocate and associate a Blog_t with an skb.
 * Parameters   :
 *  skb_p       : pointer to a non-null sk_buff
 * Returns      : A Blog_t object or NULL,
 *------------------------------------------------------------------------------
 */
Blog_t * blog_skb( struct sk_buff * skb_p )
{
    blog_assertr( (skb_p != (struct sk_buff *)NULL), BLOG_NULL );
    blog_assertr( (skb_p->blog_p == BLOG_NULL), BLOG_NULL );
    skb_p->blog_p = blog_get(); /* Allocate and associate with skb */
    return skb_p->blog_p;       /* May be null */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_null
 * Description  : Dis-associate a sk_buff with any Blog_t
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 * Returns      : Previous Blog_t associated with sk_buff
 *------------------------------------------------------------------------------
 */
inline Blog_t * _blog_null( struct sk_buff * skb_p )
{
    register Blog_t * blog_p;
    blog_p = skb_p->blog_p;
    skb_p->blog_p = BLOG_NULL;
    return blog_p;
}

Blog_t * blog_null( struct sk_buff * skb_p )
{
    blog_assertr( (skb_p != (struct sk_buff *)NULL), BLOG_NULL );
    return _blog_null( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_free
 * Description  : Free any Blog_t associated with a sk_buff
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 *------------------------------------------------------------------------------
 */
inline void _blog_free( struct sk_buff * skb_p )
{
    register Blog_t * blog_p;
    blog_p = _blog_null( skb_p );   /* Dis-associate Blog_t from skb_p */
    if ( likely(blog_p != BLOG_NULL) )
        blog_put( blog_p );         /* Recycle blog_p into free list */
}

void blog_free( struct sk_buff * skb_p )
{
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );
    _blog_free( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_skip
 * Description  : Disable further tracing of sk_buff by freeing associated
 *                Blog_t (if any)
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *------------------------------------------------------------------------------
 */
void blog_skip( struct sk_buff * skb_p )
{
    blog_print( "[<0x%08x>]", (int)__builtin_return_address(0) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );
    _blog_free( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_link
 * Description  : Associate a given blog object with an skb
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  blog_p      : Pointer to a blog_p
 *------------------------------------------------------------------------------
 */
void blog_link( struct sk_buff * skb_p, Blog_t * blog_p )
{
    blog_assertv( (blog_p != BLOG_NULL) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );
    blog_assertv( (skb_p->blog_p == BLOG_NULL) );  /* Avoid leak */
    skb_p->blog_p = blog_p;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_xfer
 * Description  : Transfer ownership of a Blog_t between two sk_buff(s)
 * Parameters   :
 *  skb_p       : New owner of Blog_t object 
 *  prev_p      : Former owner of Blog_t object
 *------------------------------------------------------------------------------
 */
void blog_xfer( struct sk_buff * skb_p, const struct sk_buff * prev_p )
{
    Blog_t * blog_p;
    struct sk_buff * mod_prev_p;
    blog_assertv( (prev_p != (struct sk_buff *)NULL) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    mod_prev_p = (struct sk_buff *) prev_p; /* const removal without warning */
    blog_p = _blog_null( mod_prev_p );
    skb_p->blog_p = blog_p;

    if ( likely(blog_p != BLOG_NULL) )
    {
        blog_print( "skb<0x%08x> to new<0x%08x> blog<0x%08x>",
                    (int)prev_p, (int)skb_p, (int)blog_p );
        blog_p->skb_p = skb_p;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clone
 * Description  : Duplicate a Blog_t for another sk_buff
 * Parameters   :
 *  skb_p       : New owner of Blog_t object 
 *  prev_p      : Blog_t object to be cloned
 *------------------------------------------------------------------------------
 */
void blog_clone( struct sk_buff * skb_p, const struct blog_t * prev_p )
{
    blog_assertv( (prev_p != (struct blog_t *)NULL) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ( likely(prev_p != BLOG_NULL) )
    {
        Blog_t * blog_p;
        
        skb_p->blog_p = blog_get(); /* Allocate and associate with skb */
        blog_p = skb_p->blog_p;

        blog_print( "orig blog<0x%08x> duplicate to" 
                    " new skb<0x%08x> with new blog<0x%08x>",
                    (int)prev_p, (int)skb_p, (int)blog_p );

        if ( likely(blog_p != BLOG_NULL) )
        {
            blog_p->skb_p = skb_p;
#define CPY(x) blog_p->x = prev_p->x
            CPY(hash);
            CPY(protocol);
            CPY(mark);
            CPY(priority);
            CPY(rx);
            blog_p->tx.word = 0;
        }
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_nfct [Down call from Netfilter]
 * Description  : Associate a nf_conn with an skb's blog object
 *                See: resolve_normal_ct() nf_conntrack_core.c
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  ct_p        : Pointer to a nf_conn
 *------------------------------------------------------------------------------
 */
void blog_nfct( struct sk_buff * skb_p, struct nf_conn * ct_p )
{
    enum ip_conntrack_info ctinfo;
    struct nf_conn * ct;
    
    blog_assertv( (ct_p != (struct nf_conn *)NULL) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ( unlikely(skb_p->blog_p == BLOG_NULL) )
        return;

    if ( unlikely(skb_p->blog_p->rx.info.multicast) )
        return;

    blog_print( "skb<0x%08x> blog<0x%08x> nfct<0x%08x>",
                (int)skb_p, (int)skb_p->blog_p, (int)ct_p );

    skb_p->blog_p->rx.ct_p = ct_p;                  /* Pointer to conntrack */
    skb_p->blog_p->nfct_events = IPCT_BLOG;         /* Blogged conntrack */

    ct = nf_ct_get(skb_p, &ctinfo);
    blog_assertv( (ct == ct_p) );
    skb_p->blog_p->rx.nf_dir = CTINFO2DIR(ctinfo);  /* Conntrack direction */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_ctev [Down call from Netfilter]
 * Description  : Set a netfilter conntrack event in the blog
 *                See nf_conntrack_event_cache() in nf_conntrack_ecache.h
 *                Function nf_conntrack_event does not invoked blog_ctev,
 *                used to notify conntrack destroys and unhelp.
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  nfct_event  : Netfilter conntrack event
 *------------------------------------------------------------------------------
 */
void blog_ctev( const struct sk_buff * skb_p, uint16_t nfct_event )
{
    blog_assertv( (skb_p != (const struct sk_buff *)NULL) );
    blog_print( "skb<0x%08x> blog<0x%08x> event<%s>",
                (int)skb_p, (int)skb_p->blog_p, ct_evtname(nfct_event) );
    if ( unlikely(skb_p->blog_p == BLOG_NULL) )
        return;

    if ( unlikely(skb_p->blog_p->rx.info.multicast) )
        return;

    blog_assertv( (skb_p->blog_p->rx.ct_p != (struct nf_conn*)NULL) );

    ((struct sk_buff *)skb_p)->blog_p->nfct_events |= (uint16_t)nfct_event;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_flow [Up call to Netfilter]
 * Description  : Associates a traffic flow key with the blogged nf_conntack
 * Parameters   :
 *  blog_p      : Pointer to a Blog_t
 *  key         : Key of the traffic flow
 *------------------------------------------------------------------------------
 */
void blog_flow( Blog_t * blog_p, uint32_t key )
{
    blog_assertv( (blog_p != BLOG_NULL) );
    blog_assertv( (blog_p->rx.ct_p != (struct nf_conn*)NULL) );

    blog_p->rx.ct_p->blog_key[ blog_p->rx.nf_dir ] = key;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_br_fdb [Down call from bridge]
 * Description  : Associate a net_bridge_fdb_entry with an skb's blog object
 *                See: br_handle_frame_finish() br_input.c
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  fdb_src     : Pointer to a net_bridge_fdb_entry of packet source
 *  fdb_dst     : Pointer to a net_bridge_fdb_entry of packet destination
 *------------------------------------------------------------------------------
 */
void blog_br_fdb( struct sk_buff * skb_p, struct net_bridge_fdb_entry * fdb_src,
                  struct net_bridge_fdb_entry * fdb_dst )
{
    blog_assertv( ((fdb_src != (struct net_bridge_fdb_entry *)NULL) || 
                   (fdb_dst != (struct net_bridge_fdb_entry *)NULL)) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ( unlikely(skb_p->blog_p == BLOG_NULL) )
        return;

    blog_print( "skb<0x%08x> blog<0x%08x> fdb_src<0x%08x> fdb_dst<0x%08x>",
                (int)skb_p, (int)skb_p->blog_p, (int)fdb_src, (int)fdb_dst );

    skb_p->blog_p->fdb_src = fdb_src;      /* Pointer to net_bridge_fdb_entry */
    skb_p->blog_p->fdb_dst = fdb_dst;      /* Pointer to net_bridge_fdb_entry */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_stop [Down call from Netfilter/bridge]
 * Description  : Invokes the bound stop function passing either 
 *                      the flow key(netfilter) or net_bridge_fdb_entry(bridge)
 *                See destroy_conntrack() in nf_conntrack_core.c
 *                       fdb_delete() in br_fdb.c
 * Parameters   :
 *  ct_p        : Pointer to a conntrack that is being destroyed.
 *  fdb_p       : Pointer to a bridge forward database that is being destroyed.
 *------------------------------------------------------------------------------
 */
void blog_stop( struct net_bridge_fdb_entry * fdb_p, struct nf_conn * ct_p )
{
    if ( likely(blog_xx_hook_g != (BlogStop_t)NULL) )
    {
        blog_print("blog_stop: fdb_p<0x%08x> ct_p<0x%08x>", (int)fdb_p, (int)ct_p );

        local_bh_disable();
        blog_xx_hook_g( fdb_p, ct_p );
        local_bh_enable();
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_time [Up call to Netfilter]
 * Description  : Refresh the nf_conn associated with this blog.
 *   blog_p     : Pointer to a blog
 *   jiffies    : Refresh period
 *------------------------------------------------------------------------------
 */

void blog_time( Blog_t * blog_p, uint32_t jiffies )
{
    if ( blog_refresh_fn  && (blog_p->rx.ct_p != (struct nf_conn *)NULL) )
    {
        nfskb_p->nfct = (struct nf_conntrack *)blog_p->rx.ct_p;
        (*blog_refresh_fn)( blog_p->rx.ct_p, 0, nfskb_p, jiffies, 0 );
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_refresh_br [Up call to bridge]
 * Description  : Refresh the bridge forward entry associated with this blog.
 *   blog_p     : Pointer to a blog
 *------------------------------------------------------------------------------
 */
extern void br_fdb_refresh( struct net_bridge_fdb_entry *fdb ); /* br_fdb.c */
void blog_refresh_br( Blog_t * blog_p )
{
    if ( blog_p->fdb_src != (struct net_bridge_fdb_entry *)NULL )
        br_fdb_refresh( blog_p->fdb_src );

}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_init
 * Description  : This function may be inserted in a physical network device's
 *                packet receive handler. A receive handler typically extracts
 *                the packet data from the rx DMA buffer ring, allocates and
 *                sets up a sk_buff, decodes the l2 headers and passes the
 *                sk_buff into the network stack via netif_receive_skb/netif_rx.
 *                Prior to decoding L2 headers, blog_init() may be invoked to
 *                associate a Blog_t object with the sk_buff.
 *
 *                This function invokes the receive blog hook.
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  dev_p       : Pointer to the net_device on which the packet arrived.
 *  channel     : Channel/Port number on which the packet arrived.
 *  xtmHdrType  : XTM device, and RFC2684 header type
 *  encap       : First encapsulation type
 *
 * Returns      :
 *  PKT_DONE    : The skb_p is consumed and device should not process skb_p.
 *  PKT_NORM    : Device may invoke netif_receive_skb for normal processing.
 *  PKT_BLOG    : PKT_NORM behaviour + Blogging enabled.
 *
 *------------------------------------------------------------------------------
 */
BlogAction_t blog_init( struct sk_buff * skb_p, struct net_device * dev_p,
                        uint32_t channel, uint8_t xtmHdrType, uint16_t encap )
{
    Blog_t * blog_p;
    BlogAction_t action;
    blog_assertr( (skb_p != (struct sk_buff *)NULL), PKT_NORM );

    if ( unlikely(blog_rx_hook_g == (BlogHook_t)NULL) )
        goto pkt_norm;

    blog_p = blog_skb( skb_p );                     /* Associate w/ skb */
    if ( unlikely(blog_p == BLOG_NULL) )
    {
        blog_print( "WARNING: Depleted blog pool reengineer/check for leaks" );
        goto pkt_norm;
    }

    blog_p->skb_p = skb_p;

    /* Explicitly cleared via blog_skb() -> blog_get() -> blog_clr() */
    blog_assertr( (blog_p->nfct_events == 0), PKT_NORM );
    blog_assertr( (blog_p->rx.ct_p == (struct nf_conn *)NULL), PKT_NORM );

    blog_p->rx.info.channel = (uint8_t)channel;
    if ( xtmHdrType != BLOG_NONXTM )                /* XTM device interface */
    {
        blog_p->rx.info.bmap.BCM_XTM = 1;
        blog_p->rx.info.rfc2684 = xtmHdrType;       /* Rfc2684_t */
    }
    blog_p->rx.length = 0;

    blog_print( "skb_p<0x%08x> blog_p<0x%08x> chnl<%u> %s RFC<%u>",
            (int)skb_p, (int)blog_p, channel, 
            (xtmHdrType == BLOG_NONXTM) ? "NonXTM" : "XTM",
            (xtmHdrType != BLOG_NONXTM) ?
            rfc2684HdrLength[blog_p->rx.info.rfc2684] : 0 );

        /*--- Invoke Rx Hook ---*/
    action = blog_rx_hook_g( skb_p, blog_p, encap );

    if ( action == PKT_NORM )
        blog_free( skb_p );                         /* Dis-associate w/ skb */

    return action;

pkt_norm:
    return PKT_NORM;                    /* continue normal stack processing */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_emit
 * Description  : This function may be inserted in a physical network device's
 *                hard_start_xmit function just before the packet data is
 *                extracted from the sk_buff and enqueued for DMA transfer.
 *
 *                This function invokes the transmit blog hook.
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *  dev_p       : Pointer to the net_device on which the packet is transmited.
 *  channel     : Channel/Port number on which the packet is transmited.
 *  xtmHdrType  : XTM device, and RFC2684 header type
 *  encap       : First encapsulation type
 *
 * Returns      :
 *  PKT_DONE    : The skb_p is consumed and device should not process skb_p.
 *  PKT_NORM    : Device may use skb_p and proceed with hard xmit 
 *                Blog object is disassociated and freed.
 *------------------------------------------------------------------------------
 */
BlogAction_t blog_emit( struct sk_buff * skb_p, struct net_device * dev_p,
                        uint32_t channel, uint8_t xtmHdrType, uint16_t encap )
{
    BlogAction_t action = PKT_NORM;
    blog_assertr( (skb_p != (struct sk_buff *)NULL), PKT_NORM );

    if ( skb_p->blog_p == BLOG_NULL )
    {
        goto pkt_norm;
    }

    if ( likely(blog_tx_hook_g != (BlogHook_t)NULL) )
    {
        register Blog_t * blog_p = skb_p->blog_p;
        blog_p->tx.dev_p = dev_p;                   /* Log device info */

        blog_p->tx.info.channel = (uint8_t)channel;
        if ( xtmHdrType != BLOG_NONXTM )
        {
            blog_p->tx.info.bmap.BCM_XTM = 1;
            blog_p->tx.info.rfc2684 = xtmHdrType;
        }
        blog_p->tx.length = 0;

        blog_p->priority = skb_p->priority;         /* Log skb info */
        blog_p->mark = skb_p->mark;

        blog_print( "skb_p<0x%08x> blog_p<0x%08x> chnl<%u> %s %u",
            (int)skb_p, (int)blog_p, channel,
            (xtmHdrType == BLOG_NONXTM) ? "NonXTM" : "XTM",
            (xtmHdrType != BLOG_NONXTM) ?
            rfc2684HdrLength[blog_p->rx.info.rfc2684] : 0 );

            /*--- Invoke Tx Hook ---*/
        action = blog_tx_hook_g( skb_p, blog_p, encap );
    }

    blog_free( skb_p );                             /* Dis-associate w/ skb */

pkt_norm:
    return action;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog
 * Description  : Log the L2 or L3+4 tuple information
 * Parameters   :
 *  skb_p       : Pointer to the sk_buff
 *  dir         : rx or tx path
 *  encap       : Encapsulation type
 *  len         : Length of header
 *  data_p      : Pointer to encapsulation header data.
 *------------------------------------------------------------------------------
 */
void blog( struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,
           size_t len, void * data_p )
{
    BlogHeader_t * bHdr_p;
    Blog_t * blog_p;

    blog_assertv( (skb_p != (struct sk_buff *)NULL ) );
    blog_assertv( (skb_p->blog_p != BLOG_NULL) );
    blog_assertv( (data_p != (void *)NULL ) );
    blog_assertv( (len <= BLOG_HDRSZ_MAX) );
    blog_assertv( (encap < PROTO_MAX ) );

    blog_p = skb_p->blog_p;
    blog_assertv( (blog_p->skb_p == skb_p) );

    bHdr_p = &blog_p->rx + dir;

    if ( encap == 10 )    /* Log the IP Tuple */
    {
        BlogTuple_t * bTuple_p = &bHdr_p->tuple;
        struct iphdr * ip_p    = (struct iphdr *)data_p;

        /* Discontinue if non IPv4 or with IP options, or fragmented */
        if ( (ip_p->version != 4) || (ip_p->ihl != 5)
             || ( ip_p->frag_off & htons(IP_OFFSET | IP_MF)) )
            goto skip;

        if ( ip_p->protocol == IPPROTO_TCP )
        {
            struct tcphdr * th_p;
            th_p = (struct tcphdr *)( (uint8_t *)ip_p + sizeof(struct iphdr) );
            if ( th_p->rst | th_p->fin )    /* Discontinue if TCP RST/FIN */
                goto skip;
            bTuple_p->port.source = th_p->source;
            bTuple_p->port.dest = th_p->dest;
        }
        else if ( ip_p->protocol == IPPROTO_UDP )
        {
            struct udphdr * uh_p;
            uh_p = (struct udphdr *)( (uint8_t *)ip_p + sizeof(struct iphdr) );
            bTuple_p->port.source = uh_p->source;
            bTuple_p->port.dest = uh_p->dest;
        }
        else
            goto skip;  /* Discontinue if non TCP or UDP upper layer protocol */

        bTuple_p->ttl = ip_p->ttl;
        bTuple_p->tos = ip_p->tos;
        bTuple_p->check = ip_p->check;
        bTuple_p->saddr = _read32_align16( (uint16_t *)&ip_p->saddr );
        bTuple_p->daddr = _read32_align16( (uint16_t *)&ip_p->daddr );
        blog_p->protocol = ip_p->protocol;
    }
    else    /* L2 encapsulation */
    {
        register short int * d;
        register const short int * s;

        blog_assertv( (bHdr_p->info.count < BLOG_ENCAP_MAX) );
        blog_assertv( ((len<=20) && ((len & 0x1)==0)) );
        blog_assertv( ((bHdr_p->length + len) < BLOG_HDRSZ_MAX) );

        bHdr_p->info.hdrs |= (1U << encap);
        bHdr_p->encap[ bHdr_p->info.count++ ] = encap;
        s = (const short int *)data_p;
        d = (short int *)&(bHdr_p->l2hdr[bHdr_p->length]);
        bHdr_p->length += len;

        switch ( len ) /* common lengths, using half word alignment copy */
        {
            case 20: *(d+9)=*(s+9);
                     *(d+8)=*(s+8);
                     *(d+7)=*(s+7);
            case 14: *(d+6)=*(s+6);
            case 12: *(d+5)=*(s+5);
            case 10: *(d+4)=*(s+4);
            case  8: *(d+3)=*(s+3);
            case  6: *(d+2)=*(s+2);
            case  4: *(d+1)=*(s+1);
            case  2: *(d+0)=*(s+0);
                 break;
            default:
                 goto skip;
        }
    }

    return;

skip:   /* Discontinue further logging by dis-associating Blog_t object */

    blog_skip( skb_p );

    /* DO NOT ACCESS blog_p !!! */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_nfct_dump
 * Description  : Dump the nf_conn context
 *  dev_p       : Pointer to a net_device object
 * CAUTION      : nf_conn is not held !!!
 *------------------------------------------------------------------------------
 */
void blog_nfct_dump( struct sk_buff * skb_p, struct nf_conn * ct,
                     uint16_t events, uint32_t dir )
{
    struct nf_conn_help *help_p;
    struct nf_conn_nat  *nat_p;
    int bitix;
    if ( ct == NULL )
    {
        blog_error( "NULL NFCT error" );
        return;
    }

#ifdef CONFIG_NF_NAT_NEEDED
    nat_p = nfct_nat(ct);
#else
    nat_p = (struct nf_conn_nat*)NULL;
#endif

    help_p = nfct_help(ct);

    printk( "\tNFCT: ct<%u|0x%08x> info<%x> master<0x%08x> mark<0x%08x>\n"
            "\t\tfeatures<%d> F_NAT<%d> nat<0x%08x> keys[%u %u] dir<%s>\n"
            "\t\tF_HELP<%d> help<0x%08x> helper<%s>\n",
            (int)ct->id, (int)ct, (int)skb_p->nfctinfo, (int)ct->master,
#if defined(CONFIG_NF_CONNTRACK_MARK)
            ct->mark,
#else
            -1,
#endif
            (int)ct->features,
            (ct->features & NF_CT_F_NAT)?1:0, (int)nat_p,
            ct->blog_key[IP_CT_DIR_ORIGINAL], ct->blog_key[IP_CT_DIR_REPLY],
            (dir<IP_CT_DIR_MAX)?strIpctDir[dir]:strIpctDir[IP_CT_DIR_MAX],
            (ct->features & NF_CT_F_HELP)?1:0, (int)help_p,
            (help_p && help_p->helper) ? help_p->helper->name : "NONE" );

    printk( "\t\tSTATUS[ " );
    for ( bitix = 0; bitix <= IPS_BLOG_BIT; bitix++ )
        if ( ct->status & (1 << bitix) )
            printk( "%s ", strIpctStatus[bitix] );
    printk( "]\n" );

    printk( "\t\tEVENT[ " );
    for ( bitix = 0; bitix <= IPCT_BLOG_BIT; bitix++ )
        if ( events & ( 1 << bitix) )
            printk( "%s ", strIpctEvents[bitix] );
    printk( "]\n" );

}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_netdev_dump
 * Description  : Dump the contents of a net_device object.
 *  dev_p       : Pointer to a net_device object
 *
 * CAUTION      : Net device is not held !!!
 *
 *------------------------------------------------------------------------------
 */
static void blog_netdev_dump( struct net_device * dev_p )
{
    int i;
    printk( "\tDEVICE: %s dev_p<0x%08x>: poll[<%08x>] hard_start_xmit[<%08x>]\n"
            "\t  hard_header[<%08x>] hard_header_cache[<%08x>]\n"
            "\t  dev_addr[ ", dev_p->name,
            (int)dev_p, (int)dev_p->poll, (int)dev_p->hard_start_xmit,
            (int)dev_p->hard_header, (int)dev_p->hard_header_cache );
    for ( i=0; i<dev_p->addr_len; i++ )
        printk( "%02x ", *((uint8_t *)(dev_p->dev_addr) + i) );
    printk( "]\n" );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_tuple_dump
 * Description  : Dump the contents of a BlogTuple_t object.
 *  bTuple_p    : Pointer to the BlogTuple_t object
 *------------------------------------------------------------------------------
 */
static void blog_tuple_dump( BlogTuple_t * bTuple_p )
{
    printk( "\tTUPLE: Src<%u.%u.%u.%u:%u> Dst<%u.%u.%u.%u:%u>\n"
            "\t\tttl<%3u> tos<%3u> check<0x%04x>\n",
            _IP_(bTuple_p->saddr), bTuple_p->port.source,
            _IP_(bTuple_p->daddr), bTuple_p->port.dest,
            bTuple_p->ttl, bTuple_p->tos, bTuple_p->check );
}
 
/*
 *------------------------------------------------------------------------------
 * Function     : blog_l2_dump
 * Description  : parse and dump the contents of all L2 headers
 *  bHdr_p      : Pointer to logged header
 *------------------------------------------------------------------------------
 */
void blog_l2_dump( BlogHeader_t * bHdr_p)
{
    register int i, ix, length, offset = 0;
    BlogEncap_t type;
    char * value = bHdr_p->l2hdr;

    for ( ix=0; ix<bHdr_p->info.count; ix++ )
    {
        type = bHdr_p->encap[ix];

        switch ( type )
        {
            case PPP_1661   : length = sizeof(hProto_t);        break;
            case PPPoE_2516 : length = sizeof(struct pppoe_hdr)
                                     + sizeof(uint16_t);        break;
            case VLAN_8021Q : length = sizeof(struct vlan_hdr); break;
            case ETH_802x   : length = sizeof(struct ethhdr);   break;
            case BCM_SWC    : length = sizeof(struct bcmhdr);   break;

            case WLAN_HDR   :
            case USB_CDC11  :
            case BCM_XTM    :
            default         : printk( "Unsupported type %d\n", type );
                              return;
        }

        printk( "\tENCAP %d. %10s +%2d %2d [ ",
                ix, strBlogEncap[type], offset, length );

        for ( i=0; i<length; i++ )
            printk( "%02x ", (uint8_t)value[i] );

        offset += length;
        value += length;

        printk( "]\n" );
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_dump
 * Description  : Dump the contents of a Blog object.
 *  blog_p      : Pointer to the Blog_t object
 *------------------------------------------------------------------------------
 */
void blog_dump( Blog_t * blog_p )
{
    if ( blog_p == BLOG_NULL )
        return;

    printk( "BLOG <0x%08x> owner<0x%08x> ct<0x%08x><%s>\n"
            "\t\tfdb_src<0x%08x> fdb_dst<0x%08x>\n"
            "\t\tprio<0x%08x> prot<%u> mark<0x%08x>\n",
            (int)blog_p, (int)blog_p->skb_p, (int)blog_p->rx.ct_p,
            (blog_p->rx.info.bmap.BCM_XTM) ? "XTM" : "NonXTM",
            (int)blog_p->fdb_src, (int)blog_p->fdb_dst,
            blog_p->priority, blog_p->protocol, blog_p->mark );

    if ( blog_p->nfct_events )
        blog_nfct_dump( blog_p->skb_p, blog_p->rx.ct_p,
                        blog_p->nfct_events, blog_p->rx.nf_dir );

    printk( "  RX count<%u> conn_id<%02u> RFC2684<%2u> bmap<%x>\n",
            blog_p->rx.info.count, blog_p->rx.info.channel,
            blog_p->rx.info.rfc2684, blog_p->rx.info.hdrs );
    blog_tuple_dump( &blog_p->rx.tuple );
    blog_l2_dump( &blog_p->rx );

    printk( "  TX count<%u> conn_id<%02u> RFC2684<%2u> bmap<%x>\n",
            blog_p->tx.info.count, blog_p->tx.info.channel,
            blog_p->tx.info.rfc2684, blog_p->tx.info.hdrs );
    blog_netdev_dump( blog_p->tx.dev_p );
    blog_tuple_dump( &blog_p->tx.tuple );
    blog_l2_dump( &blog_p->tx );

#if defined(CC_CONFIG_BLOG_DEBUG)
    printk( "\t\textends<%d> free<%d> used<%d> HWM<%d> fails<%d>\n",
            blog_extends, blog_cnt_free, blog_cnt_used, blog_cnt_hwm,
            blog_cnt_fails );
#endif

}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind
 * Description  : Override default rx and tx hooks.
 *  blog_rx     : Function pointer to be invoked in blog_init()
 *  blog_tx     : Function pointer to be invoked in blog_emit()
 *  blog_xx     : Function pointer to be invoked in blog_stop()
 *------------------------------------------------------------------------------
 */
void blog_bind( BlogHook_t blog_rx, BlogHook_t blog_tx, BlogStop_t blog_xx )
{
    blog_print( "Bind Rx[<%08x>] Tx[<%08x>] Stop[<%08x>]",
                (int)blog_rx, (int)blog_tx, (int)blog_xx );
    blog_rx_hook_g = blog_rx;   /* Receive  hook */
    blog_tx_hook_g = blog_tx;   /* Transmit hook */
    blog_xx_hook_g = blog_xx;   /* Destroy  hook */
}

/*
 *------------------------------------------------------------------------------
 * Function     : __init_blog
 * Description  : Incarnates the blog system during kernel boot sequence,
 *                in phase subsys_initcall()
 *------------------------------------------------------------------------------
 */
static int __init __init_blog( void )
{
    nfskb_p = alloc_skb( 0, GFP_ATOMIC );
    blog_refresh_fn = (blog_refresh_t)NULL;
    blog_construct( BLOG_EXTEND_SIZE * BLOG_EXTEND_MAX );
    blog_print( "%d Blogs allocated of size %d", 
                BLOG_EXTEND_SIZE * BLOG_EXTEND_MAX, sizeof(Blog_t) );
    printk( CLRb "BLOG %s Initialized" CLRN, BLOG_VERSION );
    return 0;
}

subsys_initcall(__init_blog);

EXPORT_SYMBOL(strBlogAction);
EXPORT_SYMBOL(strBlogEncap);

EXPORT_SYMBOL(strRfc2684);
EXPORT_SYMBOL(rfc2684HdrLength);
EXPORT_SYMBOL(rfc2684HdrData);

EXPORT_SYMBOL(blog_refresh_fn);
EXPORT_SYMBOL(blog_refresh_br);

EXPORT_SYMBOL(blog_put);
EXPORT_SYMBOL(blog_skb);
EXPORT_SYMBOL(blog_free);
EXPORT_SYMBOL(blog_dump);
EXPORT_SYMBOL(blog_skip);
EXPORT_SYMBOL(blog_null);
EXPORT_SYMBOL(blog_link);
EXPORT_SYMBOL(blog_xfer);
EXPORT_SYMBOL(blog_clone);

EXPORT_SYMBOL(blog_br_fdb);
EXPORT_SYMBOL(blog_nfct);
EXPORT_SYMBOL(blog_ctev);
EXPORT_SYMBOL(blog_flow);
EXPORT_SYMBOL(blog_stop);
EXPORT_SYMBOL(blog_time);

EXPORT_SYMBOL(blog_init);
EXPORT_SYMBOL(blog_emit);

EXPORT_SYMBOL(blog);

EXPORT_SYMBOL(blog_bind);
