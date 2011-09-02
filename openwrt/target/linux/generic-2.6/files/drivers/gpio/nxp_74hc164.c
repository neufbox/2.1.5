/*
 *  NXP 74HC153 - Dual 4-input multiplexer GPIO driver
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *  Miguel Gaio miguel.gaio@efixo.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <asm/bitops.h>

struct nxp_74hc164_chip {
	struct gpio_chip gpio_chip;
	spinlock_t lock;
#ifdef CONFIG_LEDS_NEUFBOX
	unsigned long dirty;
#endif
	unsigned long data;
	int gpio_pin_data;
	int gpio_pin_clk;
};

static struct nxp_74hc164_chip *gpio_to_nxp(struct gpio_chip *gc)
{
	return container_of(gc, struct nxp_74hc164_chip, gpio_chip);
}

static int nxp_74hc164_get_value(struct gpio_chip *gc, unsigned gpio)
{
	struct nxp_74hc164_chip *nxp = gpio_to_nxp(gc);

	return test_bit(gpio, &nxp->data);
}

static void nxp_74hc164_serialize(struct nxp_74hc164_chip *nxp)
{
	unsigned long flags;
	int i;
	u8 data;

	if (nxp->gpio_pin_clk < 0)
		return;

	spin_lock_irqsave(&nxp->lock, flags);
	data = nxp->data;
	for (i = 8; i > 0; --i, data <<= 1) {
		int gpio;

		gpio = nxp->gpio_pin_data;
		gpio_set_value(gpio, data & 0x80);

		/* clock frame */
		gpio = nxp->gpio_pin_clk;
		gpio_set_value(gpio, 1);
		__delay(5);
		gpio_set_value(gpio, 0);
		__delay(5);
	}
	spin_unlock_irqrestore(&nxp->lock, flags);
}

static void nxp_74hc164_set_value(struct gpio_chip *gc, unsigned gpio, int val)
{
	struct nxp_74hc164_chip *nxp = gpio_to_nxp(gc);

	if (val) {
		if (test_and_set_bit(gpio, &nxp->data) != val)
#ifdef CONFIG_LEDS_NEUFBOX
			__set_bit(1, &nxp->dirty);
#else
			nxp_74hc164_serialize(nxp);
#endif
	} else {
		if (test_and_clear_bit(gpio, &nxp->data) != val)
#ifdef CONFIG_LEDS_NEUFBOX
			__set_bit(1, &nxp->dirty);
#else
			nxp_74hc164_serialize(nxp);
#endif
	}
}

static int nxp_74hc164_direction_input(struct gpio_chip *gc, unsigned gpio)
{
	WARN_ON(1);
	return -EINVAL;
}

static int nxp_74hc164_direction_output(struct gpio_chip *gc,
					unsigned gpio, int val)
{
	nxp_74hc164_set_value(gc, gpio, val);
	return 0;
}

static struct nxp_74hc164_chip nxp_74hc164 = {
	.gpio_chip = {
		      .label = "nxp-74hc164",
		      .direction_input = nxp_74hc164_direction_input,
		      .direction_output = nxp_74hc164_direction_output,
		      .get = nxp_74hc164_get_value,
		      .set = nxp_74hc164_set_value,
		      .base = 64,
		      .ngpio = 8,
		      },
	.gpio_pin_clk = -1,
};

#ifdef CONFIG_LEDS_NEUFBOX
void nxp_74hc164_flush(void)
{
	struct nxp_74hc164_chip *nxp = &nxp_74hc164;

	if (test_and_clear_bit(1, &nxp->dirty)) {
		nxp_74hc164_serialize(nxp);
	}
}

EXPORT_SYMBOL(nxp_74hc164_flush);
#endif

static int nxp_74hc164_probe(struct platform_device *pdev)
{
	struct nxp_74hc164_chip *nxp = &nxp_74hc164;
	int err;

	if (pdev->num_resources != 1) {
		printk(KERN_ERR ": device may only have 1 resource\n");
		return -EINVAL;
	}

	nxp->gpio_pin_clk = pdev->resource[0].start;
	nxp->gpio_pin_data = pdev->resource[0].end;
	spin_lock_init(&nxp->lock);

	err = gpio_request(nxp->gpio_pin_clk, "nxp-74hc164:clk");
	if (err) {
		printk("%s unable to claim gpio %u, err=%d\n",
		       "nxp-74hc164", nxp->gpio_pin_clk, err);
		return err;
	}

	err = gpio_request(nxp->gpio_pin_data, "nxp-74hc164:data");
	if (err) {
		printk("%s unable to claim gpio %u, err=%d\n",
		       "nxp-74hc164", nxp->gpio_pin_data, err);
		goto err_free_clk;
	}

	err = gpio_direction_output(nxp->gpio_pin_clk, 0);
	if (err) {
		printk("%s unable to set direction of gpio %u, err=%d\n",
		       "nxp-74hc164", nxp->gpio_pin_clk, err);
		goto err_free_data;
	}

	err = gpio_direction_output(nxp->gpio_pin_data, 0);
	if (err) {
		printk("%s unable to set direction of gpio %u, err=%d\n",
		       "nxp-74hc164", nxp->gpio_pin_data, err);
		goto err_free_data;
	}

	err = gpiochip_add(&nxp->gpio_chip);
	if (err) {
		printk("%s unable to add gpio chip, err=%d\n", "nxp-74hc164",
		       err);
		goto err_free_data;
	}

	platform_set_drvdata(pdev, nxp);
	printk(KERN_INFO "%s registering %d GPIOs\n", "nxp-74hc164", 8);
	return 0;

 err_free_data:
	gpio_free(nxp->gpio_pin_data);
 err_free_clk:
	gpio_free(nxp->gpio_pin_clk);
	return err;
}

static int nxp_74hc164_remove(struct platform_device *pdev)
{
	struct nxp_74hc164_chip *nxp = platform_get_drvdata(pdev);
	int err;

	if (nxp) {
		if ((err = gpiochip_remove(&nxp->gpio_chip)) < 0) {
			printk("%s unable to remove gpio chip, err=%d\n", err);
			return err;
		}

		gpio_free(nxp->gpio_pin_clk);
		gpio_free(nxp->gpio_pin_data);

		nxp->gpio_pin_clk = -1;

		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static struct platform_driver nxp_74hc164_driver = {
	.probe = nxp_74hc164_probe,
	.remove = nxp_74hc164_remove,
	.driver = {
		   .name = "nxp-74hc164",
		   .owner = THIS_MODULE,
		   },
};

static int __init nxp_74hc164_init(void)
{
	int ret = platform_driver_register(&nxp_74hc164_driver);
	if (ret)
		printk(KERN_INFO ": Error registering platfom driver!");

	return ret;
}

subsys_initcall(nxp_74hc164_init);

static void __exit nxp_74hc164_exit(void)
{
	platform_driver_unregister(&nxp_74hc164_driver);
}

module_exit(nxp_74hc164_exit);

MODULE_DESCRIPTION("GPIO expander driver for NXP 74HC164");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" NXP_74HC164_DRIVER_NAME);
