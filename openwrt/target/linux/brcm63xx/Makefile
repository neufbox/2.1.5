#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=brcm63xx
BOARDNAME:=Broadcom BCM63xx
FEATURES:=squashfs jffs2 usb pci pcmcia gpio
SUBTARGETS=generic neufbox4

INITRAMFS_EXTRA_FILES:=$(TOPDIR)/target/linux/$(BOARD)/image/initramfs-base-files.txt

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += gpioctl

define Target/Description
	Build firmware images for Broadcom based xDSL/routers
	currently supports BCM6348 and BCM6358 based devices.
	(e.g. Inventel Livebox, Siemens SE515, neufbox4)
endef

$(eval $(call BuildTarget))
