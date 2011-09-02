/*!
 * \file autoconf.h
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
 * $Id: autoconf.h 9487 2008-11-19 08:19:46Z mgo $
 */


#ifndef _AUTOCONF_H
#define _AUTOCONF_H


#include <stddef.h>

/*! \fn int nbd_autoconf_start ( void )
    \brief Start autoconf.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_start( void );


/*! \fn int nbd_autoconf_stop ( void )
    \brief Stop autoconf.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_stop( void );


/*! \fn int nbd_autoconf_get( char const *key, char *data, size_t len )
    \brief Get an autoconf entry.
    \param key  Entry name.
    \param data Buffer to write into.
    \param len  Max size.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_get( char const *key, char *data, size_t len );

/*! \fn int nbd_autoconf_set( char const *key, char *data )
    \brief Set an autoconf entry.
    \param key  Entry name.
    \param data New value.
    \return: 0: success  1: unchanged -1: failure
 */
int nbd_autoconf_set( char const *key, char const *data );

/*! \fn int nbd_autoconf_attr( char const *key, char const *attr, char *data, size_t len )
    \brief Get an autoconf entry attribute.
    \param key  Entry name.
    \param attr Attribute name.
    \param data Buffer to write into.
    \param len  Max size.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_attr( char const *key, char const *attr, char *data,
		       size_t len );

/*! \fn int nbd_autoconf_list( char const *list, char *data, size_t len);
    \brief Get list of autoconf data.
    \param list Entry name.
    \param data Buffer to write into.
    \param len  Max size.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_list( char const *list, char *data, size_t len );


/*! \fn int nbd_autoconf_list_long( char const *list, char *data, size_t len);
    \brief Get list of autoconf data : format [key=value].
    \param list Entry name.
    \param data Buffer to write into.
    \param len  Max size.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_list_long( char const *list, char *data, size_t len );


/*! \fn int nbd_autoconf_list_count(char const *list );
    \brief Get list element count.
    \param list Entry name.
    \return: element count: success  -1: failure
 */
int nbd_autoconf_list_count( char const *list );


/*! \fn int nbd_autoconf_show(char *data, size_t len);
    \brief Get all  Data.
    \param data Buffer to write into.
    \param len  Max size.
    \return: 0: success  -1: failure
 */
int nbd_autoconf_show( char *data, size_t len );


/*! \fn int nbd_autoconf_append( char * file, char const * key );
    \brief Append a xml file to autoconf xml struct.
    \param key  Where you want to insert the file
    \param file The xml file
    \return: 0: success  -1: failure
 */
int nbd_autoconf_append( char const *key, char const *file );



/*! \fn int nbd_autoconf_matches (char const *key, char const *data)
    \brief Compare a autoconf value to a given string.
    \param key Entry name.
    \param ref String to compare.
    \return: 0: strings are not equal  1: they are equal
 */
#define nbd_autoconf_matches( key, c_str ) \
({ \
	const char __dummy[] = c_str; \
	char __s[sizeof( c_str )]; \
 	\
	(void)(&__dummy); \
	( nbd_autoconf_get( key, __s, sizeof( __s ) ) < 0 ) ? \
		0 : ( memcmp ( __s, c_str, sizeof( c_str ) ) == 0 );	\
})


#endif //_AUTOCONF_H
