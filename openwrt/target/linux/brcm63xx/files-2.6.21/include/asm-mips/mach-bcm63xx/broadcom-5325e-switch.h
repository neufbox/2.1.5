
#ifndef _BROADCOM_5325E_SWITCH_H_
#define _BROADCOM_5325E_SWITCH_H_

#define SWITCH_MEDIA_AUTO	(1<<0)
#define SWITCH_MEDIA_100	(1<<1)
#define SWITCH_MEDIA_1000	(1<<2)
#define SWITCH_MEDIA_FD		(1<<3)

union robo_reg_union {
	__be16 raw[4];
	__be16 u16;
	__be32 u32;
	__be64 u64;
};

struct robo_cmd {
	union robo_reg_union u;
	__u8 page;
	__u8 reg;
	__u8 len;
};

enum {
	SIOCGROBOREGS = SIOCDEVPRIVATE + 14,
	SIOCSROBOREGS
};

#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/if.h>

int switch_print_media(char *buf, int media);

void switch_robo_monitor_ports(struct net_device *);
int switch_robo_ioctl(struct net_device *, struct ifreq *, int);

#else
#include <linux/sockios.h>
#endif				/* __KERNEL__ */

/****************************************************************************
  External switch pseudo PHY: Page (0x00)
 ****************************************************************************/

#define PAGE_CONTROL						0x00

	#define ROBO_PORT_CONTROL0				0x00
	#define ROBO_PORT_CONTROL1				0x01
	#define ROBO_PORT_CONTROL2				0x02
	#define ROBO_PORT_CONTROL3				0x03
	#define ROBO_PORT_CONTROL4				0x04
		#define STP_NO_SPANNING_TREE			0
		#define STP_DISABLED_STATE			1
		#define STP_BLOCKING_STATE			2
		#define STP_LISTENING_STATE			3
		#define STP_LEARNING_STATE			4
		#define STP_FORWARDING_STATE			5

	#define ROBO_MII_PORT_CONTROL				0x08
		#define ROBO_MII_PORT_CONTROL_RX_UCST_EN	0x10
		#define ROBO_MII_PORT_CONTROL_RX_MCST_EN	0x08
		#define ROBO_MII_PORT_CONTROL_RX_BCST_EN	0x04

	#define ROBO_SWITCH_MODE				0x0b
		#define SWITCH_FORWARDING_DISABLE		0x00
		#define SWITCH_FORWARDING_ENABLE		0x01
		#define SWITCH_MODE_UNMANAGED			0x00
		#define SWITCH_MODE_MANAGED			0x01

	#define ROBO_SWITCH_MODE_FRAME_MANAGE_MODE		0x01
	#define ROBO_SWITCH_MODE_SW_FWDG_EN			0x02

	#define ROBO_CONTROL_MII1_PORT_STATE_OVERRIDE		0x0e
		#define ROBO_CONTROL_MPSO_MII_SW_OVERRIDE	0x80
		#define ROBO_CONTROL_MPSO_REVERSE_MII		0x10
		#define ROBO_CONTROL_MPSO_LP_FLOW_CONTROL	0x08
		#define ROBO_CONTROL_MPSO_SPEED100		0x04
		#define ROBO_CONTROL_MPSO_FDX			0x02
		#define ROBO_CONTROL_MPSO_LINKPASS		0x01

	#define ROBO_CONTROL_MULTICAST_IP_CONTROL		0x21
		#define ROBO_CONTROL_IP_MULTICAST		(1<<0)

	#define ROBO_CONTROL_MPSO_LP_FLOW_CONTROL_5397		0x30
	#define ROBO_CONTROL_MPSO_SPEED1000_5397		0x08

	#define ROBO_CONTROL_MPSO_OVERRIDE_ALL ( \
				ROBO_CONTROL_MPSO_MII_SW_OVERRIDE | \
				ROBO_CONTROL_MPSO_REVERSE_MII | \
				ROBO_CONTROL_MPSO_LP_FLOW_CONTROL | \
				ROBO_CONTROL_MPSO_SPEED100 | \
				ROBO_CONTROL_MPSO_FDX | \
				ROBO_CONTROL_MPSO_LINKPASS \
				)

	#define ROBO_CONTROL_MPSO_OVERRIDE_ALL_5397 ( \
				ROBO_CONTROL_MPSO_MII_SW_OVERRIDE | \
				ROBO_CONTROL_MPSO_LP_FLOW_CONTROL_5397 | \
				ROBO_CONTROL_MPSO_SPEED100 | \
				ROBO_CONTROL_MPSO_FDX | \
				ROBO_CONTROL_MPSO_LINKPASS \
				)

	#define ROBO_POWER_DOWN_MODE				0x0f
		#define ROBO_POWER_DOWN_MODE_PORT1_PHY_DISABLE	0x01
		#define ROBO_POWER_DOWN_MODE_PORT2_PHY_DISABLE	0x02
		#define ROBO_POWER_DOWN_MODE_PORT3_PHY_DISABLE	0x04
		#define ROBO_POWER_DOWN_MODE_PORT4_PHY_DISABLE	0x08
		#define ROBO_POWER_DOWN_MODE_PORT5_PHY_DISABLE	0x10
		#define ROBO_POWER_DOWN_MODE_ALL		0x1f

	#define ROBO_DISABLE_LEARNING				0x3c

