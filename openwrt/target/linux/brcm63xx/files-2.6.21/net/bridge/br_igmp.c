#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include <linux/ip.h>
#include "br_private.h"
#include "br_igmp.h"
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_MIPS_BRCM)
#include <linux/blog.h>
#endif

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)

int br_igmp_snooping = 0;
struct proc_dir_entry *br_igmp_entry = NULL;

static void addr_conv(unsigned char *in, char * out)
{
    sprintf(out, "%02x%02x%02x%02x%02x%02x", in[0], in[1], in[2], in[3], in[4], in[5]);
}

/* Define ipv6 multicast mac address to let them pass through control filtering.
 * All ipv6 multicast mac addresses start with 0x33 0x33. So control_filter
 * only need to compare the first 2 bytes of the address.
 */
static mac_addr ipv6_mc_addr = {{0x33, 0x33, 0x00, 0x00, 0x00, 0x00}}; /* only the left two bytes are significant */

static mac_addr upnp_addr = {{0x01, 0x00, 0x5e, 0x7f, 0xff, 0xfa}};
static mac_addr sys1_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x01}};
static mac_addr sys2_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x02}};
static mac_addr ospf1_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x05}};
static mac_addr ospf2_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x06}};
static mac_addr ripv2_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x09}};
static mac_addr sys_addr = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

static int control_filter(unsigned char *dest)
{
   if ((!memcmp(dest, &upnp_addr, ETH_ALEN)) ||
       (!memcmp(dest, &sys1_addr, ETH_ALEN)) ||
       (!memcmp(dest, &sys2_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ospf1_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ospf2_addr, ETH_ALEN)) ||
       (!memcmp(dest, &sys_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ripv2_addr, ETH_ALEN)) ||
       (!memcmp(dest, &ipv6_mc_addr, 2)))
      return 0;
   else
      return 1;
}

static void conv_ip_to_mac(char *ipa, char *maca)
{
   maca[0] = 0x01;
   maca[1] = 0x00;
   maca[2] = 0x5e;
   maca[3] = 0x7F & ipa[1];
   maca[4] = ipa[2];
   maca[5] = ipa[3];

   return;
}

static void query_timeout(unsigned long ptr)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *tmp;
	struct list_head *lh;
	struct net_bridge *br;
    
	br = (struct net_bridge *) ptr;

	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if (time_after_eq(jiffies, dst->tstamp)) {
		list_del_rcu(&dst->list);
		kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
		
	mod_timer(&br->igmp_timer, jiffies + TIMER_CHECK_TIMEOUT*HZ);		
}

static int br_mc_fdb_update(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	int ret = 0;
	int filt_mode;

        if(mode == SNOOP_IN_ADD)
          filt_mode = MCAST_INCLUDE;
        else
          filt_mode = MCAST_EXCLUDE;
    
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if (!memcmp(&dst->addr, dest, ETH_ALEN)) {
	       if((src->s_addr == dst->src_entry.src.s_addr) &&
	          (filt_mode == dst->src_entry.filt_mode) && 
	          (!memcmp(&dst->host, host, ETH_ALEN)) && 
	          (dst->dst == prt)) {
	             dst->tstamp = jiffies + QUERY_TIMEOUT*HZ;
	             ret = 1;
	       }
#if defined(CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH)
	/* patch for igmp report flooding by robo */
	       else if ((0 == dst->src_entry.src.s_addr) &&
	                (MCAST_EXCLUDE == dst->src_entry.filt_mode)) {
	           dst->tstamp = jiffies + QUERY_TIMEOUT*HZ;
	       }
#endif /* CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH*/
           }
	}

	return ret;
}

static struct net_bridge_mc_fdb_entry *br_mc_fdb_get(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	int filt_mode;
    
        if(mode == SNOOP_IN_CLEAR)
          filt_mode = MCAST_INCLUDE;
        else
          filt_mode = MCAST_EXCLUDE;
          
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && 
                (!memcmp(&dst->host, host, ETH_ALEN)) &&
                (filt_mode == dst->src_entry.filt_mode) && 
                (dst->src_entry.src.s_addr == src->s_addr)) {
		if (dst->dst == prt)
		    return dst;
	    }
	}
	
	return NULL;
}

extern mac_addr upnp_addr;

