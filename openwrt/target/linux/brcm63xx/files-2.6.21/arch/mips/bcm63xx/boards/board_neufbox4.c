/*!
 * \file board.c
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
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/if_ether.h>
#include <linux/proc_fs.h>

#include <boardparms.h>
#include <bcm_map_part.h>
#include <bcm_hwdefs.h>
#include <bcm63xx_cpu.h>
#include <bcm63xx_regs.h>
#include <bcm63xx_io.h>
#include <bcm63xx_irq.h>
#include <bcm63xx_board.h>
#include <bcm63xx_dev_uart.h>
#include <bcm63xx_dev_wdt.h>

#include <neufbox/events.h>
#include <neufbox/leds.h>
#include <box/board.h>
#include <box/partition.h>

#define SES_TIMEOUT		(130*HZ)

void __init gpio_poll_init(struct gpio_poll_struct *gpio);

struct board neufbox_board;

#define GPIO_74HC164D_OFFSET 64
#define GPIO_74HC164D(X) (GPIO_74HC164D_OFFSET + (X))
#define GPIO_CPU_OFFSET 0
#define GPIO_CPU(X) (GPIO_CPU_OFFSET + (X))

static struct resource neufbox4_nxp_74hc164_data[] __initdata = {
	{
	 .start = GPIO_CPU(6),	/* clock */
	 .end = GPIO_CPU(7),	/* data */
	 },
};

static struct platform_device neufbox4_nxp_74hc164_dev = {
	.name = "nxp-74hc164",
	.resource = neufbox4_nxp_74hc164_data,
	.num_resources = ARRAY_SIZE(neufbox4_nxp_74hc164_data),
};

static u16 leds_maps_nb4_sercomm[] = {
	[led_id_wan] = GPIO_74HC164D(4) | LED_ACTIVE_LOW,
	[led_id_traffic] = GPIO_CPU(2) | LED_ACTIVE_LOW,
	[led_id_tel] = GPIO_74HC164D(3) | LED_ACTIVE_LOW,
	[led_id_tv] = GPIO_74HC164D(2) | LED_ACTIVE_LOW,
	[led_id_wifi] = GPIO_CPU(15) | LED_ACTIVE_LOW,
	[led_id_alarm] = GPIO_74HC164D(0) | LED_ACTIVE_LOW,

	[led_id_red] = GPIO_CPU(29) | LED_ACTIVE_LOW,
	[led_id_green] = GPIO_CPU(30) | LED_ACTIVE_LOW,
	[led_id_blue] = GPIO_CPU(4) | LED_ACTIVE_LOW,
};

static u16 leds_maps_nb4f_sercomm[] = {
	[led_id_wan] = GPIO_CPU(1) | LED_ACTIVE_LOW,
	[led_id_traffic] = GPIO_CPU(2) | LED_ACTIVE_LOW,
	[led_id_tel] = GPIO_CPU(3) | LED_ACTIVE_LOW,
	[led_id_tv] = GPIO_CPU(5) | LED_ACTIVE_LOW,
	[led_id_wifi] = GPIO_CPU(15) | LED_ACTIVE_LOW,
	[led_id_alarm] = GPIO_CPU(8) | LED_ACTIVE_LOW,

	[led_id_red] = GPIO_CPU(29) | LED_ACTIVE_LOW,
	[led_id_green] = GPIO_CPU(30) | LED_ACTIVE_LOW,
	[led_id_blue] = GPIO_CPU(4) | LED_ACTIVE_LOW,
};

