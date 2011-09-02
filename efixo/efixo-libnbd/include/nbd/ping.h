/*
 * ping.h
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
 * $Id: ping.h 8219 2008-09-16 08:17:00Z mgo $
 */

#ifndef LIBNBD_PING_H
#define LIBNBD_PING_H

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"

#include "plugins/ping.h"

typedef PingStatus nbd_ping_status_t;

extern int nbd_ping_start( int *id, const char *source,
			   const char *hostname, unsigned int count );
extern int nbd_ping_stop( int id );
extern int nbd_ping_status( int id, nbd_ping_status_t * ping_status );

#endif