int br_igmp_mc_fdb_add(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *mc_fdb;
#if defined(CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH)
	struct net_bridge_mc_fdb_entry *mc_fdb_robo;
	struct list_head *lh;
	struct list_head *tmp;
#endif /* CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH */

	//printk("--- add mc entry ---\n");

        if(!br || !prt || !dest || !host)
            return 0;

        if((SNOOP_IN_ADD != mode) && (SNOOP_EX_ADD != mode))             
            return 0;

	if (!memcmp(dest, &upnp_addr, ETH_ALEN))
	    return 0;
	    
	if (br_mc_fdb_update(br, prt, dest, host, mode, src))
	    return 0;

#if defined(CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH)
	/* patch for snooping entry when LAN client access port is moved & 
           igmp report flooding by robo */
	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    mc_fdb_robo = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	   if ((!memcmp(&mc_fdb_robo->addr, dest, ETH_ALEN)) &&
                (0 == mc_fdb_robo->src_entry.src.s_addr) &&
                (MCAST_EXCLUDE == mc_fdb_robo->src_entry.filt_mode) && 
		(!memcmp(&mc_fdb_robo->host, host, ETH_ALEN)) &&
                (mc_fdb_robo->dst != prt)) {
		   list_del_rcu(&mc_fdb_robo->list);
		   kfree(mc_fdb_robo);
	   }
	}
	spin_unlock_bh(&br->mcl_lock);
#endif /* CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH */
	    
	mc_fdb = kmalloc(sizeof(struct net_bridge_mc_fdb_entry), GFP_KERNEL);
	if (!mc_fdb)
	    return ENOMEM;
	memcpy(mc_fdb->addr.addr, dest, ETH_ALEN);
	memcpy(mc_fdb->host.addr, host, ETH_ALEN);
	memcpy(&mc_fdb->src_entry, src, sizeof(struct in_addr));
	mc_fdb->src_entry.filt_mode = 
                  (mode == SNOOP_IN_ADD) ? MCAST_INCLUDE : MCAST_EXCLUDE;
	mc_fdb->dst = prt;
	mc_fdb->tstamp = jiffies + QUERY_TIMEOUT*HZ;

	spin_lock_bh(&br->mcl_lock);
	list_add_tail_rcu(&mc_fdb->list, &br->mc_list);
	spin_unlock_bh(&br->mcl_lock);

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	blog_stop(NULL, NULL);
#endif

	if (!br->start_timer) {
    	    init_timer(&br->igmp_timer);
	    br->igmp_timer.expires = jiffies + TIMER_CHECK_TIMEOUT*HZ;
	    br->igmp_timer.function = query_timeout;
	    br->igmp_timer.data = (unsigned long) br;
	    add_timer(&br->igmp_timer);
	    br->start_timer = 1;
	}

	return 1;
}

void br_igmp_mc_fdb_cleanup(struct net_bridge *br)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	struct list_head *tmp;
    
	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    list_del_rcu(&dst->list);
	    kfree(dst);
	}
	spin_unlock_bh(&br->mcl_lock);
}

void br_igmp_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	struct list_head *tmp;

	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && (dst->dst == prt)) {
		list_del_rcu(&dst->list);
		kfree(dst);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		blog_stop(NULL, NULL);
#endif
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
}

int br_igmp_mc_fdb_remove(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *mc_fdb;

	//printk("--- remove mc entry ---\n");

        if((SNOOP_IN_CLEAR != mode) && (SNOOP_EX_CLEAR != mode))             
            return 0;
	
	if ((mc_fdb = br_mc_fdb_get(br, prt, dest, host, mode, src))) {
	    spin_lock_bh(&br->mcl_lock);
	    list_del_rcu(&mc_fdb->list);
	    kfree(mc_fdb);
	    spin_unlock_bh(&br->mcl_lock);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		blog_stop(NULL, NULL);
#endif

	    return 1;
	}
	
	return 0;
}

static struct net_bridge_mc_fdb_entry *br_mc_fdb_find(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
    
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && 
                (!memcmp(&dst->host, host, ETH_ALEN)) &&
                (dst->src_entry.src.s_addr == src->s_addr)) {
		if (dst->dst == prt)
		    return dst;
	    }
	}
	
	return NULL;
}

void br_igmp_process_info(struct net_bridge *br, struct sk_buff *skb)
{
	char destS[16];
	char srcS[16];
	unsigned char tmp[6];
	unsigned char igmp_type = 0;
	char *extif = NULL;
	struct iphdr *pip = skb->nh.iph;
	unsigned char *src = eth_hdr(skb)->h_source;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

	if (br_igmp_snooping && br->igmp_proxy) {
	  if (skb->data[9] == IPPROTO_IGMP) {
	    if(pip->ihl == 5) {
	      igmp_type = skb->data[20];
	    } else {
	      igmp_type = skb->data[24];
	    }

	    if (igmp_type == IGMP_HOST_LEAVE_MESSAGE) {
			
	      if(pip->ihl == 5) {
	        conv_ip_to_mac(&skb->data[24], tmp);
	      } else {
	        conv_ip_to_mac(&skb->data[28], tmp);
	      }

	      addr_conv(tmp, destS);
	    }
	    else {
	      addr_conv(dest, destS);
            }
	    addr_conv(src, srcS);

	    extif = kmalloc(BCM_SNOOPING_BUFSZ, GFP_ATOMIC);

	    if(extif) {
	      memset(extif, 0, BCM_SNOOPING_BUFSZ);
	      sprintf(extif, "%s %s %s/%s", 
                        br->dev->name, p->dev->name, destS, srcS);
	      skb->extif = extif;
	    }
	  }
	}

	return;
} /* br_process_igmp_info */

