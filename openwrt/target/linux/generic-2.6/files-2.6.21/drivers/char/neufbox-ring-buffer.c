/*
 *      neufbox_ring_buffer.c
 *
 *      Copyright 2006 Miguel GAIO <miguel.gaio@efixo.com>
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

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* printk(), min() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/poll.h>
#include <linux/time.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/errno.h>

#define DRV_NAME        "neufbox_ring_buffer"
#define DRV_VERSION     "1.00"
#define DRV_RELDATE     "10-Mar-2008"
#define PFX		DRV_NAME ": "

#define RING_BUFFER 229

/** from struct ring_buffer_device
 * \note struct ring_buffer_device use semaphore instead of spinlock
 * read process is NOT destructive process
 * write success if buffer is full
 */
struct ring_buffer_device {
	struct cdev cdev;	/* Char device structure */
	unsigned char *buffer;	/* the buffer holding the data */
	size_t size;		/* the size of the allocated buffer */
	unsigned long long in;	/* data is added at offset (in % size) */
	unsigned long long out;	/* data is extracted from off. (out % size) */
	unsigned long long pos;	/* position for tail purpose. (pos % size) */
	struct semaphore lock;	/* lock */
};

static struct ring_buffer_device *ring_buffer_devices;

#ifdef CONFIG_BOARD_NEUFBOX4
static int ring_buffer_sizes[] = {
	64 << 10,		/* daemon.log */
	64 << 10,		/* kern.log */
	64 << 10,		/* vodsl_proto.log */
	64 << 10,		/* vodsl.log */
	16 << 10,		/* messages */
	64 << 10,		/* syslog */
	16 << 10,		/* debug */
	16 << 10,		/* vodsl_events.log */
	64 << 10,		/* hotspot.log */
	4 << 10,		/* lighttpd.log */
	64 << 10,		/* status.log */
	4 << 10,		/* fastcgi.log */
};
#elif defined ( CONFIG_CIBOX )
static int ring_buffer_sizes[] = {
	32 << 10,		/* daemon.log */
	32 << 10,		/* kern.log */
	32 << 10,		/* vodsl_proto.log */
	32 << 10,		/* vodsl.log */
	16 << 10,		/* messages */
	16 << 10,		/* syslog */
	16 << 10,		/* debug */
	16 << 10,		/* vodsl_events.log */
	0 << 10,		/* hotspot.log */
	0 << 10,		/* lighttpd.log */
	32 << 10,		/* status.log */
	4 << 10,		/* fastcgi.log */
};
#elif defined ( CONFIG_BOARD_NEUFBOX5 )
static int ring_buffer_sizes[] = {
	128 << 10,		/* daemon.log */
	128 << 10,		/* kern.log */
	128 << 10,		/* fastcgi.log */
	128 << 10,		/* lighttpd.log */
	128 << 10,		/* messages */
	128 << 10,		/* syslog */
	128 << 10,		/* voip reserved 1 */
	128 << 10,		/* voip reserved 2 */
	128 << 10,		/* voip reserved 3 */
	128 << 10,		/* hotspot */
};
#endif

static inline size_t __ring_buffer_len(struct ring_buffer_device *dev)
{
	return (size_t) (dev->in - dev->out);
}

static size_t __ring_buffer_get(struct ring_buffer_device *dev,
				char __user * buf, size_t len)
{
	size_t l;

	/* max len: current unread size */
	len = min(len, __ring_buffer_len(dev) - (size_t) (dev->pos - dev->out));
	l = min(len, dev->size - (size_t) (dev->pos & (dev->size - 1)));

	/* first get the data from fifo->out until the end of the buffer */
	copy_to_user(buf, dev->buffer + (dev->pos & (dev->size - 1)), l);
	/* then get the rest (if any) from the beginning of the buffer */
	copy_to_user(buf + l, dev->buffer, len - l);

	return len;
}

static size_t __ring_buffer_put(struct ring_buffer_device *dev,
				const char __user * buf, size_t len)
{
	size_t l;

	/* max len: buffer size */
	len = min(len, dev->size);
	l = min(len, dev->size - (size_t) (dev->in & (dev->size - 1)));

	/* first put the data starting from fifo->in to buffer end */
	copy_from_user(dev->buffer + (dev->in & (dev->size - 1)), buf, l);
	/* then put the rest (if any) at the beginning of the buffer */
	copy_from_user(dev->buffer, buf + l, len - l);

	dev->in += len;

	if (__ring_buffer_len(dev) > dev->size) {
		dev->out = dev->in - dev->size;
	}

	return len;
}

static inline size_t ring_buffer_len(struct ring_buffer_device *dev)
{
	size_t ret;

	down(&dev->lock);

	ret = __ring_buffer_len(dev);

	up(&dev->lock);

	return ret;
}

static int ring_buffer_open(struct inode *inode, struct file *file)
{
	struct ring_buffer_device *dev;	/* device information */

	dev = container_of(inode->i_cdev, struct ring_buffer_device, cdev);

	file->private_data = dev;	/* for other methods */

	down(&dev->lock);

	dev->pos = dev->out;

	up(&dev->lock);

	return 0;		/* success */
}

static int ring_buffer_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t ring_buffer_read(struct file *file, char __user * buf,
				size_t count, loff_t * f_pos)
{
	struct ring_buffer_device *dev = file->private_data;
	size_t ret;

	down(&dev->lock);

	ret = __ring_buffer_get(dev, buf, count);
	dev->pos += ret;

	up(&dev->lock);

	*f_pos += ret;

	return ret;
}

