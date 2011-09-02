/*!
 * \file dsl.h
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
 * $Id: dsl.h 11739 2009-04-23 09:29:17Z avd $
 */


#ifndef _DSL_H_
#define _DSL_H_


#include <stddef.h>


/*! \fn int nbd_dsl_start (void)
    \brief Start dsl.
    \return: 0: success  -1: failure
 */
int nbd_dsl_start( void );


/*! \fn int nbd_dsl_stop (void)
    \brief Stop dsl.
    \return: 0: success  -1: failure
 */
int nbd_dsl_stop( void );


/*! \fn int nbd_dsl_restart (void)
    \brief Restart dsl.
    \return: 0: success  -1: failure
 */
int nbd_dsl_restart( void );


/*! \fn int nbd_dsl_diag_start (void)
    \brief Start dsl diag daemon.
    \return: 0: success  -1: failure
 */
int nbd_dsl_diag_start( void );


/*! \fn int nbd_dsl_diag_stop (void)
    \brief Stop dsl diag daemon.
    \return: 0: success  -1: failure
 */
int nbd_dsl_diag_stop( void );

/*! \fn int nbd_dsl_get( const char *name, char *buf, size_t buf_size )
    \brief Get dsl info.
    \param variable name
    \param mod Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_get( const char *name, char *buf, size_t buf_size );

/*! \fn int nbd_dsl_mod_set (char const *mod)
    \brief Set mod parameter.
    \param mod parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_mod_set( char const *mod );


/*! \fn int nbd_dsl_mod_get (char *mod, size_t len)
    \brief Get mod parameter.
    \param mod Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_mod_get( char *mod, size_t len );


/*! \fn int nbd_dsl_lpair_set (char const *lpair)
    \brief Set lpair parameter.
    \param lpair parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_lpair_set( char const *lpair );


/*! \fn int nbd_dsl_lpair_get (char *lpair, size_t len)
    \brief Get lpair parameter.
    \param lpair Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_lpair_get( char *lpair, size_t len );


/*! \fn int nbd_dsl_trellis_set (char const *trellis)
    \brief Set trellis parameter.
    \param trellis parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_trellis_set( char const *trellis );


/*! \fn int nbd_dsl_trellis_get (char *trellis, size_t len)
    \brief Get trellis parameter.
    \param trellis Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_trellis_get( char *trellis, size_t len );


/*! \fn int nbd_dsl_snr_set (char const *snr)
    \brief Set snr parameter.
    \param snr parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_snr_set( char const *snr );


/*! \fn int nbd_dsl_snr_get (char *snr, size_t len)
    \brief Get snr parameter.
    \param snr Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_snr_get( char *snr, size_t len );


/*! \fn int nbd_dsl_bitswap_set (char const *bitswap)
    \brief Set bitswap parameter.
    \param bitswap parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_bitswap_set( char const *bitswap );


/*! \fn int nbd_dsl_bitswap_get (char *bitswap, size_t len)
    \brief Get bitswap parameter.
    \param bitswap Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_bitswap_get( char *bitswap, size_t len );


/*! \fn int nbd_dsl_sesdrop_set (char const *sesdrop)
    \brief Set sesdrop parameter.
    \param sesdrop parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_sesdrop_set( char const *sesdrop );


/*! \fn int nbd_dsl_sesdrop_get (char *sesdrop, size_t len)
    \brief Get sesdrop parameter.
    \param sesdrop Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_sesdrop_get( char *sesdrop, size_t len );


/*! \fn int nbd_dsl_sra_set (char const *sra)
    \brief Set sra parameter.
    \param sra parameter.
    \return: 0: success  -1: failure
 */
int nbd_dsl_sra_set( char const *sra );


/*! \fn int nbd_dsl_sra_get (char *sra, size_t len)
    \brief Get sra parameter.
    \param sra Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_sra_get( char *sra, size_t len );


/*! \fn int nbd_dsl_mod_get (char *data, size_t len)
    \brief Get rate up.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_rate_up_get( char *data, size_t len );


/*! \fn int nbd_dsl_rate_down_get (char *data, size_t len)
    \brief Get rate down.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_rate_down_get( char *data, size_t len );


/*! \fn int nbd_dsl_noise_up_get (char *data, size_t len)
    \brief Get noise up.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_noise_up_get( char *data, size_t len );


/*! \fn int nbd_dsl_noise_down_get (char *data, size_t len)
    \brief Get noise down.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_noise_down_get( char *data, size_t len );


/*! \fn int nbd_dsl_attenuation_up_get (char *data, size_t len)
    \brief Get attenuation up.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_attenuation_up_get( char *data, size_t len );


/*! \fn int nbd_dsl_attenuation_down_get (char *data, size_t len)
    \brief Get attenuation down.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_attenuation_down_get( char *data, size_t len );


/*! \fn int nbd_dsl_linemode_get (char *data, size_t len)
    \brief Get linemode.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_linemode_get( char *data, size_t len );

/*! \fn int nbd_dsl_crc_get (char *data, size_t len)
    \brief Get crc.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_crc_get( char *data, size_t len );

/*! \fn int nbd_dsl_plugtest (char **xml, size_t *xml_size)
    \brief Do a dsl plug test.
    \param double pointed data Buffer to write into - allocated in function.
    \param pointed len max size.
    \return: 0: success  -1: failure
 */
int nbd_dsl_plugtest( char **xml, size_t * xml_size );

#endif /* _DSL_H_ */
