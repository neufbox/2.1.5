/*!
 * \file sambactl.h
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
 * $Id: sambactl.h 9487 2008-11-19 08:19:46Z mgo $
 */

#ifndef _LIBNBD_SAMBACTL_H_
#define _LIBNBD_SAMBACTL_H_

#include "plugins/sambactl.h"

#include <string.h>

int nbd_sambactl_start( void );
int nbd_sambactl_stop( void );
int nbd_sambactl_restart( void );
int nbd_sambactl_status( char **buf_xml, size_t * buf_xml_size );
int nbd_sambactl_add_share( const char *name, const char *uuid,
			    const char *dir );
int nbd_sambactl_del_share( unsigned int idx );

#endif