union robo_control {
	__u8 u8;
	struct robo_control_register_s {
		__u8 stp_state:3;
		__u8 reserved_4_2:3;
		__u8 TX_disable:1;
		__u8 RX_disable:1;
	} s;
};
union robo_mii_control {
	__u8 u8;
	struct robo_mii_control_register_s {
		__u8 reserved_7_5:3;
		__u8 RX_unicast:1;
		__u8 RX_multicast:1;
		__u8 RX_broadcast:1;
		__u8 reserved_1_0:2;
	} s;
};

union robo_mode {
	__u8 u8;
	struct robo_mode_register_s {
		__u8 reserved_7_2:6;
		__u8 switch_forwarding:1;
		__u8 switch_mode:1;
	} s;
};

/****************************************************************************
  External status registers PHY: Page (0x01)
 ****************************************************************************/

#define PAGE_STATUS						0x01

	#define ROBO_LINK_STATUS				0x00
		#define LINK_STATUS_FAIL			0x00
		#define LINK_STATUS_PASS			0x01

	#define ROBO_LINK_STATUS_CHANGE				0x02
		#define LINK_STATUS_CONSTANT			0x00
		#define LINK_STATUS_CHANGE			0x01

	#define ROBO_PORT_SPEED_SUMMARY				0x04
		#define PORT_SPEED_10MBPS			0x00
		#define PORT_SPEED_100MBPS			0x01
		#define PORT_SPEED_1000MBPS			0x02
		#define PORT_SPEED_ILLEGAL_STATE		0x03

	#define ROBO_DUPLEX_STATUS_SUMMARY			0x06
		#define DUPLEX_STATUS_HALF_DUPLEX		0x00
		#define DUPLEX_STATUS_FULL_DUPLEX		0x01

	#define ROBO_PAUSE_STATUS_SUMMARY			0x0A
	#define ROBO_SOURCE_ADDRESS_CHANGE			0x0E
	#define ROBO_LAST_SOURCE_ADDRESS_PORT0			0x10
	#define ROBO_LAST_SOURCE_ADDRESS_PORT1			0x16
	#define ROBO_LAST_SOURCE_ADDRESS_PORT2			0x1C
	#define ROBO_LAST_SOURCE_ADDRESS_PORT3			0x22
	#define ROBO_LAST_SOURCE_ADDRESS_PORT4			0x28
	#define ROBO_LAST_SOURCE_ADDRESS_IMP_PORT		0x40