static ssize_t ring_buffer_write(struct file *file, const char __user * buf,
				 size_t count, loff_t * f_pos)
{
	struct inode *inode = file->f_dentry->d_inode;
	struct ring_buffer_device *dev = file->private_data;
	size_t ret;

	down(&dev->lock);

	ret = __ring_buffer_put(dev, buf, count);

	i_size_write(inode, dev->in - dev->out);

	up(&dev->lock);

	*f_pos += ret;

	return ret;
}

static loff_t ring_buffer_llseek(struct file *file, loff_t off, int whence)
{
	struct ring_buffer_device *dev = file->private_data;

	switch (whence) {
	case 0 /* SEEK_SET */ :
		down(&dev->lock);

		dev->pos = dev->out + off;

		up(&dev->lock);
		break;

	case 1 /* SEEK_CUR */ :
		down(&dev->lock);

		dev->pos += off;

		up(&dev->lock);
		break;

	case 2 /* SEEK_END */ :
		down(&dev->lock);

		dev->pos = dev->in + off;

		up(&dev->lock);
		break;

	default:		/* can't happen */
		return -EINVAL;
	}

	down(&dev->lock);

	file->f_pos = (dev->pos - dev->out);

	up(&dev->lock);

	return file->f_pos;
}

static int ring_buffer_proc_read(char *page, char **start, off_t off, int count,
				 int *eof, void *data)
{
	char *page_current = page;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(ring_buffer_sizes); i++) {
		struct ring_buffer_device *dev = ring_buffer_devices + i;

		page_current +=
		    sprintf(page_current, " %u = %zu/%zu\n", i,
			    ring_buffer_len(dev), dev->size);
	}

	return (page_current - page);
}

static int ring_buffer_proc_write(struct file *file, const char *buffer,
				  unsigned long count, void *data)
{
	int i;

	if (sscanf(buffer, "%d", &i) == 1) {
		if (i >= 0 && i < ARRAY_SIZE(ring_buffer_sizes)) {
			struct ring_buffer_device *dev =
			    ring_buffer_devices + i;

			down(&dev->lock);

			dev->in = dev->out = 0;

			up(&dev->lock);
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(ring_buffer_sizes); i++) {
			struct ring_buffer_device *dev =
			    ring_buffer_devices + i;

			down(&dev->lock);

			dev->in = dev->out = 0;

			up(&dev->lock);
		}
	}

	return count;
}

struct file_operations ring_buffer_fops = {
	.owner = THIS_MODULE,
	.open = ring_buffer_open,
	.release = ring_buffer_release,
	.llseek = ring_buffer_llseek,
	.read = ring_buffer_read,
	.write = ring_buffer_write,
};

static int ring_buffer_setup_cdev(struct ring_buffer_device *dev, int index)
{
	dev_t devno;
	int err;

	dev->size = (ring_buffer_sizes[index]);
	dev->buffer = kmalloc(dev->size, GFP_KERNEL);
	if (!dev->buffer) {
		printk(KERN_ERR PFX "error allocating n=%d size=%zu\n", index,
		       dev->size);
		return -ENOMEM;
	}
	dev->in = dev->out = 0u;
	init_MUTEX(&dev->lock);

	cdev_init(&dev->cdev, &ring_buffer_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &ring_buffer_fops;
	devno = MKDEV(RING_BUFFER, index);
	err = cdev_add(&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR PFX "error %d adding n=%d", err, index);
		return err;
	}

	return 0;
}

static int __init ring_buffer_init(void)
{
	struct proc_dir_entry *proc;
	dev_t devno;
	int result;
	int i;

	printk(KERN_INFO PFX "setup %zu ring buffers...\n",
	       ARRAY_SIZE(ring_buffer_sizes));

	devno = MKDEV(RING_BUFFER, 0);
	if ((result =
	     register_chrdev_region(devno, ARRAY_SIZE(ring_buffer_sizes),
				    "ring_buffer")) < 0) {
		printk(KERN_ERR PFX "can't get MAJOR %d\n", RING_BUFFER);
		return result;
	}

	ring_buffer_devices =
	    kmalloc(ARRAY_SIZE(ring_buffer_sizes) *
		    sizeof(struct ring_buffer_device), GFP_KERNEL);
	if (!ring_buffer_devices) {
		result = -ENOMEM;
	}
	memset(ring_buffer_devices, 0,
	       ARRAY_SIZE(ring_buffer_sizes) *
	       sizeof(struct ring_buffer_device));
	for (i = 0; i < ARRAY_SIZE(ring_buffer_sizes); i++) {
		ring_buffer_setup_cdev(&ring_buffer_devices[i], i);
	}

	/* proc */
	proc = create_proc_entry("ring_buffer", 0444, NULL);
	if (!proc) {
		return -ENOMEM;
	}

	proc->owner = THIS_MODULE;
	proc->read_proc = ring_buffer_proc_read;
	proc->write_proc = ring_buffer_proc_write;

	return result;
}

static void __exit ring_buffer_exit(void)
{
	int i;
	dev_t devno = MKDEV(RING_BUFFER, 0);

	printk(KERN_INFO PFX "cleanup...\n");

	/* Get rid of our char dev entries */
	if (ring_buffer_devices) {
		for (i = 0; i < ARRAY_SIZE(ring_buffer_sizes); i++) {
			kfree(ring_buffer_devices[i].buffer);
			cdev_del(&ring_buffer_devices[i].cdev);
		}
		kfree(ring_buffer_devices);
	}

	remove_proc_entry("ring_buffer", NULL);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, ARRAY_SIZE(ring_buffer_sizes));
}

module_init(ring_buffer_init);
module_exit(ring_buffer_exit);

MODULE_AUTHOR("Miguel GAIO");
MODULE_DESCRIPTION("neufbox Ring Buffer");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");
