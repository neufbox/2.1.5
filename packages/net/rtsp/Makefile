# 
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 11868 2008-07-18 22:57:30Z nbd $

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ipt-rtsp
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ipt-rtsp
  SUBMENU:=Netfilter Extensions
  TITLE:=RTSP Conntrack and NAT helpers
  DEPENDS:= kmod-ipt-core +kmod-ipt-nat
  FILES:=$(PKG_BUILD_DIR)/nf_conntrack_rtsp.$(LINUX_KMOD_SUFFIX) \
	 $(PKG_BUILD_DIR)/nf_nat_rtsp.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,45,nf_conntrack_rtsp nf_nat_rtsp)
endef

define KernelPackage/ipt-rtsp/description
Kernel RTSP Conntrack and NAT module
endef

define Build/Prepare
	$(CP) src/* $(PKG_BUILD_DIR)
	$(call Build/Prepare/Default)
endef

KERNEL_MAKEOPTS := -C $(LINUX_DIR) \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	KBUILD_HAVE_NLS=no \
	CONFIG_SHELL="$(BASH)"

define Build/Compile
	$(MAKE_VARS) $(MAKE) $(KERNEL_MAKEOPTS) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,ipt-rtsp))