static u16 leds_maps_nb4_foxconn[] = {
	[led_id_wan] = GPIO_74HC164D(4) | LED_ACTIVE_LOW,
	[led_id_traffic] = GPIO_CPU(2) | LED_ACTIVE_HIGH,
	[led_id_tel] = GPIO_74HC164D(3) | LED_ACTIVE_LOW,
	[led_id_tv] = GPIO_74HC164D(2) | LED_ACTIVE_LOW,
	[led_id_wifi] = GPIO_CPU(15) | LED_ACTIVE_HIGH,
	[led_id_alarm] = GPIO_74HC164D(0) | LED_ACTIVE_LOW,

	[led_id_red] = GPIO_CPU(29) | LED_ACTIVE_HIGH,
	[led_id_green] = GPIO_CPU(30) | LED_ACTIVE_HIGH,
	[led_id_blue] = GPIO_CPU(4) | LED_ACTIVE_HIGH,
};

static u16 *leds_map_neufbox = leds_maps_nb4_foxconn;

u16 led_info(enum led_id led)
{
	return leds_map_neufbox[led];
}

EXPORT_SYMBOL(led_info);

/* ------- */

static u16 nb4_ser_r0[] = {
	[gpio_clip_button] = GPIO_CPU(31),
	[gpio_service_button] = GPIO_CPU(27),
	[gpio_reset_button] = GPIO_CPU(34),
	[gpio_ses_button] = GPIO_CPU(37),

	[gpio_voip_relay] = GPIO_CPU(10),

	[irq_reset_button] = INTERRUPT_ID_EXTERNAL_0,
	[irq_ses_button] = INTERRUPT_ID_EXTERNAL_3,
};

static u16 nb4f_ser_r0[] = {
	[gpio_clip_button] = GPIO_CPU(13),
	[gpio_service_button] = GPIO_CPU(27),
	[gpio_reset_button] = GPIO_CPU(34),
	[gpio_ses_button] = GPIO_CPU(37),

	[gpio_voip_relay] = GPIO_CPU(10),

	[irq_reset_button] = INTERRUPT_ID_EXTERNAL_0,
	[irq_ses_button] = INTERRUPT_ID_EXTERNAL_3,
#if 0
	[gpio_femtocell_factory_reset] = BP_GPIO_9_AH,
	[gpio_femtocell_reset] = BP_GPIO_11_AL,
	[gpio_ip101_reset] = BP_GPIO_22_AL,
	[gpio_femtocell_module_detection] = BP_GPIO_23_AL,
	[gpio_femtocell_gpio1] = BP_GPIO_25_AL,
	[gpio_femtocell_gpio2] = BP_GPIO_26_AL,
	[gpio_femtocell_functionning_status] = BP_GPIO_31_AH,
	[gpio_femtocell_regulator_enable] = BP_GPIO_36_AH,
#endif
};

static u16 nb4_fxc_r0[] = {
	[gpio_clip_button] = GPIO_CPU(31),
	[gpio_service_button] = GPIO_CPU(27),
	[gpio_reset_button] = GPIO_CPU(34),
	[gpio_ses_button] = GPIO_CPU(37),

	[gpio_voip_relay] = GPIO_CPU(10),

	[irq_reset_button] = INTERRUPT_ID_EXTERNAL_3,
	[irq_ses_button] = INTERRUPT_ID_EXTERNAL_0,
};

static u16 nb4_fxc_r1[] = {
	[gpio_clip_button] = GPIO_CPU(31),
	[gpio_service_button] = GPIO_CPU(27),
	[gpio_reset_button] = GPIO_CPU(34),
	[gpio_ses_button] = GPIO_CPU(37),

	[gpio_voip_relay] = GPIO_CPU(10),

	[irq_reset_button] = INTERRUPT_ID_EXTERNAL_0,
	[irq_ses_button] = INTERRUPT_ID_EXTERNAL_3,
};

/* ------- */

int gpio_to_irq(unsigned gpio)
{
	if (gpio == neufbox_resource(gpio_reset_button)) {
		return neufbox_resource(irq_reset_button);
	} else if (gpio == neufbox_resource(gpio_ses_button)) {
		return neufbox_resource(irq_ses_button);
	}

	return -EINVAL;
}

int irq_to_gpio(unsigned irq)
{
	if (irq == neufbox_resource(irq_reset_button)) {
		return (neufbox_resource(gpio_reset_button));
	} else if (irq == neufbox_resource(irq_ses_button)) {
		return (neufbox_resource(gpio_ses_button));
	}

	return -EINVAL;
}