static void br_igmp_process_v3(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest, struct igmpv3_report *report)
{
  struct igmpv3_grec *grec;
  int i;
  struct in_addr src;
  union ip_array igmpv3_mcast;
  int num_src;
  unsigned char tmp[6];
  struct net_bridge_mc_fdb_entry *mc_fdb;

  if(report) {
    grec = &report->grec[0];
    for(i = 0; i < report->ngrec; i++) {
      igmpv3_mcast.ip_addr = grec->grec_mca;
      conv_ip_to_mac(igmpv3_mcast.ip_ar, tmp);
      switch(grec->grec_type) {
        case IGMPV3_MODE_IS_INCLUDE:
        case IGMPV3_CHANGE_TO_INCLUDE:
        case IGMPV3_ALLOW_NEW_SOURCES:
          for(num_src = 0; num_src < grec->grec_nsrcs; num_src++) {
            src.s_addr = grec->grec_src[num_src];
            mc_fdb = br_mc_fdb_find(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, &src);
            if((NULL != mc_fdb) && 
               (mc_fdb->src_entry.filt_mode == MCAST_EXCLUDE)) {
              br_igmp_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_CLEAR, &src);
            }
            else {
              br_igmp_mc_fdb_add(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_IN_ADD, &src);
            }
          }

          if(0 == grec->grec_nsrcs) {
            src.s_addr = 0;
            br_igmp_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_CLEAR, &src);
          }
         break;
       
         case IGMPV3_MODE_IS_EXCLUDE:
         case IGMPV3_CHANGE_TO_EXCLUDE:
         case IGMPV3_BLOCK_OLD_SOURCES:
          for(num_src = 0; num_src < grec->grec_nsrcs; num_src++) {
            src.s_addr = grec->grec_src[num_src];
            mc_fdb = br_mc_fdb_find(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, &src);
            if((NULL != mc_fdb) && 
               (mc_fdb->src_entry.filt_mode == MCAST_INCLUDE)) {
              br_igmp_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_IN_CLEAR, &src);
            }
            else {
              br_igmp_mc_fdb_add(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_ADD, &src);
            }
          }

          if(0 == grec->grec_nsrcs) {
            src.s_addr = 0;
            br_igmp_mc_fdb_add(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_ADD, &src);
          }
        break;
      }
      grec = (struct igmpv3_grec *)((char *)grec + IGMPV3_GRP_REC_SIZE(grec));
    }
  }
  return;
}