union robo_link_status {
	__u16 u16;
	struct robo_link_status_register_s {
		__u16 reserved_9_15:7;
		__u16 IMP_port:1;
		__u16 reserved_5_7:3;
		__u16 port4:1;
		__u16 port3:1;
		__u16 port2:1;
		__u16 port1:1;
		__u16 port0:1;
	} s;
};

union robo_link_speed {
	__u16 u16;
	struct robo_link_speed_register_s {
		__u32 reserved_15_9:7;
		__u32 IMP_port:1;
		__u32 reserved_7_5:3;
		__u32 port4:1;
		__u32 port3:1;
		__u32 port2:1;
		__u32 port1:1;
		__u32 port0:1;

	} s;
};

union robo_link_duplex {
	__u16 u16;
	struct robo_link_duplex_register_s {
		__u16 reserved_9_15:7;
		__u16 IMP_port:1;
		__u16 reserved_5_7:3;
		__u16 port4:1;
		__u16 port3:1;
		__u16 port2:1;
		__u16 port1:1;
		__u16 port0:1;
	} s;
};

/****************************************************************************
  External switch pseudo PHY: Page (0x02)
 ****************************************************************************/

#define PAGE_MANAGEMENT						0x02
	#define ROBO_GLOBAL_CONFIGURATION			0x00
		#define NO_FRAME_MANAGEMENT			0x00
		#define ENABLE_MII_PORT				0x80
		#define RECEIVE_IGMP				0x08
		#define RECEIVE_BPDU				0x02

union robo_global_configuration {
	__u8 u8;
	struct robo_global_configuration_register_s {
		__u8 enable_IMP_port:2;
		__u8 reserved_5:1;
		__u8 interrupt_enable:1;
		__u8 reserved_3_2:2;
		__u8 RX_BDPU_enable:1;
		__u8 reset_MIB:1;
	} s;
};

	#define ROBO_DEV_ID					0x30

	#define ROBO_PROTOCOL_CONTROL				0x50

union robo_protocol_control {
	__u16 u16;
	struct robo_protocol_control_register_s {
		__u16 reserved_15_9:7;
		__u16 igmp_fwd_mode:1;
		__u16 igmp_dip_en:1;
		__u16 igmp_ip_en:1;
		__u16 icmpv6_fwd_mode:1;
		__u16 icmpv6_en:1;
		__u16 icmpv4_en:1;
		__u16 dhcp_en:1;
		__u16 rarp_en:1;
		__u16 arp_en:1;
	} s;
};


/****************************************************************************
  External switch pseudo PHY: Page (0x04)
 ****************************************************************************/

#define PAGE_ARL_CONTROL					0x04

	#define ROBO_ARL_CONTROL_CONTROL			0x00

	#define ROBO_ARL_ADDR0					0x10
	#define ROBO_ARL_VEC0					0x16
	#define ROBO_ARL_ADDR1					0x20
	#define ROBO_ARL_VEC1					0x26


/****************************************************************************
  External switch pseudo PHY: Page (0x05)
 ****************************************************************************/

#define PAGE_ARL						0x05
	#define ROBO_ARL_CONTROL				0x00
		#define ROBO_ARL_CONTROL_READ			0x01
		#define ROBO_ARL_CONTROL_START			0x80
		#define ROBO_ARL_CONTROL_DONE			0x80

	#define ROBO_ARL_ADDR_INDEX				0x02

	#define ROBO_ARL_VID_TABLE_INDEX			0x08

	#define ROBO_ARL_ENTRY0					0x10
	#define ROBO_ARL_ENTRY1					0x18

	#define ROBO_ARL_CPU_PORT				( 3 << 5 )

union switch_robo_arl_table_control {
	__u8 u8;
	struct switch_robo_arl_table_control_s {
		__u8 start_done:1;
		__u8 reserved_6_1:6;
		__u8 read:1;
	} s;
};

