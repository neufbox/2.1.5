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
//  Filename:       ip_nat_proto_esp.c
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the ESP ALG connectiontracking.
//
*****************************************************************************/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/if.h>

#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_nat_protocol.h>

#define TEMP_SPI_START 1500

static int
esp_in_range(const struct ip_conntrack_tuple *tuple,
	     enum ip_nat_manip_type maniptype,
	     const union ip_conntrack_manip_proto *min,
	     const union ip_conntrack_manip_proto *max)
{
	//printk ( KERN_DEBUG "%s:%s spi %hu dpi %hu sip %u.%u.%u.%u dip %u.%u.%u.%u\n",
	//	 __FILE__, __FUNCTION__, ntohs(tuple->src.u.esp.spi),
	//	 ntohs(tuple->dst.u.esp.spi), NIPQUAD(tuple->src.ip), NIPQUAD(tuple->dst.ip) );
	return 1;
}

static int
esp_unique_tuple(struct ip_conntrack_tuple *tuple,
                  const struct ip_nat_range *range,
                  enum ip_nat_manip_type maniptype,
                  const struct ip_conntrack *conntrack)
{
        static u_int16_t id = 0, *tspi;
        unsigned int range_size = 0x40;
        unsigned int i;

	//printk ( KERN_DEBUG "%s:%s manitype %d sip %u.%u.%u.%u dip %u.%u.%u.%u"
	//	   " sspi %u dspi %u\n", __FILE__, __FUNCTION__, maniptype,
	//	   NIPQUAD(tuple->src.ip),
	//	   NIPQUAD(tuple->dst.ip), tuple->src.u.esp.spi, tuple->dst.u.esp.spi );
        /* If no range specified... */
        if (!(range->flags & IP_NAT_RANGE_PROTO_SPECIFIED))
                range_size = 0x40;

	if ( maniptype == IP_NAT_MANIP_SRC ) {
		tspi = &tuple->src.u.esp.spi;
	} else {
		tspi = &tuple->dst.u.esp.spi;
	}

        for (i = 0; i < range_size; i++, id++) {
                *tspi = TEMP_SPI_START + (id % range_size);
                if (!ip_nat_used_tuple(tuple, conntrack))
                        return 1;
        }

        return 0;
}

static int
esp_manip_pkt(struct sk_buff **pskb,
              unsigned int hdroff,	
              const struct ip_conntrack_manip *manip,
              enum ip_nat_manip_type maniptype)
{
        u_int32_t oldip;
	//struct esphdr *hdr = (void *)(*pskb)->data + hdroff;

        if (maniptype == IP_NAT_MANIP_SRC) {
                /* Get rid of src ip and src pt */
                oldip = (*pskb)->nh.iph->saddr;
		//printk ( KERN_DEBUG "%s:%s MANIP_SRC oldip %u.%u.%u.%u daddr"
		//	   " %u.%u.%u.%u manip %u.%u.%u.%u spi 0x%x seq 0x%x\n",
		//	   __FILE__, __FUNCTION__, NIPQUAD(oldip), NIPQUAD(iph->daddr),
		//	   NIPQUAD(manip->ip), ntohl(hdr->spi),
		//	   ntohl(hdr->seq) );
        } else {
                /* Get rid of dst ip and dst pt */
                oldip = (*pskb)->nh.iph->daddr;
		//printk ( KERN_DEBUG "%s:%s MANIP_DST oldip %u.%u.%u.%u saddr"
		//	   " %u.%u.%u.%u manip %u.%u.%u.%u spi 0x%x seq 0x%x\n",
		//	   __FILE__, __FUNCTION__, NIPQUAD(oldip), NIPQUAD(iph->saddr),
		//	   NIPQUAD(manip->ip), ntohl(hdr->spi),
		//	   ntohl(hdr->seq) );
        }
	return 1;
}

static unsigned int
esp_print(char *buffer,
	  const struct ip_conntrack_tuple *match,
	  const struct ip_conntrack_tuple *mask)
{
	unsigned int len = 0;

	//printk ( KERN_DEBUG "%s:%s mask spi %u dpi %u\n",
	//	 __FILE__, __FUNCTION__, mask->src.u.esp.spi, mask->dst.u.esp.spi );
	if (mask->src.u.esp.spi)
		len += sprintf(buffer + len, "spi=%u ",
			       ntohs(match->src.u.esp.spi));

	return len;
}

static unsigned int
esp_print_range(char *buffer, const struct ip_nat_range *range)
{
	//printk ( KERN_DEBUG "%s:%s min %u max %u\n", __FILE__, __FUNCTION__,
	//	 ntohs(range->min.esp.spi), ntohs(range->max.esp.spi) );
	if (range->min.esp.spi != 0 ) {
		return sprintf(buffer, "port %u ",
			       ntohs(range->min.esp.spi));
	} else if ( range->max.esp.spi != 0 ) {
		return sprintf(buffer, "ports %u-%u ",
			       ntohs(range->min.esp.spi),
			       ntohs(range->max.esp.spi));
	}
	else return 0;
}

struct ip_nat_protocol ip_nat_protocol_esp
= { { NULL, NULL }, "ESP", IPPROTO_ESP,
    esp_manip_pkt,
    esp_in_range,
    esp_unique_tuple,
    esp_print,
    esp_print_range
};
