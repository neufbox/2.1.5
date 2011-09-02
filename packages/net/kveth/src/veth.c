/*
 *  drivers/net/veth.c
 *
 *  Copyright (C) 2007 OpenVZ http://openvz.org, SWsoft Inc
 *
 * Author: Pavel Emelianov <xemul@openvz.org>
 * Ethtool interface from: Eric W. Biederman <ebiederm@xmission.com>
 *
 */

/*
 * Miguel GAIO <miguel.gaio@efixo.com>
 * Modified from drivers/net/veth.c for neufbox use
 */

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <net/dst.h>
#include <net/xfrm.h>

#define DRV_NAME	"veth"
#define DRV_VERSION	"1.0"
#define PFX DRV_NAME" "DRV_VERSION"::"

struct veth_priv {
	struct net_device *peer;
	struct net_device_stats stats;
};

static struct net_device_stats *veth_get_stats(struct net_device *dev)
{
	struct veth_priv *priv;
	struct net_device_stats *dev_stats;

	priv = netdev_priv(dev);
	dev_stats = &priv->stats;

	return dev_stats;
}

static int veth_open(struct net_device *dev)
{
	struct veth_priv *priv;

	priv = netdev_priv(dev);
	if (priv->peer == NULL)
		return -ENOTCONN;

	netif_start_queue(dev);

	if (priv->peer->flags & IFF_UP) {
		netif_carrier_on(dev);
		netif_carrier_on(priv->peer);
	}
	return 0;
}

static int veth_close(struct net_device *dev)
{
	struct veth_priv *priv = netdev_priv(dev);

	netif_carrier_off(dev);
	netif_carrier_off(priv->peer);

	netif_stop_queue(dev);

	return 0;
}

static void veth_rx(struct sk_buff *skb, struct net_device *dev)
{
	struct veth_priv *priv;
	int len;

	pr_debug(PFX "%s (%s)\n", __func__, dev->name);

	priv = netdev_priv(dev);

	dev->last_rx = jiffies;
	len = skb->len;

	/* skb_orphan(skb); */
	dst_release(skb->dst);
	skb->dst = NULL;
	secpath_reset(skb);
	nf_reset(skb);

	skb->protocol = eth_type_trans(skb, dev);
	skb->dev = dev;
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	netif_receive_skb(skb);
	priv->stats.rx_packets++;
	priv->stats.rx_bytes += len;
}

static int veth_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device *peer;
	struct veth_priv *priv;
	struct ethhdr *eth = eth_hdr(skb);

	pr_debug(PFX "%s (%s)\n", __func__, dev->name);

	priv = netdev_priv(dev);
	peer = priv->peer;

	if (!(peer->flags & IFF_UP))
		goto outf;

	/* drop ipv6 :'( */
	if (skb->protocol == htons(ETH_P_IPV6))
		goto outf;

	/* drop multicast */
	if (is_multicast_ether_addr(eth->h_dest)
	    && !is_broadcast_ether_addr(eth->h_dest))
		goto outf;

	/* drop ppp */
	if ((eth->h_proto == htons(ETH_P_PPP_SES))
	    || (eth->h_proto == htons(ETH_P_PPP_DISC)))
		goto outf;

	dev->trans_start = jiffies;
	priv->stats.tx_bytes += skb->len;
	priv->stats.tx_packets++;

	veth_rx(skb, peer);
	return 0;

 outf:
	dev_kfree_skb(skb);
	priv->stats.tx_dropped++;
	return 0;
}

static void veth_timeout(struct net_device *dev)
{
	pr_debug(PFX "%s (%s)\n", __func__, dev->name);
}

static int veth_poll(struct net_device *dev, int *budget)
{
	pr_debug(PFX "%s (%s)\n", __func__, dev->name);

	netif_rx_complete(dev);

	return 0;
}

static int veth_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	pr_debug(PFX "%s (%s)\n", __func__, dev->name);

	return 0;
}

void veth_setup(struct net_device *dev)
{
	ether_setup(dev);

	/* setup mac_addr */
	dev->dev_addr[0] = 0x02; /* local ether address */
	dev->dev_addr[1] = 0x11;
	dev->dev_addr[2] = 0x22;
	dev->dev_addr[3] = 0x33;
	dev->dev_addr[4] = 0x44;
	dev->dev_addr[5] = 0x55;

	dev->open = veth_open;
	dev->stop = veth_close;
	dev->hard_start_xmit = veth_xmit;
	dev->tx_timeout = veth_timeout;
	dev->get_stats = veth_get_stats;
	dev->do_ioctl = veth_ioctl;
	dev->poll = veth_poll;
	dev->weight = 64;
	dev->flags |= IFF_NOARP;
	dev->flags &= ~IFF_MULTICAST;
	dev->features |= NETIF_F_NO_CSUM;
	dev->hard_header_cache = NULL;	/* Disable caching */
}

static __init int veth_init(void)
{
	struct net_device *dev = NULL, *peer = NULL;
	struct veth_priv *priv, *peer_priv;

	printk(PFX "%s\n", __func__);

	dev = alloc_netdev(sizeof(*priv), "vth%d", veth_setup);
	peer = alloc_netdev(sizeof(*peer_priv), "vth%d", veth_setup);
	if (!(dev && peer)) {
		printk(PFX "%s(%s) %s\n", "alloc_netdev", "vth%d", "failed");
		goto err;
	}

	memcpy(peer->dev_addr, dev->dev_addr, sizeof(peer->dev_addr));
	peer->dev_addr[5] = 0x56;

	priv = netdev_priv(dev);
	priv->peer = peer;

	peer_priv = netdev_priv(peer);
	peer_priv->peer = dev;

	if (!!register_netdev(dev)) {
		printk(PFX "%s(%s) %s\n", "register_netdev", dev->name,
		       "failed");
		goto err;
	}

	if (!!register_netdev(peer)) {
		printk(PFX "%s(%s) %s\n", "register_netdev", peer->name,
		       "failed");
		goto err;
	}

	return 0;

 err:
	if (dev) {
		if (dev->reg_state == NETREG_REGISTERED) {
			unregister_netdev(dev);
		}
		free_netdev(dev);
	}
	if (peer) {
		if (peer->reg_state == NETREG_REGISTERED) {
			unregister_netdev(peer);
		}
		free_netdev(peer);
	}
	printk(PFX "%s %s\n", __func__, "failed");
	return -1;
}

static __exit void veth_exit(void)
{
	struct net_device *dev;

	printk(PFX "%s\n", __func__);
	dev = __dev_get_by_name("vth0");
	if (dev) {
		unregister_netdev(dev);
		free_netdev(dev);
	}
	dev = __dev_get_by_name("vth1");
	if (dev) {
		unregister_netdev(dev);
		free_netdev(dev);
	}
}

module_init(veth_init);
module_exit(veth_exit);

MODULE_DESCRIPTION("Virtual Ethernet Tunnel");
MODULE_LICENSE("GPL v2");
