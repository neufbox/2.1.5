/*
 * ctlan.h
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
 * $Id: ctlan.h 8219 2008-09-16 08:17:00Z mgo $
 */

#ifndef LIBNBD_CTLAN_H
#define LIBNBD_CTLAN_H

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/unistd.h>

#include "core.h"

#define MAC_ALLOCLEN 18
#define IP_ALLOCLEN 16
#define PORT_ALLOCLEN 5

typedef struct s_ctlan_item {
	char mac[MAC_ALLOCLEN];
	char ip[IP_ALLOCLEN];
	char port[PORT_ALLOCLEN];
} ctlan_item_t;

typedef struct s_ctlan_list {
	u_int16_t size;
	ctlan_item_t items[1];
} ctlan_list_t;

extern int nbd_ctlan_getlist( ctlan_list_t * client_list, int size );
extern int nbd_ctlan_getsizelist( int *size );

#endif
