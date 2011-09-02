#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=octeon
BOARDNAME:=Cavium Networks Octeon
FEATURES:=jffs2 usb pci pcmcia gpio
SUBTARGETS=openwrt nb5

include $(INCLUDE_DIR)/target.mk

define Target/Description
	Build firmware images for Cavium Networks Octeon-based boards.
endef

$(eval $(call BuildTarget))
