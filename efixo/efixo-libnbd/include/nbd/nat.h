/*!
 * \file nat.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 * $Id: nat.h 9648 2008-11-28 10:01:23Z avd $
 */

#ifndef LIBNBD_NAT_H
#define LIBNBD_NAT_H

#include "nbu/list.h"
#include <sys/types.h>

#include "plugins/nat.h"

struct nat_s {
	char rulename[RULENAME_SIZE];
	char proto[PROTO_SIZE];
	char ext_port[EXTPORT_SIZE];
	char dst_ip[DSTIP_SIZE];
	char dst_port[DSTPORT_SIZE];
	char activated[NAT_ACTIVATED_SIZE];
};

extern int nbd_nat_add( struct nat_s *rule );
extern int nbd_nat_del_by_index( unsigned int idx );
extern int nbd_nat_list( char **xml, size_t * xml_size );

extern int nbd_nat_upnp_list( nbu_list_t ** list );
extern unsigned int nbd_nat_upnp_count( void );
extern int nbd_nat_flush( void );
extern int nbd_nat_active( unsigned int idx, const char *activation );

#endif
