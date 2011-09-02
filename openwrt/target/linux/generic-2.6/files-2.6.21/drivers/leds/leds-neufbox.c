/*!
 * \file leds.c
 *
 * \brief Implement neufbox leds
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

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

#include <asm/bitops.h>
#include <asm/atomic.h>

#include <neufbox/leds.h>

#define LEDS_MINOR 132		/* LED device driver */

struct leds_device {
	struct timer_list timer; /** leds timer structure */
	unsigned long counter;	/** leds timer counter */
	atomic_t mode; /** leds device mode */
	void (*fn_leds_mode_process) (struct leds_device *);	/* callback for leds management */
	atomic_t command[led_id_last]; /** leds command registers */
	enum led_state state[led_id_last]; /** leds states registers */
};

static struct leds_device *leds_dev;

static char const *leds_name[] = {
	[led_id_wan] = "wan",
	[led_id_traffic] = "traffic",
	[led_id_tel] = "tel",
	[led_id_tv] = "tv",
	[led_id_wifi] = "wifi",
	[led_id_alarm] = "alarm",

	[led_id_red] = "red",
	[led_id_green] = "green",
	[led_id_blue] = "blue",
};

static inline void led_off(enum led_id led)
{
	u16 info = led_info(led);

	gpio_set_value(LED_GPIO(info), LED_OFF(info));
}

static inline void led_on(enum led_id led)
{
	u16 info = led_info(led);

	gpio_set_value(LED_GPIO(info), LED_ON(info));
}

static inline int led_toggle(enum led_id led)
{
	u16 info = led_info(led);
	int value = !gpio_get_value(LED_GPIO(info));

	gpio_set_value(LED_GPIO(info), value);

	return (info & LED_ACTIVE_LOW) ? value : !value;
}

static void leds_process_disable(struct leds_device *dev)
{
	/* nothing TODO */
}

static void leds_process_control(struct leds_device *dev)
{
	enum led_id id;
	enum led_state state;
	int blink_state = -1;

	for (id = led_id_wan; id < led_id_last; ++id) {
		/* critical region */
		state = atomic_read(&dev->command[id]);
		(void)atomic_cmpxchg(&dev->command[id], state,
				     led_state_unchanged);

		switch (state) {
		case led_state_unchanged:
			continue;
		case led_state_on:
			led_on(id);
			break;

		case led_state_off:
			led_off(id);
			break;

		case led_state_toggle:
			led_toggle(id);
			break;

		case led_state_blinkonce:
			led_toggle(id);
			/* critical region */
			(void)atomic_cmpxchg(&dev->command[id],
					     led_state_unchanged,
					     led_state_toggle);
			break;

		case led_state_slowblinks:
			if (!(dev->counter % 8)) {
				led_toggle(id);
			}
			/* critical region */
			(void)atomic_cmpxchg(&dev->command[id],
					     led_state_unchanged,
					     led_state_slowblinks);
			break;

		case led_state_blinks:
			if (blink_state < 0) {
				blink_state = led_toggle(id);
			} else if (blink_state) {
				led_off(id);
			} else {
				led_on(id);
			}
			/* critical region */
			(void)atomic_cmpxchg(&dev->command[id],
					     led_state_unchanged,
					     led_state_blinks);
			break;

		default:
			printk(KERN_ERR
			       "leds: invalid state [ID=%d STATE=%d\n",
			       id, state);
			continue;
		}

		dev->state[id] = state;
	}

	nxp_74hc164_flush();
}

static void leds_process_k2000(struct leds_device *dev)
{
	enum led_id id;

	for (id = led_id_wan; id <= led_id_alarm; ++id)
		led_off(id);

	switch (dev->counter % 10) {
	case 0:
		led_on(led_id_wan);
		break;

	case 1:
		led_on(led_id_traffic);
		break;

	case 2:
		led_on(led_id_tel);
		break;

	case 3:
		led_on(led_id_tv);
		break;

	case 4:
		led_on(led_id_wifi);
		break;

	case 5:
		led_on(led_id_alarm);
		break;

	case 6:
		led_on(led_id_wifi);
		break;

	case 7:
		led_on(led_id_tv);
		break;

	case 8:
		led_on(led_id_tel);
		break;

	case 9:
		led_on(led_id_traffic);
		break;
	}

	nxp_74hc164_flush();

	dev->timer.expires = jiffies + (HZ);
}

static void leds_process_downloading(struct leds_device *dev)
{
	leds_process_k2000(dev);

	/* set led service to yellow */
	led_on(led_id_red);
	led_on(led_id_green);
	led_off(led_id_blue);
}

static void leds_process_burning(struct leds_device *dev)
{
	leds_process_k2000(dev);

	/* set led service to red */
	led_on(led_id_red);
	led_off(led_id_green);
	led_off(led_id_blue);
}

static void leds_process_panic(struct leds_device *dev)
{
	enum led_id id;

	if (dev->counter & 0x01) {
		for (id = led_id_wan; id < led_id_last; ++id) {
			led_on(id);
		}
	} else {
		for (id = led_id_wan; id < led_id_last; ++id) {
			led_off(id);
		}
	}

	nxp_74hc164_flush();
}

