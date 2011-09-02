#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id:$

include $(TOPDIR)/rules.mk

PKG_NAME:=yaddns
PKG_REV:=44
PKG_VERSION:=r$(PKG_REV)
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=svn://patatrac.info/yaddns/trunk/
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=svn

include $(INCLUDE_DIR)/package.mk

define Package/yaddns
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+efixo-libnbd
  TITLE:=Yet Another DDNS client
  URL:=http://patatrac.info/yaddns
endef

define Package/yaddns/description
  This package contains the yaddns client
endef

define Package/yaddns/install
	$(MAKE) -C $(PKG_BUILD_DIR) ROOT="$(1)" install
endef

$(eval $(call BuildPackage,yaddns))
