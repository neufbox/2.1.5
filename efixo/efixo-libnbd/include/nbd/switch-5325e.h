/*!
 * \file lib/switch-5325e.h
 *
 * \brief  
 *
 * \author Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
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
 * $Id: switch-robo.h 9486 2008-11-18 17:47:21Z mgo $
 */

#ifndef _LIB_SWITCH_5325E_H_
#define _LIB_SWITCH_5325E_H_


#define SWITCH_5325E
#include <kernel/broadcom-5325e-switch.h>


static inline void switch_5325e_arl_read( int fd, unsigned n, struct
					  switch_robo_arl *entry )
{
	entry->u.raw = switch_robo_read64( fd, PAGE_ARL, ROBO_ARL_ENTRY0 + n );
}

static inline void switch_5325e_arl_write( int fd, unsigned n, struct
					   switch_robo_arl *entry,
					   struct ether_addr const *e )
{
	memcpy( entry->u.s.mac_address, e, sizeof( *e ) );
	switch_robo_write64( fd, PAGE_ARL, ROBO_ARL_ENTRY0 + n, entry->u.raw );
}

static inline char *switch_5325e_arl_print( char *buf, size_t n, struct switch_robo_arl const
					    *entry )
{
	snprintf( buf, n, "%016jX", entry->u.raw );

	return buf;
}


#endif /* _LIB_SWITCH_5325E_H_ */