union switch_robo_arl_union {
	__u64 raw;
	struct switch_robo_arl_union_s {
		__u64 valid:1;
		__u64 _static:1;
		__u64 age:1;
		__u64 reserved_60_55:6;
		__u64 portid:7;
		__u8 mac_address[6];
	} s;
} __attribute__ ((packed));

#define ROBO_ARL_ENTRY_COUNT 2
struct switch_robo_arl {
	union switch_robo_arl_union u;
} __attribute__ ((packed));

/****************************************************************************
    PORT MIB REGISTER: Page (0x20 -> 0x24)
****************************************************************************/

#define PAGE_PORT0_MIB						0x20
#define PAGE_PORT1_MIB						0x21
#define PAGE_PORT2_MIB						0x22
#define PAGE_PORT3_MIB						0x23
#define PAGE_PORT4_MIB						0x24
#define PAGE_PORTIMP_MIB					0x28
/* mode 0 */
        #define MIB_TX_PACKETS                                  0x00
	#define MIB_TX_UNICAST_PACKETS				0x02
        #define MIB_RX_GOOD_PACKETS                             0x04
        #define MIB_RX_UNICAST_PACKETS                          0x06

/* mode 1 */
	#define MIB_TX_COLLISIONS				0x00
	#define MIB_TX_OCTETS					0x02
	#define MIB_RX_FCS_ERRORS				0x04
	#define MIB_RX_GOOD_OCTETS				0x06

/****************************************************************************
QOS: Page (0x30)
 ****************************************************************************/

#define PAGE_PORT_QOS						0x30

	#define ROBO_QOS_CONTROL				0x00
		#define ROBO_QOS_CPU_CTRL_ENABLE		0x8000
		#define ROBO_QOS_TXQ_MODE_MULT			0x0400
	#define ROBO_QOS_PQ_CONTROL				0x02
		#define ROBO_QOS_LAYER_SEL			0x04
	#define ROBO_QOS_8021P_ENABLE				0x04
	#define ROBO_QOS_DIFFSERV_ENABLE			0x06
	#define ROBO_QOS_PAUSE_ENABLE				0x13
	#define ROBO_QOS_PRIO_THRESHOLD				0x15
	#define ROBO_QOS_DSCP_PRIO00				0x30

#define QOS_DSCP_PRIO_REG_SIZE					8
#define QOS_DSCP_PRIO_REG_COUNT					2

#define DSCP_COUNT						64
#define QOS_DSCP_PRIO_LOG2 \
	( ( ( QOS_DSCP_PRIO_REG_SIZE * QOS_DSCP_PRIO_REG_COUNT * \
	      8UL /* bit per byte */ ) / DSCP_COUNT ) - 1UL )

#define QOS_PRIO_DSCP_MASK ( ( 1UL << (QOS_DSCP_PRIO_LOG2 + 1UL ) ) - 1UL )

#define QOS_PRIO_DSCP_REG(dscp) \
	( ( ( ( dscp << QOS_DSCP_PRIO_LOG2 ) / DSCP_COUNT ) \
	    * QOS_DSCP_PRIO_REG_SIZE ) + ROBO_QOS_DSCP_PRIO00 )

#define QOS_PRIO_DSCP_SHIFT(dscp) \
	( ( dscp & ( ( DSCP_COUNT >> QOS_DSCP_PRIO_LOG2 ) - 1UL ) ) \
	  * ( QOS_DSCP_PRIO_LOG2 + 1UL ) )


/****************************************************************************
  External switch pseudo PHY: Page (0x31)
 ****************************************************************************/

#define PAGE_PORT_BASED_VLAN					0x31

/****************************************************************************
  External switch pseudo PHY: Page (0xff)
 ****************************************************************************/

#define ROBO_PAGE_SELECT					0xff

/****************************************************************************
  External switch pseudo PHY: Page (0x34)
 ****************************************************************************/

