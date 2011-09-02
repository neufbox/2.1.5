/*
 *      neufbox-eventqueue.c
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

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/kfifo.h>
#include <linux/kmod.h>
#include <linux/gpio.h>

#include <asm/bitops.h>

#include <neufbox/events.h>
#ifdef CONFIG_BOARD_NEUFBOX4
#include <box/board.h>
#endif				/* CONFIG_BOARD_NEUFBOX4 */

#define EVENTQUEUE_MINOR 222	/* EVENT device driver */
#define DRVNAME "event-queue: "

struct events_device {
	unsigned long inuse;

	wait_queue_head_t wait_queue;

	struct kfifo *fifo;
	spinlock_t fifo_lock;
};

static struct events_device *events_dev;

int event_enqueue(enum event_id id)
{
	struct events_device *dev = events_dev;

	if (kfifo_put(dev->fifo, (unsigned char *)&id, sizeof(id))
	    != sizeof(id)) {
		printk(DRVNAME "id:%d dropped\n", id);
	} else {
		wake_up_interruptible(&dev->wait_queue);
	}

	return 0;
}

static int events_open(struct inode *inode, struct file *file)
{
	struct events_device *dev = events_dev;

	if (test_and_set_bit(0, &dev->inuse))
		return -EBUSY;

	file->private_data = dev;

	return nonseekable_open(inode, file);
}

static int events_release(struct inode *inode, struct file *file)
{
	struct events_device *dev = file->private_data;

	clear_bit(0, &dev->inuse);
	return 0;
}

static unsigned int events_poll(struct file *file, poll_table * wait)
{
	struct events_device *dev = file->private_data;

	poll_wait(file, &dev->wait_queue, wait);

	if (kfifo_len(dev->fifo) != 0) {
		return POLLIN | POLLRDNORM;
	} else {
		return 0;
	}
}

static ssize_t events_read(struct file *file, char __user * buf,
			   size_t count, loff_t * f_pos)
{
	struct events_device *dev = file->private_data;
	enum event_id id;

	if (count < sizeof(id)) {
		return 0;
	}

	if (wait_event_interruptible(dev->wait_queue, kfifo_len(dev->fifo))) {
		return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */
	}

	kfifo_get(dev->fifo, (unsigned char *)&id, sizeof(id));

	if (id == event_id_reset_down) {
		char *argv[2] = { "/sbin/reset_to_factory", NULL };
		char *envp[3] =
		    { "HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

		printk("*** reset to factory ***\n");
		call_usermodehelper(argv[0], argv, envp, 0 /* no wait */ );
	}

	copy_to_user(buf, &id, sizeof(id));

	return sizeof(id);
}

static struct file_operations events_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.poll = events_poll,
	.read = events_read,
	.open = events_open,
	.release = events_release,
};

static struct miscdevice events_miscdev = {
	.minor = EVENTQUEUE_MINOR,
	.name = "events",
	.fops = &events_fops,
};

static int __init events_init(void)
{
	struct events_device *dev;
	int err;

	printk("events: init...\n");

	err = misc_register(&events_miscdev);
	if (err < 0) {
		return err;
	}

	/* alloc events device structure */
	if (!(events_dev = dev = kzalloc(sizeof(*dev), GFP_KERNEL))) {
		printk(KERN_ERR DRVNAME
		       "error: alloc device structure size=%zu\n",
		       sizeof(*dev));
		return -ENOMEM;
	}

	init_waitqueue_head(&dev->wait_queue);

	spin_lock_init(&dev->fifo_lock);
	dev->fifo = kfifo_alloc(64, GFP_KERNEL, &dev->fifo_lock);

	return 0;
}

static void __exit events_exit(void)
{
	struct events_device *dev = events_dev;

	printk(DRVNAME "cleanup...\n");

	misc_deregister(&events_miscdev);

	kfifo_free(dev->fifo);
	kfree(dev);
}

EXPORT_SYMBOL(event_enqueue);

subsys_initcall(events_init);

MODULE_AUTHOR("Miguel GAIO");
MODULE_DESCRIPTION("neufbox event queue");
MODULE_ALIAS_MISCDEV(EVENTS_MINOR);
MODULE_LICENSE("GPL");
