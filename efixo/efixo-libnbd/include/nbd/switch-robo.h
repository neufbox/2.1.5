/*!
 * \file lib/switch-robo.h
 *
 * \brief  
 *
 * \author Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
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
 * $Id: switch-robo.h 10906 2009-02-19 09:12:47Z mgo $
 */

#if defined(SWITCH_ROBO_5325E) || defined(SWITCH_ROBO_5395S)
#ifndef _LIB_SWITCH_ROBO_H_
#define _LIB_SWITCH_ROBO_H_


#include <stdint.h>
#include <sys/types.h>

/* LINUX */
typedef uint64_t __u64;
typedef uint64_t __be64;
#include <net/if.h>
#include <netinet/ether.h>



/*! \fn void switch_robo_open( char const *ifname )
 *  \brief Always use this fonction before using read or write
 *  \param ifname Robo Switch netdevice name
 */
int switch_robo_open( char const *ifname );

/*! \fn void switch_robo_close( void )
 *  \brief Use this function when you finish using read and write.
 */
void switch_robo_close( int fd );

/*! \fn uint16_t switch_robo_read16( uint8_t page, uint8_t reg )
 *  \brief Read a 16 bits register in  memory
 *  \param page Page adress
 *  \param reg  Register adress
 */
uint8_t switch_robo_read8( int fd, uint8_t page, uint8_t reg );

/*! \fn uint16_t switch_robo_read16( uint8_t page, uint8_t reg )
 *  \brief Read a 16 bits register in  memory
 *  \param page Page adress
 *  \param reg  Register adress
 */
uint16_t switch_robo_read16( int fd, uint8_t page, uint8_t reg );

/*! \fn uint32_t switch_robo_read32( uint8_t page, uint8_t reg )
 *  \brief Read a 32 bits register in  memory
 *  \param page Page adress
 *  \param reg  Register adress
 */
uint32_t switch_robo_read32( int fd, uint8_t page, uint8_t reg );

/*! \fn void switch_robo_read48( uint8_t page, uint8_t reg, void *v )
 *  \brief Read a 48 bits register in  memory
 *  \param page Page adress
 *  \param reg  Register adress
 */
void switch_robo_read48( int fd, uint8_t page, uint8_t reg, void * );

/*! \fn uint32_t switch_robo_read64( uint8_t page, uint8_t reg )
 *  \brief Read a 32 bits register in  memory
 *  \param page Page adress
 *  \param reg  Register adress
 */
uint64_t switch_robo_read64( int fd, uint8_t page, uint8_t reg );


/*! \fn void switch_robo_write8( uint8_t page, uint8_t reg, uint8_t val8 )
 *  \brief Write a 16 bits register in memory
 *  \param page  Page adress
 *  \param reg   Register adress
 *  \param val16 Value to write
 */
void switch_robo_write8( int fd, uint8_t page, uint8_t reg, uint8_t val8 );

/*! \fn void switch_robo_write16( uint8_t page, uint8_t reg, uint16_t val16 )
 *  \brief Write a 16 bits register in memory
 *  \param page  Page adress
 *  \param reg   Register adress
 *  \param val16 Value to write
 */
void switch_robo_write16( int fd, uint8_t page, uint8_t reg, uint16_t val16 );

/*! \fn void switch_robo_write32( uint8_t page, uint8_t reg, uint32_t val32 )
 *  \brief Write a 32 bits register in memory
 *  \param page  Page adress
 *  \param reg   Register adress
 *  \param val32 Value to write
 */
void switch_robo_write32( int fd, uint8_t page, uint8_t reg, uint32_t val32 );

/*! \fn void switch_robo_write48( uint8_t page, uint8_t reg, void const *val48 )
 *  \brief Write a 32 bits register in memory
 *  \param page  Page adress
 *  \param reg   Register adress
 *  \param val32 Value to write
 */
void switch_robo_write48( int fd, uint8_t page, uint8_t reg,
			  void const *val48 );

/*! \fn void switch_robo_write64( uint8_t page, uint8_t reg, void const *val48 )
 *  \brief Write a 32 bits register in memory
 *  \param page  Page adress
 *  \param reg   Register adress
 *  \param val32 Value to write
 */
void switch_robo_write64( int fd, uint8_t page, uint8_t reg, uint64_t val64 );



/* Efixo */
#ifdef SWITCH_ROBO_5325E
#ifdef LIBNBD
#include <switch-5325e.h>
#else
#include <nbd/switch-5325e.h>
#endif /* LIBNBD */
#endif /* SWITCH_ROBO_5325E */
#ifdef SWITCH_ROBO_5395S
#ifdef LIBNBD
#include <switch-5395s.h>
#else
#include <nbd/switch-5395s.h>
#endif /* LIBNBD */
#endif /* SWITCH_ROBO_5395S */



static inline void switch_robo_arl_start( int fd, struct ether_addr const *e )
{
	/* mac address: index register */
	switch_robo_write48( fd, PAGE_ARL, ROBO_ARL_ADDR_INDEX, e );

	/* read access */
	switch_robo_write8( fd, PAGE_ARL, ROBO_ARL_CONTROL,
			    ROBO_ARL_CONTROL_READ | ROBO_ARL_CONTROL_START );
}

