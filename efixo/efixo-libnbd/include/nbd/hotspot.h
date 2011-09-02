/*!
 * \file hotspot.h
 *
 * \note Copyright 2007 Arnaud REBILLOUT <arnaud.rebillout@efixo.com>
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
 * $Id: hotspot.h 13167 2009-09-30 12:43:16Z avd $
 */


#ifndef _HOTSPOT_H
#define _HOTSPOT_H

#include <stddef.h>
#include "plugins/hotspot.h"

#include "nbu/list.h"

/*! \fn int nbd_hotspot_start ( void )
    \brief Start hotspot.
    \return: 0: success  -1: failure
 */
int nbd_hotspot_start( void );

/*! \fn int nbd_hotspot_stop ( void )
    \brief Stop hotspot.
    \return: 0: success  -1: failure
 */
int nbd_hotspot_stop( void );

/*! \fn int nbd_hotspot_stop ( void )
    \brief Restart hotspot.
    \return: 0: success  -1: failure
 */
int nbd_hotspot_restart( void );

/*! \fn int nbd_hotspot_client_list( nbu_list_t ** list )
    \brief get client list
    \param nbu_list_t list
    \return: 0: success  -1: failure
 */
int nbd_hotspot_client_list( nbu_list_t ** list );

/*! \fn int nbd_hotspot_ssid_list( char **xml, size_t * xml_size )
    \brief get ssid list
    \return: 0: success  -1: failure
 */
int nbd_hotspot_ssid_list( char **xml, size_t * xml_size );

#endif //_HOTSPOT_H
