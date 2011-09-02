/*
 * neufbox4-flash.c
 *
 * Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
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
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>

#include <asm/io.h>

#include <box/partition.h>
#include <bcmTag.h>

#define BUSWIDTH 2		/* Buswidth */
#define EXTENDED_SIZE 0xBFC00000

#define PFX KBUILD_MODNAME ": "

static struct mtd_info *mymtd;

static struct map_info neufbox4_map = {
	.name = "neufbox4",
	.bankwidth = BUSWIDTH,
};

static struct mtd_partition neufbox4_mtd_parts[] = {
	{
	 .name = "bootloader",
	 .offset = NEUFBOX_BOOTLOADER_OFFSET,
	 .size = NEUFBOX_BOOTLOADER_SIZE,
	 },
	{
	 .name = "main firmware",
	 .offset = NEUFBOX_MAINFIRMWARE_OFFSET,
	 .size = NEUFBOX_MAINFIRMWARE_SIZE,
	 },
	{
	 .name = "local data (jffs2)",
	 .offset = NEUFBOX_JFFS2_OFFSET,
	 .size = NEUFBOX_JFFS2_SIZE,
	 },
	{
	 .name = "rescue firmware",
	 .offset = NEUFBOX_RESCUEFIRMWARE_OFFSET,
	 .size = NEUFBOX_RESCUEFIRMWARE_SIZE,
	 },
	{
	 .name = "adsl driver",
	 .offset = NEUFBOX_ADSLFIRMWARE_OFFSET,
	 .size = NEUFBOX_ADSLFIRMWARE_SIZE,
	 },
	{
	 .name = "nvram",
	 .offset = NEUFBOX_NVRAM_OFFSET,
	 .size = NEUFBOX_NVRAM_SIZE,
	 },
	{
	 .name = "rootfs",
	 .offset = 0,		/* runtime filled */
	 .size = 0,		/* runtime filled */
	 .mask_flags = MTD_WRITEABLE	/* force read-only */
	 },
	{
	 .name = "Flash",
	 .offset = 0,
	 .size = NEUFBOX_FLASH_SIZE,
	 },
};

static int parse_cfe_partitions(struct mtd_info *master,
				struct mtd_partition *pparts)
{
	int ret;
	size_t retlen;
	unsigned int rootfsaddr, rootfslen;
	struct _FILE_TAG tag;

	/* Get the tag */
	ret =
#ifdef CONFIG_NEUFBOX_MAIN		
	    master->read(master, NEUFBOX_MAINFIRMWARE_OFFSET, sizeof(tag), &retlen,
			 (void *)&tag);
#else /* CONFIG_NEUFBOX_RESCUE */
            master->read(master, NEUFBOX_RESCUEFIRMWARE_OFFSET, sizeof(tag), &retlen,
			 (void *)&tag);
#endif
	if (retlen != sizeof(tag)) {
		return -EIO;
	}
	printk(KERN_INFO PFX
	       "CFE boot tag found with version %s and board type %s.\n",
	       tag.tagVersion, tag.boardId);

	/* Get the values and calculate */
	sscanf(tag.rootfsAddress, "%u", &rootfsaddr);
	rootfsaddr = rootfsaddr - EXTENDED_SIZE;
	sscanf(tag.rootfsLen, "%u", &rootfslen);

	pparts[6].offset = rootfsaddr;
	pparts[6].size = rootfslen;

	return 0;
}

static int neufbox4_probe(struct platform_device *pdev)
{
	struct resource *r;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	neufbox4_map.phys = r->start;
	neufbox4_map.size = (r->end - r->start) + 1;
	neufbox4_map.virt = ioremap(neufbox4_map.phys, neufbox4_map.size);

	if (!neufbox4_map.virt) {
		printk(KERN_ERR PFX "ioremap(%zx, %lu) failed\n",
		       neufbox4_map.phys, neufbox4_map.size);
		return -EIO;
	}

	printk(KERN_INFO PFX "0x%08lx at 0x%zx\n",
	       neufbox4_map.size, neufbox4_map.phys);

	simple_map_init(&neufbox4_map);

	mymtd = do_map_probe("cfi_probe", &neufbox4_map);
	if (!mymtd) {
		printk(KERN_ERR "CFI probe failed\n");
		iounmap(neufbox4_map.virt);

		return -ENXIO;
	}

	mymtd->owner = THIS_MODULE;

	if (parse_cfe_partitions(mymtd, neufbox4_mtd_parts)) {
		iounmap(neufbox4_map.virt);

		return -EIO;
	}

	return add_mtd_partitions(mymtd, neufbox4_mtd_parts,
				  ARRAY_SIZE(neufbox4_mtd_parts));
}

static int neufbox4_remove(struct platform_device *pdev)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
	}

	if (neufbox4_map.virt) {
		iounmap(neufbox4_map.virt);
	}

	return 0;
}

static struct platform_driver neufbox4_mtd_dev = {
	.probe = neufbox4_probe,
	.remove = neufbox4_remove,
	.driver = {
		   .name = "neufbox4-flash",
		   .owner = THIS_MODULE,
		   },
};

static int __init neufbox4_mtd_init(void)
{
	return platform_driver_register(&neufbox4_mtd_dev);
}

static void __exit neufbox4_mtd_exit(void)
{
	platform_driver_unregister(&neufbox4_mtd_dev);
}

module_init(neufbox4_mtd_init);
module_exit(neufbox4_mtd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miguel GAIO <miguel.gaio@efixo.com>");
MODULE_DESCRIPTION("neufbox MTD map driver");