EXPORT_SYMBOL(irq_to_gpio);
EXPORT_SYMBOL(gpio_to_irq);

static void switch_ses_timeout(unsigned long data)
{
	int irq;
	int gpio;

	printk("SES timeout\n");

	gpio = neufbox_resource(gpio_ses_button);
	irq = gpio_to_irq(gpio);

	/* Clear external interrupt */
	PERF->ExtIrqCfg |=
	    (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));
	PERF->ExtIrqCfg |=
	    (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT));
	enable_irq(irq);

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_ses_timeout);
#endif				/* CONFIG_NEUFBOX_EVENTS */
}

static irqreturn_t switch_ses_handler(int irq, void *dev_id)
{
	struct board *board = &neufbox_board;

	printk("IRQ: %s\n", "SES down");
	init_timer(&board->ses_timeout);
	board->ses_timeout.function = switch_ses_timeout;
	board->ses_timeout.expires = jiffies + (SES_TIMEOUT);
	add_timer(&board->ses_timeout);

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_ses_start);
#endif				/* CONFIG_NEUFBOX_EVENTS */

	PERF->ExtIrqCfg |=
	    (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));
	disable_irq(irq);

	return IRQ_HANDLED;
}

static irqreturn_t switch_reset_handler(int irq, void *dev_id)
{
	printk("IRQ: %s\n", "RESET down");

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_reset_down);
#endif				/* CONFIG_NEUFBOX_EVENTS */

	PERF->ExtIrqCfg |=
	    (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));

	return IRQ_HANDLED;
}

static void trigger_service_long(struct gpio_poll_struct *gpio)
{
	printk("%s long\n", gpio->name);

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_service_long);
#endif				/* CONFIG_NEUFBOX_EVENTS */
}

static void trigger_service_up(void)
{
#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_service_up);
#endif				/* CONFIG_NEUFBOX_EVENTS */
}

static void trigger_service_down(void)
{
#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_service_down);
#endif				/* CONFIG_NEUFBOX_EVENTS */
}

static void trigger_clip_long(struct gpio_poll_struct *gpio)
{
	int gpio_phy_link = 0;
	int gpio_phy_speed = 28;

	/*  notification for Fab control */
	gpio_set_value(gpio_phy_speed, gpio_get_value(gpio_phy_link));

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_clip_long);
#endif				/* CONFIG_NEUFBOX_EVENTS */

	printk("%s long\n", gpio->name);
}

static void trigger_clip_up(void)
{
	int gpio_phy_link = 0;
	int gpio_phy_speed = 28;

	/*  notification for Fab control */
	gpio_set_value(gpio_phy_speed, gpio_get_value(gpio_phy_link));

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_clip_up);
#endif				/* CONFIG_NEUFBOX_EVENTS */
}

static void trigger_clip_down(void)
{
	int gpio_phy_link = 0;
	int gpio_phy_speed = 28;

	/*  notification for Fab control */
	gpio_set_value(gpio_phy_speed, !gpio_get_value(gpio_phy_link));

#ifdef CONFIG_NEUFBOX_EVENTS
	event_enqueue(event_id_clip_down);
#endif				/* CONFIG_NEUFBOX_EVENTS */

}

