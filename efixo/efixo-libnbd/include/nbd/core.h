/*!
 * \file core.h
 *
 * \note Copyright 2006 Miguel GAIO <miguel.gaio@efixo.com>
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
 * $Id: core.h 9487 2008-11-19 08:19:46Z mgo $
 */


#ifndef _CORE_H_
#define _CORE_H_


#include <stddef.h>


#define NBD_CMDLEN      ( 512 )
#define NBD_64K_BUFFER  ( 64 << 10 )


/*! \fn int nbd_open (void)
    \brief Open nbd connection.
    \return: 0: success  -1: failure
 */
int nbd_open( void );


/*! \fn void nbd_close (void)
    \brief Close nbd connection.
    \return: 0
 */
void nbd_close( void );


/*! \fn int nbd_query (char const *plugin, char const *cmd, char const *arg1, char const *arg2, void *out, size_t olen)
    \brief Send command to nbd.
    \param out output data.
    \param olen sizeof output data.
    \param plugin Plugin target id.
    \param ... list of arguments.
    \return: 0: success  -1: failure
 */
int nbd_query( void *out, size_t outlen, char const *plugin, ... );


/*! \fn int nbd_query (char const *plugin, char const *cmd, char const *arg1, char const *arg2, void *out, size_t olen)
    \brief Send command to nbd.
    \param out output data.
    \param olen sizeof output data.
    \param plugin Plugin target id.
    \param ... list of arguments.
    \return: 0: success  -1: failure
 */
int nbd_query_new( char **out, size_t * outlen, char const *plugin, ... );


/*! \def int nbd_set(plugin, ...)
    \brief Send "set" command to nbd.
    \param plugin Plugin target id.
    \param ... list of arguments.
    \return: 0: success  -1: failure
 */
#define nbd_set(plugin, ...)           nbd_query(NULL, 0, plugin, __VA_ARGS__)

/*! \def int nbd_get(out, len, plugin, ...)
    \brief Send "set" command to nbd.
    \param out output data.
    \param olen sizeof output data.
    \param plugin Plugin target id.
    \param ... list of arguments.
    \return: 0: success  -1: failure
 */
#define nbd_get(out, len, plugin, ...) nbd_query(out, len, plugin, __VA_ARGS__)

#endif /* _CORE_H_ */
