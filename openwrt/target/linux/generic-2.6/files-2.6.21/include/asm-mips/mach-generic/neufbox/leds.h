/*
 *      neufbox/leds.h
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

#ifndef _NEUFBOX_LEDS_H_
#define _NEUFBOX_LEDS_H_

enum led_mode {
	led_mode_disable,
	led_mode_control,
	led_mode_downloading,
	led_mode_burning,
	led_mode_panic,
	led_mode_demo,

	led_mode_last
};

enum led_id {
	led_id_wan,
	led_id_traffic,
	led_id_tel,
	led_id_tv,
	led_id_wifi,
	led_id_alarm,

	led_id_red,
	led_id_green,
	led_id_blue,

	led_id_last
};

enum led_state {
	led_state_unchanged,
	led_state_on,
	led_state_off,
	led_state_toggle,
	led_state_blinkonce,
	led_state_slowblinks,
	led_state_blinks
};

/** led dev ioctl */

#define LED_IOCTL_MAGIC	'M'

struct leds_dev_ioctl_struct {
	enum led_mode mode;
	enum led_id id;
	enum led_state state;
};

#define LED_IOCTL_SET_MODE _IOW(LED_IOCTL_MAGIC, 0, struct leds_dev_ioctl_struct)
#define LED_IOCTL_GET_MODE _IOR(LED_IOCTL_MAGIC, 1, struct leds_dev_ioctl_struct)
#define LED_IOCTL_SET_LED  _IOW(LED_IOCTL_MAGIC, 2, struct leds_dev_ioctl_struct)
#define LED_IOCTL_GET_LED  _IOWR(LED_IOCTL_MAGIC, 3, struct leds_dev_ioctl_struct)

#ifdef __KERNEL__

#define LED_ACTIVE_LOW	0x8000
#define LED_ACTIVE_HIGH	0x0000

#define LED_OFF(X)	(!!(X & LED_ACTIVE_LOW))
#define LED_ON(X)	(!(X & LED_ACTIVE_LOW))
#define LED_GPIO(X)	((X) & (~ LED_ACTIVE_LOW))

int leds_control(enum led_id id, enum led_state status);

u16 led_info(enum led_id led);
void nxp_74hc164_flush(void);

#ifdef CONFIG_BOARD_NEUFBOX4
void leds_burn_show_progress(void);
void leds_burn_reset(void);
#endif

#endif				/* __KERNEL__ */

#endif				/* _NEUFBOX_LEDS_H_ */
