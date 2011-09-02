/*
 * igmp.h
 *
 * Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
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
 * $Id: igmp.h 11392 2009-03-20 10:40:12Z mgo $
 */

#ifndef LIBNBD_IGMP_H
#define LIBNBD_IGMP_H


int nbd_igmp_setup( void );

int nbd_igmp_stb_add( char const *mac_addr, char const *ipaddr );

int nbd_igmp_stb_del( char const *idx );

int nbd_igmp_stb_list( char *buf, size_t len );

int nbd_igmp_stb_list_count( void );

int nbd_igmp_stb_list_flush( void );


int nbd_igmp_stream_add( char const *ipsrc, char const *ipmc );

int nbd_igmp_stream_del( char const *ipsrc, char const *ipmc );

int nbd_igmp_group_del( char const *ipmc );

#endif /* LIBNBD_IGMP_H */
