/*
 *      nb4/board.h
 *
 *      Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _NEUFBOX_BOARD_H_
#define _NEUFBOX_BOARD_H_

#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/timer.h>

#include "bcm_map_part.h"
#include "bcm_intr.h"

#include "neufbox/leds.h"
#include "box/partition.h"

#else

#include <stdint.h>

#endif				/* __KERNEL__ */

/*! \struct serialization
 *  \brief
 */
struct serialization {
	char pid[20];
	char wpa_key[20];
	unsigned char auth_key[32];
	unsigned char mac_address[6];
	unsigned char dummy[2];
	uint32_t cc;
};

#ifdef __KERNEL__

enum neufbox_resource {
	gpio_clip_button,
	gpio_service_button,
	gpio_reset_button,
	gpio_ses_button,

	gpio_voip_relay,

	irq_reset_button,
	irq_ses_button
};

struct gpio_poll_struct {
	char const *name;
	u32 gpio;
	struct timer_list timer;
	u32 timerid;
	u32 trigger;
	int value;
	void (*fn_up) (void);
	void (*fn_down) (void);
	void (*fn_long) (void);
};

struct board {
	/* serialization data */
	struct serialization serialization;

	/* reference to board resources */
	u16 *resource;

	/* neufbox proc entry */
	struct proc_dir_entry *procfs;

	/* GPIO poll descriptor */
	struct gpio_poll_struct clip_poll;
	struct gpio_poll_struct service_poll;

	/* SES timeout timer */
	struct timer_list ses_timeout;

	int femtocell;
};

extern struct board neufbox_board;

static inline u16 neufbox_femtocell(void)
{
	return !!neufbox_board.femtocell;
}

static inline u16 neufbox_resource(enum neufbox_resource ressource)
{
	return neufbox_board.resource[ressource];
}

#endif				/* __KERNEL__ */

/*! \enum net_infra
 *  \brief Enum net infradescriptor
 */
enum net_infra {
	/*! \brief Net infra adsl */
	NET_INFRA_ADSL,
	/*! \brief Net infra adsl */
	NET_INFRA_FTTH,
	/*! \brief Net infra adsl */
	NET_INFRA_MEDIAFIBRE,
	/*! \brief End of enum */
	NET_INFRA_END
};

#define NET_INFRA_MAGIC 0xE0

/*! \enum rootfs
 *  \brief Enum rootfs descriptor
 */
enum rootfs {
	/*! \brief rootfs flash */
	rootfs_flash,
	/*! \brief rootfs nfs */
	rootfs_nfs,
	/*! \brief rootfs usb */
	rootfs_usb,
	/*! \brief End of enum */
	rootfs_end
};

struct adslphy_tag {
	uint32_t size;
	uint32_t crc;
	char version[32 - 2 * sizeof(uint32_t)];
};

struct boot_tag {
	unsigned char boot_counter;
	unsigned char net_infra;
	char dummy_constant[126];
	unsigned char rootfs;
	char dummy_resettable[127];
};


#ifdef __KERNEL__

static inline void neufbox_pid(char *pid, size_t len)
{
	strncpy(pid, neufbox_board.serialization.pid, len);
}

#endif				/* __KERNEL__ */

#endif				/* _NEUFBOX_BOARD_H_ */
