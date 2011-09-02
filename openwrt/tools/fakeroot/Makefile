# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=fakeroot
PKG_VERSION:=1.9.5

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/fakeroot
PKG_MD5SUM:=95c0c78c734b9782ec8d47e99c8e3fc8

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
	$(call Host/Configure/Default,)
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
endef

define Host/Clean
	$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(MAKE) -C $(HOST_BUILD_DIR) clean
	$(call Host/Clean/Default)
endef

$(eval $(call HostBuild))
