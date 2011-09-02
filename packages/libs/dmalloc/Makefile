# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12228 2008-08-06 22:43:15Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=dmalloc
PKG_VERSION:=5.5.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://dmalloc.com/releases/
PKG_MD5SUM:=f92e5606c23a8092f3d5694e8d1c932e

PATCH_DIR=./patches/$(ARCH)

include $(INCLUDE_DIR)/package.mk

CONFIGURE_ARGS += \
	--disable-cxx \
	--with-pagesize=12

define Package/dmalloc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Dmalloc library
  URL:=http://www.dmalloc.com
endef

define Package/dmalloc/description
        Dmalloc library to find leak memory
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/dmalloc.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libdmalloc.a $(1)/usr/lib/
endef

define Package/dmalloc/install
endef

$(eval $(call BuildPackage,dmalloc))