#define VLAN_ID_MAX5350                                         15

#define PAGE_VLAN						0x34

	#define ROBO_VLAN_CTRL0					0x00
		#define ROBO_VLAN_CTRL0_ENABLE_1Q		(1 << 7)
		#define ROBO_VLAN_CTRL0_SVLM			(0 << 5)
		#define ROBO_VLAN_CTRL0_IVLM			(3 << 5)
		#define ROBO_VLAN_CTRL0_FR_CTRL_CHG_PRI		(1 << 2)
		#define ROBO_VLAN_CTRL0_FR_CTRL_CHG_VID		(2 << 2)
	#define ROBO_VLAN_CTRL1					0x01
	#define ROBO_VLAN_CTRL2					0x02
	#define ROBO_VLAN_CTRL3					0x03
		#define ROBO_VLAN_CTRL3_8BIT_CHECK		(1 << 7)
		#define ROBO_VLAN_CTRL3_MAXSIZE_1532		(1 << 6)
		#define ROBO_VLAN_CTRL3_MII_DROP_NON_1Q		(0 << 5)
		#define ROBO_VLAN_CTRL3_DROP_NON_1Q_SHIFT	0
	#define ROBO_VLAN_CTRL4				        0x04
	#define ROBO_VLAN_CTRL5					0x05
		#define ROBO_VLAN_CTRL5_VID_HIGH_8BIT_NOT_CHECKED (1 << 5)
		#define ROBO_VLAN_CTRL5_APPLY_BYPASS_VLAN	(1 << 4)
		#define ROBO_VLAN_CTRL5_DROP_VTAB_MISS		(1 << 3)
		#define ROBO_VLAN_CTRL5_ENBL_MANAGE_RX_BYPASS	(1 << 1)
		#define ROBO_VLAN_CTRL5_ENBL_CRC_GEN		(1 << 0)
	#define ROBO_VLAN_ACCESS					0x06
		#define ROBO_VLAN_ACCESS_START_DONE		(1 << 13)
		#define ROBO_VLAN_ACCESS_WRITE_STATE		(1 << 12)
		#define ROBO_VLAN_ACCESS_HIGH8_VID_SHIFT		4
		#define ROBO_VALN_ACCESS_LOW4_VID_SHIFT		0
	#define ROBO_VLAN_WRITE					0x08
		#define ROBO_VLAN_WRITE_VALID			(1 << 20)
		#define ROBO_VLAN_HIGH_8BIT_VID_SHIFT		12
		#define ROBO_VLAN_UNTAG_SHIFT			6
		#define ROBO_VLAN_GROUP_SHIFT			0
		#define MII_PORT_FOR_VLAN			5
	#define ROBO_VLAN_READ					0x0C
		#define ROBO_VLAN_READ_VALID			(1 << 20)
	#define ROBO_VLAN_PTAG0					0x10
	#define ROBO_VLAN_PTAG1					0x12
	#define ROBO_VLAN_PTAG2					0x14
	#define ROBO_VLAN_PTAG3					0x16
	#define ROBO_VLAN_PTAG4					0x18
	#define ROBO_VLAN_PTAG5					0x1A
	#define ROBO_VLAN_PMAP					0x20

/****************************************************************************
  Registers for pseudo PHY access
 ****************************************************************************/

			#define ROBO_MII_PAGE			0x10
				#define ROBO_MII_PAGE_SHIFT	8
				#define ROBO_MII_PAGE_ENABLE	0x01

			#define ROBO_MII_ADDR			0x11
				#define ROBO_MII_ADDR_SHIFT	8
				#define ROBO_MII_OP_DONE	0x00
				#define ROBO_MII_ADDR_WRITE	0x01
				#define ROBO_MII_ADDR_READ	0x02

			#define ROBO_MII_DATA0			0x18

#endif				/* _BROADCOM_5395S_SWITCH_H_ */