int br_igmp_mc_forward(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest,int forward, int clone)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	int status = 0;
	struct sk_buff *skb2;
	struct net_bridge_port *p;
	unsigned char tmp[6];
	struct igmpv3_report *report;
	struct iphdr *pip = skb->nh.iph;
	struct in_addr src;
        unsigned char igmp_type = 0;

	if (!br_igmp_snooping)
		return 0;

	if ((br_igmp_snooping == SNOOPING_BLOCKING_MODE) && control_filter(dest))
	    status = 1;

	if (skb->data[9] == IPPROTO_IGMP) {
	    // For proxy; need to add some intelligence here 
	    if (!br->igmp_proxy) {
		if(pip->ihl == 5) {
                  igmp_type = skb->data[20];
		} else {
                  igmp_type = skb->data[24];
		}
		if ((igmp_type == IGMPV2_HOST_MEMBERSHIP_REPORT) &&
		    (skb->protocol == __constant_htons(ETH_P_IP))) {
	            src.s_addr = 0;
		    br_igmp_mc_fdb_add(br, skb->dev->br_port, dest, eth_hdr(skb)->h_source, SNOOP_EX_ADD, &src);
                }
                else if((igmp_type == IGMPV3_HOST_MEMBERSHIP_REPORT) &&
                        (skb->protocol == __constant_htons(ETH_P_IP))) {
                    if(pip->ihl == 5) {
                      report = (struct igmpv3_report *)&skb->data[20];
                    }
                    else {
                      report = (struct igmpv3_report *)&skb->data[24];
                    }
                    if(report) {
                      br_igmp_process_v3(br, skb, dest, report);
                    }
                }
		else if (igmp_type == IGMP_HOST_LEAVE_MESSAGE) {
		    tmp[0] = 0x01;
		    tmp[1] = 0x00;
		    tmp[2] = 0x5e;
                    if(pip->ihl == 5) {
                      tmp[3] = 0x7F & skb->data[24];
                      tmp[4] = skb->data[25];
                      tmp[5] = skb->data[26];
                    } 
                    else {
                      tmp[3] = 0x7F & skb->data[29];
                      tmp[4] = skb->data[30];
                      tmp[5] = skb->data[31];
                    }
	            src.s_addr = 0;
		    br_igmp_mc_fdb_remove(br, skb->dev->br_port, tmp, eth_hdr(skb)->h_source, SNOOP_EX_CLEAR, &src);
		}
		else
		    ;
	    }
	    return 0;
	}

	/*
	if (clone) {
		struct sk_buff *skb3;

		if ((skb3 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb3;
	}
	*/
	
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if (!memcmp(&dst->addr, dest, ETH_ALEN)) {
              if((dst->src_entry.filt_mode == MCAST_INCLUDE) && 
                 (pip->saddr == dst->src_entry.src.s_addr)) {

		if (!dst->dst->dirty) {
		    skb2 = skb_clone(skb, GFP_ATOMIC);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
			blog_clone(skb, skb2->blog_p);
#endif
		    if (forward)
			br_forward(dst->dst, skb2);
		    else
			br_deliver(dst->dst, skb2);
		}
		dst->dst->dirty = 1;
		status = 1;
              }
              else if(dst->src_entry.filt_mode == MCAST_EXCLUDE) {
                if((0 == dst->src_entry.src.s_addr) ||
                   (pip->saddr != dst->src_entry.src.s_addr)) {

		  if (!dst->dst->dirty) {
		    skb2 = skb_clone(skb, GFP_ATOMIC);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
			blog_clone(skb, skb2->blog_p);
#endif
		    if (forward)
			br_forward(dst->dst, skb2);
		    else
			br_deliver(dst->dst, skb2);
		  }
		  dst->dst->dirty = 1;
		  status = 1;
                }
                else if(pip->saddr == dst->src_entry.src.s_addr) {
                  status = 1;
                }
              }
	    }
	}
	if (status) {
	    list_for_each_entry_rcu(p, &br->port_list, list) {
		p->dirty = 0;
	  }
	}

	if ((!forward) && (status))
	kfree_skb(skb);

	return status;
}

static void *snoop_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net_device *dev;
	loff_t offs = 0;

	rtnl_lock();
	for(dev = dev_base; dev; dev = dev->next) {
		if ((dev->priv_flags & IFF_EBRIDGE) &&
                    (*pos == offs)) { 
			return dev;
		}
	}
	++offs;
	return NULL;
}

static void *snoop_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net_device *dev = v;

	++*pos;
	
	for(dev = dev->next; dev; dev = dev->next) {
		if(dev->priv_flags & IFF_EBRIDGE)
			return dev;
	}
	return NULL;
}

static int snoop_seq_show(struct seq_file *seq, void *v)
{
	struct net_device *dev = v;
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	struct net_bridge *br = netdev_priv(dev);

	seq_printf(seq, "bridge	device	group		   reporter          mode  source timeout\n");

	list_for_each_rcu(lh, &br->mc_list) {
		dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
		seq_printf(seq, "%s %6s    ", br->dev->name, dst->dst->dev->name);
		seq_printf(seq, "%02x:%02x:%02x:%02x:%02x:%02x   ", 
			dst->addr.addr[0], dst->addr.addr[1], 
			dst->addr.addr[2], dst->addr.addr[3], 
			dst->addr.addr[4], dst->addr.addr[5]);

		seq_printf(seq, "%02x:%02x:%02x:%02x:%02x:%02x   ", 
			dst->host.addr[0], dst->host.addr[1], 
			dst->host.addr[2], dst->host.addr[3], 
			dst->host.addr[4], dst->host.addr[5]);

		seq_printf(seq, "%2s   %04x   %d\n", 
			(dst->src_entry.filt_mode == MCAST_EXCLUDE) ? 
			"EX" : "IN", dst->src_entry.src.s_addr, 
			(int) (dst->tstamp - jiffies)/HZ);
	}

	return 0;
}

static void snoop_seq_stop(struct seq_file *seq, void *v)
{
	rtnl_unlock();
}

static struct seq_operations snoop_seq_ops = {
	.start = snoop_seq_start,
	.next  = snoop_seq_next,
	.stop  = snoop_seq_stop,
	.show  = snoop_seq_show,
};

static int snoop_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &snoop_seq_ops);
}

static struct file_operations br_igmp_snoop_proc_fops = {
	.owner = THIS_MODULE,
	.open  = snoop_seq_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void br_igmp_snooping_init(void)
{
	br_igmp_entry = create_proc_entry("net/igmp_snooping", 0, NULL);

	if(br_igmp_entry) {
		br_igmp_entry->proc_fops = &br_igmp_snoop_proc_fops;
	}
}

#endif
