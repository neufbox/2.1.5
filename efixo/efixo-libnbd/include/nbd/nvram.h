/*!
 * \file nvram.h
 *
 * \note Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
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
 * $Id: nvram.h 11972 2009-05-14 15:18:45Z avd $
 */


#ifndef _NVRAM_H_
#define _NVRAM_H_


#include <stddef.h>



/*! \fn int nbd_nv_get (char const *key, char *data, size_t len)
    \brief Get NvRam Data.
    \param key Entry name.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_nv_get( char const *key, char *data, size_t len );

/*! \fn int nbd_nv_set (char const *key, char const *data)
    \brief Add NvRam entry.
    \param key Entry name.
    \param data New value.
    \return: 0: success  1: unchanged -1: failure
 */
int nbd_nv_set( char const *key, char const *data );

/*! \fn int nbd_nv_add (char const *key, char const *data)
    \brief Add NvRam entry.
    \param key Entry name.
    \param data New value.
    \return: 0: success  -1: failure
 */
int nbd_nv_add( char const *key, char const *data );

/*! \fn int nbd_nv_del (char const *key)
    \brief Del NvRam entry.
    \param key Entry name.
    \return: 0: success  -1: failure
 */
int nbd_nv_del( char const *key );

/*! \fn int nbd_nv_list (char const *list, char *data, size_t len)
    \brief Get list of NvRam Data.
    \param list Entry name.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_nv_list( char const *list, char *data, size_t len );

/*! \fn int nbd_nv_list_long (char const *list, char *data, size_t len)
    \brief Get list of NvRam Data: format [key=value].
    \param list Entry name.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_nv_list_long( char const *list, char *data, size_t len );

/*! \fn int nbd_nv_list_add (char const *list, char *data)
    \brief Add element into list. Use this for simple list.
    \param list Entry name.
    \param data New value.
    \return: 0: success  -1: failure
 */
int nbd_nv_list_add( char const *list, char const *data );

/*! \fn int nbd_nv_list_del (char const *list, char const *idx)
    \brief Del element from list.
    \param list Entry name.
    \param idx Index.
    \return: 0: success  -1: failure
 */
int nbd_nv_list_del( char const *list, char const *idx );

/*! \fn int nbd_nv_list_flush (char const *list)
    \brief Flush element from list.
    \param list Entry name.
    \param idx Index.
    \return: 0: success  -1: failure
 */
int nbd_nv_list_flush( char const *list );

/*! \fn int nbd_nv_list_contains (char const *list, char const *key)
    \brief Contains element into.
    \param list List.
    \param key Entry.
    \return: 0: success  -1: failure
 */
int nbd_nv_list_contains( char const *list, char const *key );

/*! \fn int nbd_nv_list_count (char const *list)
    \brief Get list element count.
    \param list Entry name.
    \return: element count: success  -1: failure
 */
int nbd_nv_list_count( char const *list );

/*! \fn int nbd_nv_show (char *data, size_t len)
    \brief Get all NvRam Data.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_nv_show( char *data, size_t len );

/*! \fn int nbd_nv_commit (char const *xml)
    \brief Commit NvRam data.
    \param xml Nvram section.
    \return: 0: success  -1: failure
 */
int nbd_nv_commit( char const *xml );

/*! \fn int nbd_nv_erase (char const *xml)
    \brief Erase NvRam data.
    \param xml Nvram section.
    \return: 0: success  -1: failure
 */
int nbd_nv_erase( char const *xml );


/*! \fn int nbd_nv_export (char const *xml, char const *file)
    \brief Get list of NvRam Data: format [key=value].
    \param xml Nvram section.
    \param file data.
    \return: 0: success  -1: failure
 */
int nbd_nv_export( char const *xml, char const *file );


/*! \fn int nbd_nv_import (char const *xml, char const *file)
    \brief Get list of NvRam Data: format [key=value].
    \param xml Nvram section.
    \param file data.
    \return: 0: success  -1: failure
 */
int nbd_nv_import( char const *xml, char const *file );


/*! \fn char* nbd_nv_xml( char const *key )
 *  \brief Give Xml structure
 *  \param key Entry name.
 */
char *nbd_nv_xml( char const *key );

/*! \fn int nbd_nv_cursor_new( void )
 *  \brief Create a cursor
 *  \return cursor id
 */
unsigned int nbd_nv_cursor_new( void );

/*! \fn int nbd_nv_cursor_close( unsigned int cid )
 *  \brief Close a cursor
 *  \param cid cursor id
 *  \return 0 if success, -1 otherwise
 */
int nbd_nv_cursor_close( unsigned int cid );

/*! \fn int nbd_nv_cursor_revert( unsigned int cid )
 *  \brief Cleanup all modifications do in a cursor
 *  \param cid cursor id
 *  \return 0 if success, -1 otherwise
 */
int nbd_nv_cursor_revert( unsigned int cid );

/*! \fn int nbd_nv_cursor_revert( unsigned int cid )
 *  \brief Commit all modifications do in a cursor and commit nvram data
 *  \param cid cursor id
 *  \return 0 if success, -1 otherwise
 */
int nbd_nv_cursor_commit( unsigned int cid );

/*! \fn int nbd_nv_cset (unsigned int cid, char const *key, char const *data)
    \brief Set NvRam entry in a cursor
 *  \param cid cursor id
    \param key Entry name.
    \param data New value.
    \return: 0: success  1: unchanged -1: failure, -4: invalid format
 */
int nbd_nv_cset( unsigned int cid, char const *key, char const *data );

/*! \fn int nbd_nv_matches (char const *key, char const *data)
    \brief Compare a nvram value to a given string.
    \param key Entry name.
    \param ref String to compare.
    \return: 0: strings are not equal  1: they are equal
 */
#define nbd_nv_matches( key, c_str ) \
({ \
	const char __dummy[] = c_str; \
	char __s[sizeof( c_str )]; \
 	\
	(void)(&__dummy); \
	( nbd_nv_get( key, __s, sizeof( __s ) ) < 0 ) ? \
 		0 : ( memcmp ( __s, c_str, sizeof( c_str ) ) == 0 ); \
})


#endif /* _NVRAM_H_ */
