/*
 *      neufbox/events.h
 *
 *      Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _NEUFBOX_EVENT_H_
#define _NEUFBOX_EVENT_H_

enum event_id {
	event_id_dummy,

	event_id_reset_down,

	event_id_clip_up,
	event_id_clip_down,
	event_id_clip_long,

	event_id_service_up,
	event_id_service_down,
	event_id_service_long,

	event_id_eth0_up,
	event_id_eth0_down,
	event_id_eth1_up,
	event_id_eth1_down,
	event_id_eth10_up,
	event_id_eth10_down,
	event_id_eth11_up,
	event_id_eth11_down,
	event_id_eth12_up,
	event_id_eth12_down,
	event_id_eth13_up,
	event_id_eth13_down,
	event_id_eth14_up,
	event_id_eth14_down,
	event_id_eth2_up,
	event_id_eth2_down,
	event_id_usb0_up,
	event_id_usb0_down,

	event_id_wlan_up,
	event_id_wlan_down,

	event_id_data_up,
	event_id_data_down,

	event_id_voip_up,
	event_id_voip_down,

	event_id_tv_up,
	event_id_tv_down,

	event_id_ses_start,
	event_id_ses_stop,
	event_id_ses_timeout,

	event_id_dhcp_up,
	event_id_dhcp_down,

	event_id_ppp_up,
	event_id_ppp_down,

	event_id_adsl_up,
	event_id_adsl_down,

	event_id_boot_succeeded,
	event_id_boot_failed,

	event_id_last
};

#define LINK_MASK	0x0700
#define HD		0x0000
#define FD		0x0400

#define SP10		0x0000
#define SP100		0x0100
#define SP1000		0x0200

#ifdef __KERNEL__

int event_enqueue(enum event_id id);

#endif				/* __KERNEL__ */

#endif				/* _NEUFBOX_EVENT_H_ */
