/*!
 * \file gpiopoll.c
 *
 * \brief Implement neufbox board
 *
 * \author Copyright 2006 Miguel GAIO <miguel.gaio@efixo.com>
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
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <bcm63xx_timer.h>

#include <box/board.h>

#define POLL_TRIGGER	(5)

static void gpio_poll(struct gpio_poll_struct *gpio)
{
	if (++gpio->trigger > POLL_TRIGGER) {
		gpio->trigger = POLL_TRIGGER;
	}

	if (gpio_get_value(gpio->gpio) != gpio->value) {
		gpio->trigger -= 2;

		if (gpio->trigger <= 0) {
			gpio->trigger = POLL_TRIGGER;
			gpio->value = !gpio->value;
			if (gpio->value) {	/* low -> high */
				if (timer_pending(&gpio->timer))
					del_timer(&gpio->timer);

				if (gpio->fn_up)
					(gpio->fn_up) ();

				printk("%s up\n", gpio->name);
			} else {	/* high -> low */
				if (gpio->fn_long) {
					gpio->timer.expires =
					    jiffies + (HZ << 2);
					add_timer(&gpio->timer);
				}

				if (gpio->fn_down)
					(gpio->fn_down) ();

				printk("%s down\n", gpio->name);
			}
		}
	}
}

void __init gpio_poll_init(struct gpio_poll_struct *gpio)
{
	printk("GPIO-%d polled: %s\n", gpio->gpio, gpio->name);

	gpio_request(gpio->gpio, gpio->name);
	gpio_direction_input(gpio->gpio);
	gpio->trigger = POLL_TRIGGER;
	gpio->value = 1;

	setup_timer(&gpio->timer, (void *)gpio->fn_long, (unsigned long)gpio);

	bcm63xx_timer_register(gpio->timerid, (void *)gpio_poll, gpio);
	bcm63xx_timer_set(gpio->timerid, 0, 20 * 1000 /* 20ms */ );
	bcm63xx_timer_enable(gpio->timerid);
}

void __exit gpio_poll_exit(struct gpio_poll_struct *gpio)
{
	bcm63xx_timer_disable(gpio->timerid);
	bcm63xx_timer_unregister(gpio->timerid);
	del_timer(&gpio->timer);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miguel GAIO <miguel.gaio@efixo.com>");
