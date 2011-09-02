#ifndef _BR_IGMP_H
#define _BR_IGMP_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/igmp.h>
#include <linux/in.h>

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)

#define SNOOPING_BLOCKING_MODE 2

union ip_array {
	unsigned int ip_addr;
        unsigned char ip_ar[4];
};


#define TIMER_CHECK_TIMEOUT 10
#define QUERY_TIMEOUT 130
//#define QUERY_TIMEOUT 60


#define IGMPV3_GRP_REC_SIZE(x)  (sizeof(struct igmpv3_grec) + \
                       (sizeof(struct in_addr) * ((struct igmpv3_grec *)x)->grec_nsrcs))

struct net_bridge_mc_src_entry
{
	struct in_addr		src;
	unsigned long		tstamp;
        int			filt_mode;
};

struct net_bridge_mc_fdb_entry
{
	struct net_bridge_port		*dst;
	mac_addr			addr;
	mac_addr			host;
	struct net_bridge_mc_src_entry  src_entry;
	unsigned char			is_local;
	unsigned char			is_static;
	unsigned long			tstamp;
	struct list_head 		list;
};

extern int br_igmp_snooping;
extern int br_igmp_mc_forward(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest,int forward, int clone);
void br_igmp_process_info(struct net_bridge *br, struct sk_buff *skb);
void br_igmp_delbr_cleanup(struct net_bridge *br);

extern int br_igmp_mc_fdb_add(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in_addr *src);
extern void br_igmp_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest);
extern void br_igmp_mc_fdb_cleanup(struct net_bridge *br);
extern int br_igmp_mc_fdb_remove(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host, int mode, struct in_addr *src);
/*
extern struct net_bridge_mc_fdb_entry *br_mc_fdb_find(struct net_bridge *br, 
                                               struct net_bridge_port *prt, 
                                               unsigned char *dest, 
                                               unsigned char *host, 
                                               struct in_addr *src);
void addr_conv(unsigned char *in, char * out);
void brcm_conv_ip_to_mac(char *ipa, char *maca);
*/
void br_igmp_snooping_init(void);
#endif
#endif /* _BR_IGMP_H */
