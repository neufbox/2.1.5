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
/******************************************************************************
//
//  Filename:       ip_conntrack_proto_esp.c
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the ESP ALG connectiontracking.
//
*****************************************************************************/
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <linux/in.h>
#ifdef CONFIG_MIPS_BRCM
#include <linux/ip.h>
#endif
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_esp.h>

#define ESP_REF_TMOUT   (30 * HZ)
#define ESP_CONN_TMOUT  (60 * HZ * 6)
#define ESP_TMOUT_COUNT (ESP_CONN_TMOUT/ESP_REF_TMOUT)

#ifdef CONFIG_MIPS_BRCM
#define ESP_UNREPLIEDDNS_TIMEOUT (1*HZ)
#endif

#define IPSEC_FREE     0
#define IPSEC_INUSE    1
#define MAX_PORTS      64
#define TEMP_SPI_START 1500

struct _esp_table {
        u_int32_t l_spi;
        u_int32_t r_spi;
        u_int32_t l_ip;
        u_int32_t r_ip;
        u_int32_t timeout;
        u_int16_t tspi;
        struct ip_conntrack *ct;
        struct timer_list   refresh_timer;
        int                 timer_active;
        int                 pkt_rcvd;
        int                 ntimeouts;
        int       inuse;
};

static struct _esp_table esp_table[MAX_PORTS];

#if 0
#define DEBUGP(format, args...) printk(__FILE__ ":" __FUNCTION__ ": " \
				       format, ## args)
#else
#define DEBUGP(format, args...)
#endif

static u_int16_t cur_spi = 0;

static void
esp_free_entry(int index)
{
    if (esp_table[index].inuse) {
        if (esp_table[index].timer_active) {
#if IP_CONNTRACK_ESP_DEBUG
            printk (KERN_DEBUG "%s:%s Deleting timer index %d\n",
                    __FILE__, __FUNCTION__, index);
#endif
            del_timer(&esp_table[index].refresh_timer);
        }
        memset(&esp_table[index], 0, sizeof(struct _esp_table));
    }
}

static void
esp_refresh_ct(unsigned long data)
{
    struct _esp_table *esp_entry = NULL;

    if (data > MAX_PORTS) {
        return;
    }

    esp_entry = &esp_table[data];
    if ( esp_entry == NULL ) {
        return;
    }
#if IP_CONNTRACK_ESP_DEBUG
    printk(KERN_DEBUG "ntimeouts %d pkt_rcvd %d entry %p data %lu ct %p\n",
           esp_entry->ntimeouts, esp_entry->pkt_rcvd, esp_entry, data,
           esp_entry->ct);
#endif
    if (esp_entry->pkt_rcvd) {
        esp_entry->pkt_rcvd  = 0;
        esp_entry->ntimeouts = 0;
    } else {
        esp_entry->ntimeouts++;
        if (esp_entry->ntimeouts >= ESP_TMOUT_COUNT) {
            esp_free_entry(data);
            return;
        }
    }
    esp_entry->refresh_timer.expires = jiffies + ESP_REF_TMOUT;
    esp_entry->refresh_timer.function = esp_refresh_ct;
    esp_entry->refresh_timer.data = data;
    ip_ct_refresh(esp_entry->ct, ESP_CONN_TMOUT);
    add_timer(&esp_entry->refresh_timer);
    esp_entry->timer_active = 1;
#if IP_CONNTRACK_ESP_DEBUG
    printk(KERN_DEBUG "%s:%s Refreshed timer pkt_rcvd %d timeouts %d\n",
           __FILE__, __FUNCTION__, esp_entry->pkt_rcvd, esp_entry->ntimeouts);
#endif
}

/*
 * Allocate a free IPSEC table entry.
 */
struct _esp_table *alloc_esp_entry ( void )
{
	int idx = 0;
	struct _esp_table *esp_entry = esp_table;

