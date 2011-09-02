/*!
 * \file wlan.h
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
 * $Id: wlan.h 10256 2009-01-08 10:58:08Z avd $
 */

#ifndef _WLAN_H_
#define _WLAN_H_

#include "plugins/wlan.h"

#include <sys/types.h>
#include <stddef.h>

#define WL_HOST_DIAG_BRCM             0x1	/* Running a Broadcom driver */
#define WL_HOST_DIAG_WME              0x2	/* WMM association */
#define WL_HOST_DIAG_ABCAP            0x4	/* Afterburner-capable */
#define WL_HOST_DIAG_AUTHE            0x8	/* Authenticated */
#define WL_HOST_DIAG_ASSOC            0x10	/* Associated */
#define WL_HOST_DIAG_AUTHO            0x20	/* Authorized */
#define WL_HOST_DIAG_WDS              0x40	/* Wireless Distribution System */
#define WL_HOST_DIAG_WDS_LINKUP       0x80	/* WDS traffic/probes flowing properly */
#define WL_HOST_DIAG_PS               0x100	/* HOST_DIAG is in power save mode from AP's viewpoint */
#define WL_HOST_DIAG_APSD_BE          0x200	/* APSD delv/trigger for AC_BE is default enabled */
#define WL_HOST_DIAG_APSD_BK          0x400	/* APSD delv/trigger for AC_BK is default enabled */
#define WL_HOST_DIAG_APSD_VI          0x800	/* APSD delv/trigger for AC_VI is default enabled */
#define WL_HOST_DIAG_APSD_VO          0x1000	/* APSD delv/trigger for AC_VO is default enabled */
#define WL_HOST_DIAG_N_CAP            0x2000	/* 802.11n capable */
#define WL_HOST_DIAG_SCBSTATS         0x4000	/* Per debug stats */

#define WL_HOST_DIAG_EXTRA_INFO 0xff000000

#define WL_HOST_DIAG_RATE_MASK 0x7f
#define WL_HOST_DIAG_RATE_MAGIC 0x80

typedef struct {
	u_int32_t idle;
	u_int32_t in;
	u_int32_t flags;
	u_int32_t tx_pkts;	/* # of packets transmitted */
	u_int32_t tx_failures;	/* # of packets failed */
	u_int32_t rx_ucast_pkts;	/* # of unicast packets received */
	u_int32_t rx_mcast_pkts;	/* # of multicast packets received */
	u_int32_t tx_rate;	/* Rate of last successful tx frame */
	u_int32_t rx_rate;	/* Rate of last successful rx frame */
	u_int32_t rate_count;
	u_int8_t rates[0];
} wl_host_diag_t;

/*! \fn int nbd_wlan_start ( void )
    \brief Start wlan.
    \return: 0: success  -1: failure ?
 */
int nbd_wlan_start( void );


/*! \fn int nbd_wlan_stop ( void )
    \brief Stop wlan.
    \return: 0: success  -1: failure ?
 */
int nbd_wlan_stop( void );


/*! \fn int nbd_wlan_restart ( void )
    \brief Restart wlan.
    \return: 0: success  -1: failure ?
 */
int nbd_wlan_restart( void );


/*! \fn int nbd_wlan_add_mac (char const *mac)
    \brief Add wlan mac.
    \param mac mac address. (mac format xx:xx:xx:xx:xx:xx case insensitive)
    \return: 0: success  -1: failure
 */
int nbd_wlan_add_mac( char const *mac );


/*! \fn int nbd_wlan_add_mac (char const *mac)
    \brief Add wlan mac.
    \param mac mac address. (mac format xx:xx:xx:xx:xx:xx case insensitive)
    \return: 0: success  -1: failure
 */
int nbd_wlan_del_mac( char const *mac );


/*! \fn int nbd_wlan_macfiltering (char const *mode)
    \brief Set wlan mac filtering mode.
    \param mode enable on|off
    \return: 0: success  -1: failure
 */
int nbd_wlan_macfiltering( char const *mode );


/*! \fn int nbd_wlan_macfiltering_mode (char *data, size_t len)
    \brief Get wlan mac filtering mode.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_wlan_macfiltering_mode( char *data, size_t len );


/*! \fn int nbd_wlan_list_mac (char *data, size_t len)
    \brief List wlan filtering mac.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_wlan_list_mac( char *data, size_t len );


/*! \fn int nbd_wlan_list_assoc (char *data, size_t len)
    \brief List wlan assoc mac.
    \param ifname Wlan interface.
    \param data Buffer to write into.
    \param len max size.
    \return: 0: success  -1: failure
 */
int nbd_wlan_list_assoc( char const *ifname, char *data, size_t len );


/*! \fn int nbd_wlan_assoc_count (void)
    \brief Count wlan assoc mac.
    \param ifname Wlan interface.
    \return: 0: success  -1: failure
 */
int nbd_wlan_assoc_count( char const *ifname );


/*! \fn int nbd_wlan_list_contains_assoc (char const *mac)
    \brief list_contains assoc list
    \param mac mac address. (mac format xx:xx:xx:xx:xx:xx case insensitive)
    \return: 0: success  -1: failure
 */
int nbd_wlan_list_contains_assoc( char const *mac );


/*! \fn int nbd_wlan_list_contains_mac (char const *mac)
    \brief list_contains mac filter
    \param mac mac address. (mac format xx:xx:xx:xx:xx:xx case insensitive)
    \return: 0: success  -1: failure
 */
int nbd_wlan_list_contains_mac( char const *mac );


/*! \fn int nbd_wlan_get_current_channel( char *data, size_t len )
    \brief get wlan current channel
    \param data Buffer to write into. len Length of the buffer.
    \return: 0: success  -1: failure
 */
int nbd_wlan_get_current_channel( char *data, size_t len );

/*! \fn int nbd_wlan_diag_host( char **out, int *outlen, const char* mac_addr )
    \brief get wlan diag for an host
    \param data Buffer to write into. len Length of the buffer. Mac addr to diag.
    \return: 0: success  -1: failure
 */
int nbd_wlan_diag_host( char **out, size_t * outlen, const char *mac_addr );

/*! \fn int nbd_wlan_get_diag_host( sta_info_t** sta_info, const char* mac_addr )
    \brief get wlan diag for an host
    \param sta_info_t double pointer. Mac addr to diag.
    \return: 0: success  -1: failure
 */
int nbd_wlan_get_diag_host( wl_host_diag_t ** wl_host_diag,
			    const char *mac_addr );

/*! \fn int nbd_wlan_get_rssi( u_int32_t *val, const char* mac_addr )
    \brief get wlan rssi (signal strength) for an host
    \param u_int32_t pointer to store result and Mac addr.
    \return: 0: success  -1: failure
 */
int nbd_wlan_get_rssi( u_int32_t * val, const char *mac_addr );

/*! \fn int nbd_wlan_get_counters( char ** buf, int *outlen )
    \brief get wlan counters
    \param buf char double pointer and size_t pointer
    \return: 0: success  -1: failure
 */
int nbd_wlan_get_counters( char **buf, size_t * outlen );

#endif /* _WLAN_H_ */
