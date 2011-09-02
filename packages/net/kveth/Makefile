# 
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 11868 2008-07-18 22:57:30Z nbd $

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=kveth
PKG_RELEASE:=1

NETWORK_DEVICES_MENU:=Network Devices

include $(INCLUDE_DIR)/package.mk

define KernelPackage/kveth
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Kernel Veth
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/veth.ko
  AUTOLOAD:=$(call AutoLoad,20,veth)
endef

define KernelPackage/kveth/description
Kernel veth
endef

define Build/Prepare
	$(CP) src/* $(PKG_BUILD_DIR)
	$(call Build/Prepare/Default)
endef

KERNEL_MAKEOPTS := -C $(LINUX_DIR) \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	CONFIG_SHELL="$(BASH)"

define Build/Compile
	$(MAKE_VARS) $(MAKE) $(KERNEL_MAKEOPTS) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,kveth))
