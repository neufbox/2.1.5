/*
 *  ebt_ftos
 *
 *	Authors:
 *	 Song Wang <songw@broadcom.com>
 *
 *  Feb, 2004
 *
 */

// The ftos target can be used in any chain
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ftos_t.h>


static int ebt_target_ftos(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_ftos_t_info *ftosinfo = (struct ebt_ftos_t_info *)data;
	struct iphdr *iph;
        struct vlan_hdr *frame;	
	unsigned char prio = 0;
	unsigned short TCI;
        /* Need to recalculate IP header checksum after altering TOS byte */
	u_int16_t diffs[2];

	/* if VLAN frame, we need to point to correct network header */
	if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q))
        	iph = (struct iphdr *)((*pskb)->nh.raw + VLAN_HLEN);
        else
		iph = (*pskb)->nh.iph;


	if ((ftosinfo->ftos_set & FTOS_SETFTOS) && (iph->tos != ftosinfo->ftos)) {
                //printk("ebt_target_ftos:FTOS_SETFTOS .....\n");
		/* raw socket (tcpdump) may have clone of incoming
                   skb: don't disturb it --RR */
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;
			if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q))
                		iph = (struct iphdr *)((*pskb)->nh.raw + VLAN_HLEN);
        		else
				iph = (*pskb)->nh.iph;
		}


		diffs[0] = htons(iph->tos) ^ 0xFFFF;
		iph->tos = ftosinfo->ftos;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF));		
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// member below is removed
//		(*pskb)->nfcache |= NFC_ALTERED;
	} else if (ftosinfo->ftos_set & FTOS_WMMFTOS) {
	    //printk("ebt_target_ftos:FTOS_WMMFTOS .....0x%08x\n", (*pskb)->mark & 0xf);
		/* raw socket (tcpdump) may have clone of incoming
                   skb: don't disturb it --RR */
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;
			if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q))
                		iph = (struct iphdr *)((*pskb)->nh.raw + VLAN_HLEN);
        		else
				iph = (*pskb)->nh.iph;
	}

        diffs[0] = htons(iph->tos) ^ 0xFFFF;
	    iph->tos |= (((*pskb)->mark >> PRIO_LOC_NFMARK) & PRIO_LOC_NFMASK) << DSCP_MASK_SHIFT;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF));
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// member below is removed
//        (*pskb)->nfcache |= NFC_ALTERED;
	} else if ((ftosinfo->ftos_set & FTOS_8021QFTOS) && (*pskb)->protocol == __constant_htons(ETH_P_8021Q)) {
	    
	    /* raw socket (tcpdump) may have clone of incoming
           skb: don't disturb it --RR */
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;

            iph = (struct iphdr *)((*pskb)->nh.raw + VLAN_HLEN);
            frame = (struct vlan_hdr *)((*pskb)->nh.raw);
			TCI = ntohs(frame->h_vlan_TCI);
			prio = (unsigned char)((TCI >> 13) & 0x7);
		}
        //printk("ebt_target_ftos:FTOS_8021QFTOS ..... 0x%08x\n", prio);
        diffs[0] = htons(iph->tos) ^ 0xFFFF;
	    iph->tos |= (prio & 0xf) << DSCP_MASK_SHIFT;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF)); 
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// member below is removed
//        (*pskb)->nfcache |= NFC_ALTERED;
	}

	return ftosinfo->target;
}

static int ebt_target_ftos_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_ftos_t_info *info = (struct ebt_ftos_t_info *)data;

	if (datalen != sizeof(struct ebt_ftos_t_info))
		return -EINVAL;
	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
	CLEAR_BASE_CHAIN_BIT;
	if (INVALID_TARGET)
		return -EINVAL;
	return 0;
}

static struct ebt_target ftos_target =
{
        .name           = EBT_FTOS_TARGET,
        .target         = ebt_target_ftos,
        .check          = ebt_target_ftos_check,
        .me             = THIS_MODULE,
};

static int __init init(void)
{
	return ebt_register_target(&ftos_target);
}

static void __exit fini(void)
{
	ebt_unregister_target(&ftos_target);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Song Wang, songw@broadcom.com");
MODULE_DESCRIPTION("Target to overwrite the full TOS byte in IP header");