static void leds_process_demo(struct leds_device *dev)
{
	enum led_id id;

	for (id = led_id_wan; id <= led_id_alarm; ++id)
		led_off(id);

	if (dev->counter % 6) {
		switch ((dev->counter / 6) % 3) {
		case 0:
			led_on(led_id_red);
			led_off(led_id_green);
			led_off(led_id_blue);
			break;

		case 1:
			led_off(led_id_red);
			led_on(led_id_green);
			led_off(led_id_blue);
			break;

		case 2:
			led_off(led_id_red);
			led_off(led_id_green);
			led_on(led_id_blue);
			break;
		}
	}

	switch (dev->counter % 6) {
	case 0:
		led_on(led_id_wan);
		led_on(led_id_alarm);
		break;

	case 1:
		led_on(led_id_traffic);
		led_on(led_id_wifi);
		break;

	case 2:
		led_on(led_id_tel);
		led_on(led_id_tv);
		break;

	case 3:
		led_on(led_id_tel);
		led_on(led_id_tv);
		break;

	case 4:
		led_on(led_id_traffic);
		led_on(led_id_wifi);
		break;

	case 5:
		led_on(led_id_wan);
		led_on(led_id_alarm);
		break;
	}

	nxp_74hc164_flush();
}

static void inline leds_timer_function(unsigned long data)
{
	struct leds_device *dev = (struct leds_device *)data;

	dev->timer.expires = jiffies + (HZ >> 3);

	(dev->fn_leds_mode_process) (dev);

	++dev->counter;

	add_timer(&dev->timer);
}

static int leds_set_mode(struct leds_device *dev, enum led_mode mode)
{
	enum led_mode prev_mode = atomic_read(&dev->mode);
	enum led_id id;

	if (mode == prev_mode) {
		return prev_mode;
	}

	switch (mode) {
	case led_mode_disable:
		dev->fn_leds_mode_process = leds_process_disable;
		break;

	case led_mode_control:
		dev->fn_leds_mode_process = leds_process_control;
		break;

	case led_mode_downloading:
		dev->fn_leds_mode_process = leds_process_downloading;
		break;

	case led_mode_burning:
		dev->fn_leds_mode_process = leds_process_burning;
		break;

	case led_mode_panic:
		dev->fn_leds_mode_process = leds_process_panic;
		break;

	case led_mode_demo:
		dev->fn_leds_mode_process = leds_process_demo;
		break;

	default:
		printk(KERN_ERR "leds: invalid mode %X\n", mode);
		return -1;
	}

	atomic_set(&dev->mode, mode);

	if (mode == led_mode_control) {
		for (id = led_id_wan; id < led_id_last; ++id) {
			/* critical region */
			(void)atomic_cmpxchg(&dev->command[id],
					     led_state_unchanged,
					     dev->state[id]);
		}
	} else {
		for (id = led_id_wan; id < led_id_last; ++id) {
			led_off(id);
		}

		nxp_74hc164_flush();
	}

	return prev_mode;
}

int leds_control(enum led_id id, enum led_state state)
{
	struct leds_device *dev = leds_dev;

	if (id >= led_id_last || state > led_state_blinks) {
		printk(KERN_WARNING
		       "%s: broken control [ID=%u] [STATE=%u]\n",
		       "leds", id, state);
		return -1;
	}

	/* critical region */
	(void)atomic_set(&dev->command[id], state);

	return 0;
}

static int leds_open(struct inode *inode, struct file *file)
{
	struct leds_device *dev = leds_dev;

	file->private_data = dev;

	return nonseekable_open(inode, file);
}

static long leds_compat_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	struct leds_device *dev = file->private_data;
	struct leds_dev_ioctl_struct leds_ioctl;
	enum led_mode mode;

	switch (cmd) {
	case LED_IOCTL_SET_MODE:
		if (copy_from_user
		    ((void *)&leds_ioctl, (void __user *)arg,
		     sizeof(leds_ioctl)) < 0) {
			return -EFAULT;
		}

		if ((mode = leds_set_mode(dev, leds_ioctl.mode)) < 0) {
			return -EINVAL;
		}

		leds_ioctl.mode = mode;
		__copy_to_user((void __user *)arg, (void *)&leds_ioctl,
			       sizeof(leds_ioctl));
		break;
	case LED_IOCTL_GET_MODE:
		leds_ioctl.mode = atomic_read(&dev->mode);
		if (copy_to_user
		    ((void __user *)arg, (void *)&leds_ioctl,
		     sizeof(leds_ioctl)) < 0) {
			return -EFAULT;
		}
		break;

	case LED_IOCTL_SET_LED:
		if (copy_from_user
		    ((void *)&leds_ioctl, (void __user *)arg,
		     sizeof(leds_ioctl)) < 0) {
			return -EFAULT;
		}
		if (leds_control(leds_ioctl.id, leds_ioctl.state) < 0) {
			return -EINVAL;
		}
		break;

	case LED_IOCTL_GET_LED:
		if (copy_from_user
		    ((void *)&leds_ioctl, (void __user *)arg,
		     sizeof(leds_ioctl)) < 0) {
			return -EFAULT;
		}

		if (leds_ioctl.id >= led_id_last) {
			return -EINVAL;
		}

		/* in disable mode check for command state */
		if (atomic_read(&dev->mode) == led_mode_disable) {
			leds_ioctl.state =
			    atomic_read(&dev->command[leds_ioctl.id]);
			if (leds_ioctl.state == led_state_unchanged) {
				leds_ioctl.state = dev->state[leds_ioctl.id];
			}
		} else {
			leds_ioctl.state = dev->state[leds_ioctl.id];
		}

		if (__copy_to_user
		    ((void __user *)arg, (void *)&leds_ioctl,
		     sizeof(leds_ioctl)) < 0) {
			return -EFAULT;
		}
		break;

	default:
		printk(KERN_WARNING "leds: invalid ioctl %d\n", cmd);
		return -ENOTTY;
	}

	return 0;
}

