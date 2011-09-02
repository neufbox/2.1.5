/*!
 * \file backup3g.h
 *
 * \note Copyright 2009 Severin Lageder <severin.lageder@efixo.com>
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
 * $Id: backup3g.h 9487 2008-11-19 08:19:46Z mgo $
 */

#ifndef _BACKUP3G_H_
#define _BACKUP3G_H_

#include <stddef.h>
#include <nbu/list.h>
#include "plugins/backup3g.h"

#if 0
typedef struct {
	long date;
	long duration;
	unsigned long long rx;
	unsigned long long tx;
	int started;
} backup3g_session_t;
#endif

/*! \fn int nbd_backup3g_start ( char* option )
    \brief Start 3G backup
    \param char* option
    \return: 0: success  -1: failure
 */
int nbd_backup3g_start( char *option );

/*! \fn int nbd_backup3g_stop ( char* option )
    \brief Stop 3G backup
    \param char* option
    \return: 0: success  -1: failure
 */
int nbd_backup3g_stop( char *option );

/*! \fn int nbd_backup3g_restart ( void )
    \brief Restart 3G backup
    \return: 0: success  -1: failure
 */
int nbd_backup3g_restart( void );

/*! \fn int nbd_backup3g_getInfo ( void )
    \brief get info about the 3g key state
    \return: 0: success  -1: failure
 */
int nbd_backup3g_getInfo( char **info, size_t * length );

/*! \fn int nbd_backup3g_puk_unlock( char *puk, char *newpin )
    \brief unlock sim card by puk code
    \return: 0: success  -1: failure
 */
int nbd_backup3g_puk_unlock( const char const *puk, const char const *newpin );

/*! \fn int nbd_backup3g_unlock_simprotect( void )
    \brief put simprotect to off
    \return: 0: success  -1: failure
 */
int nbd_backup3g_unlock_simprotect( void );

/*! \fn int nbd_backup3g_init_session ( void )
    \brief start new session statistics
    \return: 0: success  -1: failure
 */
int nbd_backup3g_init_session( void );

/*! \fn int nbd_backup3g_save_stats ( void )
    \brief end session statistics
    \return: 0: success  -1: failure
 */
int nbd_backup3g_end_session( void );

/*! \fn int nbd_backup3g_session_list ( void )
    \brief return session list
    \return: 0: success  -1: failure
 */
int nbd_backup3g_session_list( nbu_list_t ** list );
#endif //_BACKUP3G_H_
