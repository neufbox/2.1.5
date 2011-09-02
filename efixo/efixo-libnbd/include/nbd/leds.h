/*!
 * \file leds.h
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
 * $Id: leds.h 9532 2008-11-21 14:38:53Z mgo $
 */


#ifndef _LEDS_H_
#define _LEDS_H_


#include "kernel/box/leds.h"


/*! \fn nbd_leds_control(enum led_ids id, enum led_states status)
 *  \brief Control leds.
 * - <b>adsl</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>trafic</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>tel</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>tv</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>wifi</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>alarm</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>red</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>green</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>blue</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>logo</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>link</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>status</b> on, off, slowblinks, fastblinks, blinkonce
 * - <b>bright</b> off, low, medium, high
 * - <b>burn</b> start, stop
 * - <b>download</b> start, stop
 *  \param led Led ID
 *  \param status Led status
 *
 */
int nbd_leds_control( enum led_id id, enum led_state status );


enum led_state nbd_leds_state( enum led_id id );


enum led_mode nbd_leds_set_mode( enum led_mode mode );


enum led_mode nbd_leds_get_mode( void );


int nbd_leds_brightness( enum led_state status );


#endif /* _LEDS_H_ */
