/*
<:copyright-gpl
 Copyright 2008 Broadcom Corp. All Rights Reserved.

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
//  Filename:       ip_conntrack_esp.h
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the ESP ALG connectiontracking data structures.
//
/*****************************************************************************/
#ifndef _IP_CONNTRACK_ESP_H
#define _IP_CONNTRACK_ESP_H
/* FTP tracking. */

#ifndef __KERNEL__
#error Only in kernel.
#endif

#include <linux/netfilter_ipv4/lockhelp.h>

/* Protects ftp part of conntracks */
DECLARE_LOCK_EXTERN(ip_esp_lock);

struct esphdr {
	u_int32_t spi;
	u_int32_t seq;
};

/* This structure is per expected connection */
struct ip_ct_esp_expect
{
	/* We record spi and source IP address: all in
	 * host order. */

	u_int32_t spi;	   /* Security Parameter Identifier */
	u_int32_t saddr;   /* source IP address in the orig dir */
	u_int32_t daddr;   /* remote IP address in the orig dir */
};

/* This structure exists only once per master */
struct ip_ct_esp_master {
	u_int32_t spi;
	u_int32_t saddr;
	u_int32_t daddr;
};

#endif /* _IP_CONNTRACK_ESP_H */