static inline void switch_robo_arl_commit( int fd )
{
	/* commit */
	switch_robo_write8( fd, PAGE_ARL, ROBO_ARL_CONTROL,
			    ROBO_ARL_CONTROL_START );
}

static inline void switch_robo_arl_read( int fd, unsigned n, struct switch_robo_arl
					 *entry )
{
#ifdef SWITCH_ROBO_5325E
	switch_5325e_arl_read( fd, n, entry );
#endif /* SWITCH_ROBO_5325E */
#ifdef SWITCH_ROBO_5395S
	switch_5395s_arl_read( fd, n, entry );
#endif /* SWITCH_ROBO_5395S */
}

static inline void switch_robo_arl_write( int fd, unsigned n, struct switch_robo_arl
					  *entry, struct ether_addr const *e )
{
#ifdef SWITCH_ROBO_5325E
	switch_5325e_arl_write( fd, n, entry, e );
#endif /* SWITCH_ROBO_5325E */
#ifdef SWITCH_ROBO_5395S
	switch_5395s_arl_write( fd, n, entry, e );
#endif /* SWITCH_ROBO_5395S */
}

static inline char *switch_robo_arl_print( char *buf, size_t n,
					   struct switch_robo_arl const *entry )
{
#ifdef SWITCH_ROBO_5325E
	return switch_5325e_arl_print( buf, n, entry );
#endif /** SWITCH_ROBO_5325E */
#ifdef SWITCH_ROBO_5395S
	return switch_5395s_arl_print( buf, n, entry );
#endif /** SWITCH_ROBO_5395S */
}

static inline int switch_robo_arl_valid( struct switch_robo_arl const
					 *entry )
{
	return ( entry->u.s.valid );
}

static inline int switch_robo_arl_multicast( struct switch_robo_arl const
					     *entry )
{
	return ( entry->u.s.mac_address[0] == 0x01 );
}

static inline int switch_robo_arl_matches( struct switch_robo_arl const
					   *entry, struct ether_addr const *e )
{
	return !memcmp( entry->u.s.mac_address, e, sizeof( *e ) );
}

/*! \fn int switch_robo_arl_lookup( int fd, struct ether_addr const *e,
 *                                  struct switch_robo_arl *entry )
 */
int switch_robo_arl_lookup( int fd, struct ether_addr const *e,
			    struct switch_robo_arl *entry );

/*! \fn switch_robo_arl_setup( int fd, struct ether_addr const *e,
 *                             struct switch_robo_arl const *entry )
 */
int switch_robo_arl_setup( int fd, struct ether_addr const *e,
			   struct switch_robo_arl const *entry );


/*! \fn switch_robo_mc_add_port( int fd, struct ether_addr const *e,
 *                    uint32_t port )
 *  \brief Add port into multi cast rule
 *  \param e	Ethernet Address
 *  \param port	Port
 */
int switch_robo_mc_add_port( struct ether_addr const *e, uint32_t port );

/*! \fn switch_robo_mc_del_port( struct ether_addr const *e, uint32_t port )
 *  \brief Delete port into multi cast rule
 *  \param e	Ethernet Address
 *  \param port	Port
 */
int switch_robo_mc_del_port( struct ether_addr const *e, uint32_t port );

#define for_each_qos_map_reg(reg) \
	for ( reg = ROBO_QOS_DSCP_PRIO00; reg < \
	      QOS_DSCP_PRIO_REG_SIZE * QOS_DSCP_PRIO_REG_COUNT + \
	      ROBO_QOS_DSCP_PRIO00; reg += QOS_DSCP_PRIO_REG_SIZE )

static inline uint64_t switch_robo_qos_map_read( int fd, unsigned reg )
{
	uint64_t v64;

#ifdef SWITCH_ROBO_5325E
	v64 = switch_robo_read64( fd, PAGE_PORT_QOS, reg );
#elif defined (SWITCH_ROBO_5395S)
	switch_robo_read48( fd, PAGE_PORT_QOS, reg, &v64 );
	v64 >>= 16;	/* big endian cast 48bits -> 64bits */
#endif /* SWITCH_ROBO_5395S */

	return v64;
}

static inline void switch_robo_qos_map_write( int fd, unsigned reg,
					      uint64_t v64 )
{
#ifdef SWITCH_ROBO_5325E
	switch_robo_write64( fd, PAGE_PORT_QOS, reg, v64 );
#elif defined (SWITCH_ROBO_5395S)
	v64 <<= 16;	/* big endian cast 64bits -> 48bits */
	switch_robo_write48( fd, PAGE_PORT_QOS, reg, &v64 );
#endif /* SWITCH_ROBO_5395S */
}

unsigned switch_robo_qos_get_dscp_queue( int fd, unsigned long dscp );
void switch_robo_qos_set_dscp_queue( int fd, unsigned long dscp,
				     unsigned long queue );

#endif /* _LIB_SWITCH_ROBO_H_ */
#endif /* defined(SWITCH_ROBO_5325E) || defined(SWITCH_ROBO_5395S) */