static void serialization_init(struct board *board)
{
	u8 *boot_addr;
	u32 val;
	NVRAM_DATA const *nvram;
	struct serialization *serialization = &board->serialization;

	/* read base address of boot chip select (0) */
	val = bcm_mpi_readl(MPI_CSBASE_REG(0));
	val &= MPI_CSBASE_BASE_MASK;
	boot_addr = (u8 *) KSEG1ADDR(val);

	/* extract nvram data: mac address */
	nvram = (NVRAM_DATA const *)(boot_addr + NVRAM_DATA_OFFSET);
	memcpy(serialization->mac_address, nvram->ucaBaseMacAddr,
	       sizeof(serialization->mac_address));

	/* extract pid + wpa */
	memcpy(serialization, boot_addr + SERIALIZATION_BASE,
	       40 /* pid + wpa_key */ );

	printk("neufbox4: %s %02x:%02x:%02x:%02x:%02x:%02x\n",
	       serialization->pid, serialization->mac_address[0],
	       serialization->mac_address[1], serialization->mac_address[2],
	       serialization->mac_address[3], serialization->mac_address[4],
	       serialization->mac_address[5]);

	board->femtocell = 0;
	if (memcmp(serialization->pid, "NB4-FXC", sizeof("NB4-FXC") - 1) == 0) {
		leds_map_neufbox = leds_maps_nb4_foxconn;
		if (strcmp(serialization->pid, "NB4-FXC-r0") == 0) {
			board->resource = nb4_fxc_r0;
		} else {
			board->resource = nb4_fxc_r1;
		}
	} else
	    if (memcmp(serialization->pid, "NB4F-SER", sizeof("NB4F-SER") - 1)
		== 0) {
		board->femtocell = 1;
		leds_map_neufbox = leds_maps_nb4f_sercomm;
		board->resource = nb4f_ser_r0;
	} else {		/* if (memcmp(serialization->pid, "NB4-SER", sizeof("NB4-SER") - 1) == 0) */
		leds_map_neufbox = leds_maps_nb4_sercomm;
		board->resource = nb4_ser_r0;
	}
}

void __init board_prom_init(void)
{
	serialization_init(&neufbox_board);
}

/*
 * return board name for /proc/cpuinfo
 */
const char *board_get_name(void)
{
	return neufbox_board.serialization.pid;
}

/*
 * register & return a new board mac address
 */
int board_get_mac_address(u8 * mac, unsigned off)
{
	unsigned int *mac_last_digits = (unsigned int *)(mac + 2);
	unsigned int buf;

	memcpy(mac, neufbox_board.serialization.mac_address, ETH_ALEN);

	buf = ntohl(*mac_last_digits);
	buf += off;
	*mac_last_digits = htonl(buf);

	return 0;
}

static int proc_read_mac(char *buf, char **start, off_t offset, int count,
			 int *eof, void *data)
{
	int len = 0;
	u8 const *const mac = neufbox_board.serialization.mac_address;

	len =
	    snprintf(buf, count, "%02X%02X%02X%02X%02X%02X\n", mac[0], mac[1],
		     mac[2], mac[3], mac[4], mac[5]);
	*eof = 1;
	return len;
}

static int proc_read_pid(char *buf, char **start, off_t offset, int count,
			 int *eof, void *data)
{
	int len = 0;
	char const *const pid = neufbox_board.serialization.pid;

	len = snprintf(buf, count, "%s\n", pid);
	*eof = 1;
	return len;
}

static int proc_write_pid(struct file *file, const char __user * buffer,
			  unsigned long count, void *data)
{
	serialization_init(&neufbox_board);

	return count;
}

static struct resource gpio_resources[] __initdata = {
	{
	 .start = ~0,		/* 0-31 */
	 },
};

static struct platform_device gpio_dev = {
	.name = "GPIODEV",
	.resource = gpio_resources,
	.num_resources = ARRAY_SIZE(gpio_resources),
};

static struct resource mtd_resources[] = {
	{
	 .start = 0,		/* filled at runtime */
	 .end = 0,		/* filled at runtime */
	 .flags = IORESOURCE_MEM,
	 }
};

static struct platform_device mtd_dev = {
	.name = "neufbox4-flash",
	.resource = mtd_resources,
	.num_resources = ARRAY_SIZE(mtd_resources),
};

