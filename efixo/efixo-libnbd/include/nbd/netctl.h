/*!
 * \file netctl.h
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
 * $Id: netctl.h 9648 2008-11-28 10:01:23Z avd $
 */

#ifndef _LIBNBD_NETCTL_H_
#define _LIBNBD_NETCTL_H_

#include <stdio.h>

int nbd_netctl_list_cmn_privateport( char **xml, size_t * xml_len );
int nbd_netctl_list_lan_privateport( char **xml, size_t * size );

#endif
