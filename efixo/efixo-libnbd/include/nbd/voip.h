/*!
 * \file status.h
 *
 * \note Copyright 2008 Pierre-Lucas MORICONI <pierre-lucas.moriconi@efixo.com>
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
 * $Id: voip.h 11969 2009-05-14 14:41:30Z slr $
 */


#ifndef LIBNBD_VOIP_H
#define LIBNBD_VOIP_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "nbu/list.h"

#define NBD_PHONE_CALL_UNINIT 0xFFFF

#define NBD_PHONE_CALL_PSTN 0
#define NBD_PHONE_CALL_VOIP 1
#define NBD_PHONE_CALL_COMPLEX 2
#define NBD_PHONE_CALL_3G   3

#define NBD_PHONE_CALL_INCOMING 0
#define NBD_PHONE_CALL_OUTGOING 1

#define NBD_PHONE_START_CALL 1
#define NBD_PHONE_STOP_CALL 0

#define NB_CALL_MAX 32
#define NUMBER_MAX 32

// PMI
#define NB_MAX_CHAR_INFO 50

struct nbd_phone_call_t {
	unsigned short type;	/* NBD_PHONE_CALL_PSTN or NBD_PHONE_CALL_VOIP */
	unsigned short direction;	/* NBD_PHONE_CALL_INCOMING or NBD_PHONE_CALL_OUTGOING */
	char number[32];	/* phone number of the interlocutor */
	int length;		/* length of the call (in seconds) */
	time_t date;		/* date of the call */
};

/*! \fn int nbd_voip_start ( void )
    \brief Start VOIP
    \return: 0: success  -1: failure
 */
int nbd_voip_start( void );

/*! \fn int nbd_voip_stop ( void )
    \brief Stop VOIP
    \return: 0: success  -1: failure
 */
int nbd_voip_stop( void );

/*! \fn int nbd_voip_start ( void )
    \brief Restart VOIP.
    \return: 0: success  -1: failure
 */
int nbd_voip_restart( void );

/*! \fn int nbd_voip_add (char const *key, char const *data)
    \brief Add NvRam entry.
    \param key Entry name.
    \param data New value.
    \return: 0: success  -1: failure
 */
int nbd_voip_add( char const *key, char const *data );

/*! \fn int nbd_voip_show (char const *list, char *data, size_t len)
    \brief Get list of NvRam Data.
    \param list Entry name.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_voip_show( char const *list, char *data, size_t len );

/*! \fn int nbd_voip_list ()
    \brief Get list of NvRam Data.
    \param list Entry name.
    \param data Buffer to write into.
    \param len max size.
    \return: nbu_list_t*
 */
int nbd_voip_list( nbu_list_t ** list );

/*! \fn int nbd_voip_list_add (char const *list, char *data)
    \brief Add element into list. Use this for simple list.
    \param list Entry name.
    \param data New value.
    \return: 0: success  -1: failure
 */
int nbd_voip_list_add( char const *list, char const *data );

/*! \fn int nbd_voip_list_flush (char const *list)
    \brief Flush all elements from list.
    \param list Entry name.
    \return: 0: success  -1: failure
 */
int nbd_voip_list_flush( char const *list );

/*! \fn int nbd_voip_list_count (char const *list)
    \brief Get list element count.
    \param list Entry name.
    \return: element count: success  -1: failure
 */
int nbd_voip_list_count( char const *list );

/*! \fn int nbd_voip_list_del (char const *list, char const *idx)
    \brief Del element from list.
    \param list Entry name.
    \param idx Index.
    \return: 0: success  -1: failure
 */
int nbd_voip_list_del( char const *list, char const *idx );

/*! \fn int nbd_voip_reg_init (char const *list, char const *idx)
    \brief write/refresh /var/voice/regs.
    \return: 0: success  -1: failure
 */
int nbd_voip_reg_init( void );

/*! \fn int nbd_get_reg_wui (char const *key, char const *data )
    \brief get register value
    \return: 0: success  -1: failure
 */
int nbd_get_reg_wui( char const *key, char const *data, unsigned int *reg );


int nbd_voip_get_line_voltage( int8_t * volt );

/*! \fn int voip_number_call (char* number_called, int num)
    \brief set the number calling or the number called
    \return: 0: success  -1: failure
 */
int nbd_voip_number_call( char *number_called, int num );

/*! \fn int voip_type_call(unsigned short type_call)
    \brief set the type of the call (voip or pstn)
    \return: 0: success  -1: failure
 */
int nbd_voip_type_call( unsigned short type_call );

/*! \fn int voip_io_call(unsigned short io_call)
    \brief set the call to incoming or outgoing
    \return: 0: success  -1: failure
 */
int nbd_voip_io_call( unsigned short io_call );

/*! \fn int voip_duration_call(int start_stop_call)
    \brief set the duration of the call
    \return: 0: success  -1: failure
 */
int nbd_voip_duration_call( int start_stop_call );

/*! \fn int voip_register_call(char* call)
    \brief register the call
    \return: 0: success  -1: failure
 */
int nbd_voip_register_call( char *call );




#endif /* LIBNBD_VOIP_H */