static int __init board_init(void)
{
	struct board *board;
	struct proc_dir_entry *proc;
	int irq;
	int gpio;

	bcm63xx_uart_register();
	bcm63xx_wdt_register();

	board = &neufbox_board;

	/* Enable procfs entry, so user have access to profile */
	board->procfs = proc_mkdir("neufbox", NULL);
	if (!board->procfs) {
		printk(KERN_ERR "proc_mkdir(neufbox) failed\n");
		return -ENOMEM;
	}

	create_proc_read_entry("mac", 0, board->procfs, proc_read_mac, NULL);
	proc = create_proc_entry("pid", 0, board->procfs);
	if (!proc) {
		printk(KERN_ERR "proc neufbox/pid failed\n");
		return -ENOMEM;
	}

	proc->read_proc = proc_read_pid;
	proc->write_proc = proc_write_pid;

	/* Enable reset IRQ */
	gpio = neufbox_resource(gpio_reset_button);
	gpio_request(gpio, "BTN reset");
	gpio_direction_input(gpio);
	irq = gpio_to_irq(gpio);
	request_irq(irq, switch_reset_handler,
		    IRQF_SAMPLE_RANDOM | IRQF_DISABLED | IRQ_TYPE_EDGE_RISING,
		    "BTN reset", NULL);

	/* Enable ses IRQ */
	gpio = neufbox_resource(gpio_ses_button);
	gpio_request(gpio, "BTN ses");
	gpio_direction_input(gpio);
	irq = gpio_to_irq(gpio);
	request_irq(irq, switch_ses_handler,
		    IRQF_SAMPLE_RANDOM | IRQF_DISABLED | IRQ_TYPE_EDGE_RISING,
		    "BTN ses", NULL);

	/* Enable service Poll */
	board->service_poll.name = "BTN service";
	board->service_poll.gpio = neufbox_resource(gpio_service_button);
	board->service_poll.timerid = TIMER_T0_ID;
	board->service_poll.fn_up = trigger_service_up;
	board->service_poll.fn_down = trigger_service_down;
	board->service_poll.fn_long = (void *)trigger_service_long;
	gpio_poll_init(&board->service_poll);

	if (!neufbox_femtocell()) {
		/* Enable clip Poll */
		board->clip_poll.name = "BTN clip";
		board->clip_poll.gpio = neufbox_resource(gpio_clip_button);
		board->clip_poll.timerid = TIMER_T1_ID;
		board->clip_poll.fn_up = trigger_clip_up;
		board->clip_poll.fn_down = trigger_clip_down;
		board->clip_poll.fn_long = (void *)trigger_clip_long;
		gpio_poll_init(&board->clip_poll);
	} else {
		printk("FemToCell board detected\n");
#if 0				/* should never happen */
		/* factory reset */
		gpio = 9;
		gpio_request(gpio, "FemToCell factory reset");
		gpio_direction_output(gpio, 0);
#endif
		/* reset */
		gpio = 11;
		gpio_request(gpio, "FemToCell reset");
		gpio_direction_output(gpio, 0);
		/* ip101 reset */
		gpio = 22;
		gpio_request(gpio, "FemToCell ip101 reset");
		gpio_direction_output(gpio, 0);

		/* module detection */
		gpio = 23;
		gpio_request(gpio, "FemToCell module detection");
		gpio_direction_output(gpio, 0);
		/* functionning */
		gpio = 31;
		gpio_request(gpio, "FemToCell functionning");
		gpio_direction_output(gpio, 0);
		/* regulator */
		gpio = 36;
		gpio_request(gpio, "FemToCell regulator");
		gpio_direction_output(gpio, 0);
	}

	/* read base address of boot chip select (0) */
	mtd_resources[0].start = MPI->cs[0].base & 0xffffff00;
	mtd_resources[0].end = mtd_resources[0].start + NEUFBOX_FLASH_SIZE - 1;

	platform_device_register(&mtd_dev);
	platform_device_register(&neufbox4_nxp_74hc164_dev);
	platform_device_register(&gpio_dev);

	return 0;
}

module_init(board_init);

EXPORT_SYMBOL(neufbox_board);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miguel GAIO <miguel.gaio@efixo.com>");