	for ( ; idx < MAX_PORTS; idx++ ) {
		if ( esp_entry->inuse == IPSEC_FREE ) {
			esp_entry->tspi  = cur_spi = TEMP_SPI_START + idx;
			esp_entry->inuse = IPSEC_INUSE;
#if IP_CONNTRACK_ESP_DEBUG
			printk ( KERN_DEBUG "%s:%s New esp_entry at idx %d entry %p"
				   " tspi %u cspi %u\n", __FILE__, __FUNCTION__, idx,
				   esp_entry, esp_entry->tspi, cur_spi );
#endif
                        init_timer(&esp_entry->refresh_timer);
                        esp_entry->refresh_timer.data     = idx;
                        esp_entry->pkt_rcvd               = 0;
                        esp_entry->refresh_timer.expires  = jiffies + ESP_REF_TMOUT;
                        esp_entry->refresh_timer.function = esp_refresh_ct;
                        add_timer(&esp_entry->refresh_timer);
                        esp_entry->timer_active         = 1;
			return esp_entry;
		}
		esp_entry++;
	}
	return NULL;
}

/*
 * Search an ESP table entry by the Security Parameter Identifier (SPI).
 */
struct _esp_table *search_esp_entry_by_spi ( const struct esphdr *esph,
					     u_int32_t daddr )
{
	int idx = 0;
	struct _esp_table *esp_entry = esp_table;

#if IP_CONNTRACK_ESP_DEBUG
        printk ( KERN_DEBUG "%s:%s (0x%x) %u.%u.%u.%u\n", __FILE__, __FUNCTION__,
                 ntohl(esph->spi), NIPQUAD(daddr));
#endif
	for ( ; idx < MAX_PORTS; idx++, esp_entry++ ) {
		if ( esp_entry->inuse == IPSEC_FREE ) {
			continue;
		}
		/* If we have seen traffic both ways */
		if ( esp_entry->l_spi != 0 && esp_entry->r_spi != 0 ) {
			if ( esp_entry->l_spi == ntohl(esph->spi) ||
			     esp_entry->r_spi == ntohl(esph->spi) ) {
#if IP_CONNTRACK_ESP_DEBUG
				printk (KERN_DEBUG "%s:%s Both Ways Traffic Entry %p\n",
					__FILE__, __FUNCTION__, esp_entry);
#endif
				return esp_entry;
			}
			continue;
		}
		/* If we have seen traffic only one way */
		if ( esp_entry->l_spi == 0 || esp_entry->r_spi == 0 ) {
			/* We have traffic from local */
			if ( esp_entry->l_spi ) {
				if ( ntohl(esph->spi) == esp_entry->l_spi ) {
#if IP_CONNTRACK_ESP_DEBUG
					printk (KERN_DEBUG "%s:%s One Way Traffic From Local Entry %p\n",
						__FILE__, __FUNCTION__, esp_entry);
#endif
					return esp_entry;
				}
				/* This must be the first packet from remote */
				esp_entry->r_spi = ntohl(esph->spi);
				esp_entry->r_ip = ntohl(daddr);
#if IP_CONNTRACK_ESP_DEBUG
				printk (KERN_DEBUG "%s:%s First Packet from Remote Entry %p\n",
					__FILE__, __FUNCTION__, esp_entry);
#endif
				return esp_entry;
			/* We have seen traffic only from remote */
			} else if ( esp_entry->r_spi ) {
				if ( ntohl(esph->spi) == esp_entry->r_spi ) {
#if IP_CONNTRACK_ESP_DEBUG
					printk (KERN_DEBUG "%s:%s One Way Traffic From Remote Entry %p\n",
						__FILE__, __FUNCTION__, esp_entry);
#endif
					return esp_entry;
				}
				/* This must be the first packet from local */
				esp_entry->l_spi = ntohl(esph->spi);
#if IP_CONNTRACK_ESP_DEBUG
				printk (KERN_DEBUG "%s:%s First Packet From Remote Entry %p\n",
					__FILE__, __FUNCTION__, esp_entry);
#endif
				return esp_entry;
			}
		}
	}
#if IP_CONNTRACK_ESP_DEBUG
	printk (KERN_DEBUG "%s:%s No Entry\n", __FILE__, __FUNCTION__);
#endif
	return NULL;
}