static int leds_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg)
{
	return leds_compat_ioctl(file, cmd, arg);
}

static struct file_operations leds_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.ioctl = leds_ioctl,
	.compat_ioctl = leds_compat_ioctl,
	.open = leds_open,
};

static struct miscdevice leds_miscdev = {
	.minor = LEDS_MINOR,
	.name = "leds",
	.fops = &leds_fops,
};

static long leds_panic_blink(long time)
{
	enum led_id id;
	struct leds_device *dev;

	dev = leds_dev;
	if (!dev) {
		return 0;
	}

	if (!(time % 256)) {
		static int on = 0;

		if ((on = !on)) {
			for (id = led_id_wan; id < led_id_last; ++id) {
				led_on(id);
			}
		} else {
			for (id = led_id_wan; id < led_id_last; ++id) {
				led_off(id);
			}
		}

		nxp_74hc164_flush();
	}

	return 0;
}

static int __init leds_init(void)
{
	struct leds_device *dev;
	enum led_id led;
	int err;

	extern long (*panic_blink) (long time);

	err = misc_register(&leds_miscdev);
	if (err < 0) {
		printk(KERN_ERR "error: register leds device\n");
		return err;
	}

	/* alloc leds device structure */
	if (!(leds_dev = dev = kzalloc(sizeof(*dev), GFP_KERNEL))) {
		printk(KERN_ERR "error: alloc leds device\n");
		return -ENOMEM;
	}

	panic_blink = leds_panic_blink;

	/* enable leds GPIOs and set default values */
	for (led = led_id_wan; led < led_id_last; ++led) {
		int gpio;
		u16 info = led_info(led);

		gpio = LED_GPIO(info);
		printk("[request leds: %s gpio: %d]\n", leds_name[led], gpio);
		if (gpio_request(gpio, NULL) < 0) {
			printk(KERN_ERR "error: gpio_request(%d)\n", gpio);
			BUG();
		}
		gpio_direction_output(gpio, 0);
		dev->state[led] = led_state_off;
	}

	leds_set_mode(dev, led_mode_control);

	/* set led service to the right color */
#ifdef CONFIG_NEUFBOX_MAIN
	/* yellow */
	led_on(led_id_red);
	led_on(led_id_green);
	led_off(led_id_blue);
	leds_control(led_id_red, led_state_on);
	leds_control(led_id_green, led_state_on);
	leds_control(led_id_blue, led_state_off);
#else				/* CONFIG_NEUFBOX_RESCUE */
	/* blue */
	led_off(led_id_red);
	led_off(led_id_green);
	led_on(led_id_blue);
	leds_control(led_id_red, led_state_off);
	leds_control(led_id_green, led_state_off);
	leds_control(led_id_blue, led_state_on);
#endif				/* CONFIG_NEUFBOX_MAIN */

	nxp_74hc164_flush();

	/* setup timer */
	init_timer(&dev->timer);
	leds_dev->timer.function = leds_timer_function;
	leds_dev->timer.data = (unsigned long)dev;
	leds_dev->timer.expires = jiffies;
	add_timer(&dev->timer);

	return 0;
}

static void __exit leds_exit(void)
{
	struct leds_device *dev = leds_dev;
	enum led_id id;

	printk("%s: clean up...\n", "leds");
	del_timer(&dev->timer);

	/* reset leds states */
	leds_set_mode(dev, led_mode_disable);

	/* cleanup leds GPIOs */
	for (id = led_id_wan; id < led_id_last; ++id) {
		gpio_direction_output(LED_GPIO(id), 0);
	}

	misc_deregister(&leds_miscdev);

	/* free leds device structure */
	kfree(dev);
}

module_init(leds_init);
module_exit(leds_exit);

EXPORT_SYMBOL(leds_control);

MODULE_AUTHOR("Miguel GAIO");
MODULE_DESCRIPTION("Driver neufbox leds");
MODULE_ALIAS_MISCDEV(LEDS_MINOR);
MODULE_LICENSE("GPL");
