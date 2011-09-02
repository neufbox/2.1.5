/*
 *  ebt_wmm_mark
 *
 */
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_wmm_mark_t.h>
#include <linux/if_vlan.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/skbuff.h>

static int ebt_target_wmm_mark(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_wmm_mark_t_info *info = (struct ebt_wmm_mark_t_info *)data;

	struct iphdr *iph;
	struct vlan_hdr *frame;	
	unsigned char prio = 0;
	unsigned short TCI;

	if (info->markset != WMM_MARK_VALUE_NONE) {
		/* use marset regardless of supported classification method */
		prio = (unsigned char)info->markset;

      if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q)) {

         unsigned short pbits = (unsigned short)(info->markset & 0x0000f000);

         if (pbits) {
            frame = (struct vlan_hdr *)((*pskb)->nh.raw);
            TCI = ntohs(frame->h_vlan_TCI);
		      TCI = (TCI & 0x1fff) | (((pbits >> 12) - 1) << 13);
            frame->h_vlan_TCI = htons(TCI);
         }
      }
	} else if (info->mark & WMM_MARK_8021D) {
		if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q)) {
			frame = (struct vlan_hdr *)((*pskb)->nh.raw);
			TCI = ntohs(frame->h_vlan_TCI);
			prio = (unsigned char)((TCI >> 13) & 0x7);
        	} else
			return EBT_CONTINUE;        	
        					
	} else if (info->mark & WMM_MARK_DSCP) {
		
		/* if VLAN frame, we need to point to correct network header */
		if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q))
        		iph = (struct iphdr *)((*pskb)->nh.raw + VLAN_HLEN);
        	/* ip */
        	else if ((*pskb)->protocol == __constant_htons(ETH_P_IP))
			iph = (*pskb)->nh.iph;
		else
		/* pass for others */
			return EBT_CONTINUE;

		prio = iph->tos>>WMM_DSCP_MASK_SHIFT ;
	}
		
    //printk("markset 0x%08x, mark 0x%x, mark 0x%x \n", info->markset, info->mark, (*pskb)->mark);
	if(prio) {
		(*pskb)->mark &= ~(PRIO_LOC_NFMASK << info->markpos);		
		(*pskb)->mark |= (prio << info->markpos);
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// member below is removed
//		(*pskb)->nfcache |= NFC_ALTERED;
		//printk("mark 0x%x, mark 0x%x\n",( prio << info->markpos), (*pskb)->mark);			
	}
		
	return info->target;
}

static int ebt_target_wmm_mark_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_wmm_mark_t_info *info = (struct ebt_wmm_mark_t_info *)data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_wmm_mark_t_info)))
		return -EINVAL;
	
	//printk("e->ethproto=0x%x, e->invflags=0x%x\n",e->ethproto, e->invflags);
		
	if ((e->ethproto != __constant_htons(ETH_P_IP) && e->ethproto != __constant_htons(ETH_P_8021Q)) ||
	   e->invflags & EBT_IPROTO)
		return -EINVAL;
				
	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
		
	CLEAR_BASE_CHAIN_BIT;
	if (INVALID_TARGET)
		return -EINVAL;
	return 0;
	
}

static struct ebt_target mark_target =
{
	.name		= EBT_WMM_MARK_TARGET,
	.target		= ebt_target_wmm_mark,
	.check		= ebt_target_wmm_mark_check,
	.me		= THIS_MODULE,
};

static int __init init(void)
{
	return ebt_register_target(&mark_target);
}

static void __exit fini(void)
{
	ebt_unregister_target(&mark_target);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