static int esp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
			    struct ip_conntrack_tuple *tuple)
{
	struct esphdr esph;
	struct _esp_table *esp_entry;
	const struct iphdr *iph = skb->nh.iph;

	if (skb_copy_bits(skb, dataoff, &esph, sizeof(esph)) != 0)
		return 0;
#if IP_CONNTRACK_ESP_DEBUG
        printk ( KERN_DEBUG "%s:%s (0x%x) IP Pkt Hdr %u.%u.%u.%u <-> %u.%u.%u.%u\n",
                 __FILE__, __FUNCTION__, ntohl(esph.spi),
                 NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
        printk ( KERN_DEBUG "%s:%s (0x%x) %u.%u.%u.%u <-> %u.%u.%u.%u\n", 
                 __FILE__, __FUNCTION__, ntohl(esph.spi),
                 NIPQUAD(tuple->src.ip), NIPQUAD(tuple->dst.ip) );
#endif

	if ( (esp_entry = search_esp_entry_by_spi ( &esph, tuple->dst.ip ) ) == NULL ) {
		esp_entry = alloc_esp_entry();
		if ( esp_entry == NULL ) {
			return 0;
		}
		esp_entry->l_spi = ntohl(esph.spi);
		esp_entry->l_ip  = ntohl(tuple->src.ip);
	}
#if IP_CONNTRACK_ESP_DEBUG
	printk ( KERN_DEBUG "%s:%s tspi %u cspi %u spi 0x%x seq 0x%x"
		   " sip %u.%u.%u.%u dip %u.%u.%u.%u\n", __FILE__,
		   __FUNCTION__, esp_entry->tspi, cur_spi,
		   ntohl(esph.spi), ntohl(esph.seq),
		   NIPQUAD(tuple->src.ip), NIPQUAD(tuple->dst.ip) );
#endif
	tuple->dst.u.esp.spi = esp_entry->tspi;
	tuple->src.u.esp.spi = esp_entry->tspi;
        esp_entry->pkt_rcvd++;
	return 1;
}

static int esp_invert_tuple(struct ip_conntrack_tuple *tuple,
			    const struct ip_conntrack_tuple *orig)
{
#if IP_CONNTRACK_ESP_DEBUG
	printk ( KERN_DEBUG "%s:%s cspi 0x%x dspi 0x%x sspi 0x%x"
		   " %u.%u.%u.%u <-> %u.%u.%u.%u\n",
		   __FILE__, __FUNCTION__, cur_spi, orig->dst.u.esp.spi,
		   orig->src.u.esp.spi, NIPQUAD(tuple->src.ip),
		   NIPQUAD(tuple->dst.ip) );
#endif
	tuple->src.u.esp.spi = orig->dst.u.esp.spi;
	tuple->dst.u.esp.spi = orig->src.u.esp.spi;
	return 1;
}

/* Print out the per-protocol part of the tuple. */
static unsigned int esp_print_tuple(char *buffer,
				    const struct ip_conntrack_tuple *tuple)
{
	return sprintf(buffer, "sport=%u dport=%u ",
		       ntohs(tuple->src.u.esp.spi), ntohs(tuple->dst.u.esp.spi));
}

/* Print out the private part of the conntrack. */
static unsigned int esp_print_conntrack(char *buffer,
					const struct ip_conntrack *conntrack)
{
	return 0;
}

/* Returns verdict for packet, and may modify conntracktype */
static int esp_packet(struct ip_conntrack *conntrack,
		      const struct sk_buff *skb,
		      enum ip_conntrack_info conntrackinfo)
{
	const struct iphdr *iph = skb->nh.iph;
	const struct esphdr *esph = (void *)iph + iph->ihl*4;
	struct _esp_table *esp_entry;

#if IP_CONNTRACK_ESP_DEBUG
        printk ( KERN_DEBUG "%s:%s (0x%x) %u.%u.%u.%u <-> %u.%u.%u.%u %s\n",
                 __FILE__, __FUNCTION__, ntohl(esph->spi), 
                 NIPQUAD(iph->saddr), NIPQUAD(iph->daddr),
                 (conntrackinfo == IP_CT_NEW ) ? "CT_NEW" : "SEEN_REPLY" );
#endif
	/*
	 * This should not happen. We get into this routine only if there is
	 * an existing stream.
	 */
#if IP_CONNTRACK_ESP_DEBUG
	printk(KERN_DEBUG "%s:%s conntrackinfo %d\n", __FILE__, __FUNCTION__, conntrackinfo);
#endif
	if (conntrackinfo == IP_CT_NEW ) {
#if IP_CONNTRACK_ESP_DEBUG
		printk ( KERN_DEBUG "%s:%s IP_CT_NEW (0x%x) %u.%u.%u.%u <-> %u.%u.%u.%u\n",
                         __FILE__, __FUNCTION__, esph->spi, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
#endif
		if ( (esp_entry = search_esp_entry_by_spi ( esph, 
					iph->daddr ) ) == NULL ) {
			esp_entry = alloc_esp_entry ();
                        if ( esp_entry == NULL ) {
                               	/* All entries are currently in use */
				printk ( KERN_DEBUG "%s:%s All connections in use\n",
                                         __FILE__, __FUNCTION__);
                               	return NF_DROP;
                        }
                        esp_entry->l_spi = ntohl(esph->spi);
                        esp_entry->l_ip  = ntohl(iph->saddr);
                        esp_entry->r_spi = 0;
		}
                esp_entry->pkt_rcvd++;
#if IP_CONNTRACK_ESP_DEBUG
		printk( KERN_DEBUG "%s:%s Received COUNT %d\n", __FILE__, __FUNCTION__, esp_entry->pkt_rcvd);
#endif
	}
	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */
#if IP_CONNTRACK_ESP_DEBUG
	printk(KERN_DEBUG "%s:%s status %lu\n", __FILE__, __FUNCTION__, conntrack->status);
#endif
	if (conntrack->status & IPS_SEEN_REPLY) {
		ip_ct_refresh(conntrack, ESP_CONN_TMOUT);
		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &conntrack->status);
	} else {
		ip_ct_refresh(conntrack, ESP_REF_TMOUT);
    	}
	//esp_entry = search_esp_entry_by_spi ( esph, iph->daddr );

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static int esp_new(struct ip_conntrack *conntrack, const struct sk_buff *skb)
{
	const struct iphdr *iph = skb->nh.iph;
	const struct esphdr *esph = (void *)iph + iph->ihl*4;
	struct _esp_table *esp_entry;
#if IP_CONNTRACK_ESP_DEBUG
        printk ( KERN_DEBUG "%s:%s (0x%x) %u.%u.%u.%u <-> %u.%u.%u.%u\n",
                 __FILE__, __FUNCTION__, ntohl(esph->spi),
                 NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
#endif
	if ( (esp_entry = search_esp_entry_by_spi ( esph, iph->daddr ) ) == NULL ) {
		/*
		 * Check if this is the same LAN client creating another session.
		 * If this is true, then the LAN IP address will be the same with
		 * a new SPI value. This would indicate that the entire transaction
		 * using the previous value of SPI is now not required.
		 */
		esp_entry = alloc_esp_entry ();
		if ( esp_entry == NULL ) {
			/* All entries are currently in use */
			printk ( KERN_DEBUG "%s:%s All connections in use\n",
                                 __FILE__, __FUNCTION__);
			return NF_DROP;
		}
		esp_entry->l_spi = ntohl(esph->spi);
		esp_entry->l_ip  = ntohl(iph->saddr);
		esp_entry->r_spi = 0;
	}
        esp_entry->pkt_rcvd++;
        esp_entry->ct = conntrack;
	return 1;
}

struct ip_conntrack_protocol ip_conntrack_protocol_esp
= { { NULL, NULL }, IPPROTO_ESP, "esp",
    esp_pkt_to_tuple, esp_invert_tuple, esp_print_tuple, esp_print_conntrack,
    esp_packet, esp_new, NULL };
