/*!
 * \file usharectl.h
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
 * $Id: usharectl.h 9487 2008-11-19 08:19:46Z mgo $
 */

#ifndef _LIBNBD_USHARECTL_H_
#define _LIBNBD_USHARECTL_H_

#include "plugins/usharectl.h"

#include <string.h>

int nbd_usharectl_start( void );
int nbd_usharectl_stop( void );
int nbd_usharectl_restart( void );
int nbd_usharectl_status( char **buf_xml, size_t * buf_xml_size );
int nbd_usharectl_set_mode( const char *mode );
int nbd_usharectl_add_share( const char *uuid, const char *dir );
int nbd_usharectl_del_share( unsigned int idx );

#endif
