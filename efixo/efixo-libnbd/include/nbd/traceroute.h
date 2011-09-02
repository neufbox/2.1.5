/*
 * traceroute.h
 *
 * Copyright 2007 Anthony VIALLARD <anthony.viallard@efixo.com>
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
 * $Id: traceroute.h 9487 2008-11-19 08:19:46Z mgo $
 */

#ifndef LIBNBD_TRACEROUTE_H
#define LIBNBD_TRACEROUTE_H

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"

#include "plugins/traceroute.h"

extern int nbd_traceroute_start( int *id, const char *source,
				 const char *hostname );
extern int nbd_traceroute_stop( int id );
extern int nbd_traceroute_hop_count( int id, int *hops_count );
extern int nbd_traceroute_get( int id, TracerouteResult * traceroute_result );
extern int nbd_traceroute_get_hop( int id,
				   TracerouteHop * traceroute_hop,
				   int hop_index );

#endif
