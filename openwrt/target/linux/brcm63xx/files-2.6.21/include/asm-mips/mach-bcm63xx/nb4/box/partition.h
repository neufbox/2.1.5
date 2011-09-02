/*
 *      neufbox/flash.h
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

#ifndef _NEUFBOX_FLASH_H_
#define _NEUFBOX_FLASH_H_

#define kB(X) ( (X) << 10 )
#define MB(X) ( (X) << 20 )
#define NEUFBOX_IMAGE_TAG_SIZE 0x100

#define SERIALIZATION_BASE ( 0x0000ff80 )
#define BOOTLOADER_BASE ( 0x0000ffb0 )

/*
 *       NEUFBOX 4 & 5 MAPPING
 * 
 * +----------------------------------+
 * |  Partition mapping               |
 * |                                  |
 * +-0x00000000-----------------------+
 * |            bootloader            | 64 kb
 * +-0x00010000-----------------------+
 * |            main firmware         | 5.3125 Mb
 * +-0x00560000-----------------------+
 * |            read/write partition  | 640 kb
 * +-0x00600000-----------------------+
 * |            rescue firmware       | 1.5 Mb
 * +-0x00780000-----------------------+
 * |            read only partition   | 448 kb
 * +-0x007f0000-----------------------+
 * |            nvram                 | 64 kb
 * +-0x007f0000-----------------------+
 *
 * Be careful, partitions must be aligned on flash blocks (64k)
 */

#define NEUFBOX_FLASH_SIZE		( MB(8ul) )

#define NEUFBOX_BOOTLOADER_OFFSET	( 0ul )
#define NEUFBOX_BOOTLOADER_SIZE		( kB(64ul) )

#define NEUFBOX_MAINFIRMWARE_OFFSET	( NEUFBOX_BOOTLOADER_OFFSET + NEUFBOX_BOOTLOADER_SIZE )
#define NEUFBOX_MAINFIRMWARE_SIZE	( MB(5ul) + kB(320ul) )

#define NEUFBOX_JFFS2_OFFSET		( NEUFBOX_MAINFIRMWARE_OFFSET + NEUFBOX_MAINFIRMWARE_SIZE )
#define NEUFBOX_JFFS2_SIZE		( kB(640ul) )

#define NEUFBOX_RESCUEFIRMWARE_OFFSET	( NEUFBOX_JFFS2_OFFSET + NEUFBOX_JFFS2_SIZE )
#define NEUFBOX_RESCUEFIRMWARE_SIZE	( MB(1ul) + kB(512ul) )

#define NEUFBOX_ADSLFIRMWARE_OFFSET	( NEUFBOX_RESCUEFIRMWARE_OFFSET + NEUFBOX_RESCUEFIRMWARE_SIZE )
#define NEUFBOX_ADSLFIRMWARE_SIZE	( kB(448ul) )

#define NEUFBOX_NVRAM_OFFSET		( NEUFBOX_ADSLFIRMWARE_OFFSET + NEUFBOX_ADSLFIRMWARE_SIZE )
#define NEUFBOX_NVRAM_SIZE		( kB(64ul) )

#define NEUFBOX_NVRAM_SYSTEM_START	( 0ul )
#define NEUFBOX_NVRAM_SYSTEM_SIZE	( kB(32ul) )
#define NEUFBOX_NVRAM_USER_START	( NEUFBOX_NVRAM_SYSTEM_START + NEUFBOX_NVRAM_SYSTEM_SIZE )
#define NEUFBOX_NVRAM_USER_SIZE		( kB(32ul) - 0x100 )
#define NEUFBOX_BOOT_TAG_START		( NEUFBOX_NVRAM_USER_START + NEUFBOX_NVRAM_USER_SIZE )
#define NEUFBOX_BOOT_TAG_SIZE		( 0x100 )

#ifdef CONFIG_NEUFBOX_MAIN
#define NEUFBOX_ROOTFS_OFFSET		( NEUFBOX_MAINFIRMWARE_OFFSET + NEUFBOX_IMAGE_TAG_SIZE )
#define NEUFBOX_ROOTFS_SIZE		( NEUFBOX_MAINFIRMWARE_SIZE - NEUFBOX_IMAGE_TAG_SIZE )
#endif
#ifdef CONFIG_NEUFBOX_RESCUE
#define NEUFBOX_ROOTFS_OFFSET		( NEUFBOX_RESCUEFIRMWARE_OFFSET + NEUFBOX_IMAGE_TAG_SIZE )
#define NEUFBOX_ROOTFS_SIZE		( NEUFBOX_RESCUEFIRMWARE_SIZE - NEUFBOX_IMAGE_TAG_SIZE )
#endif


#endif				/* _NEUFBOX_FLASH_H_ */
