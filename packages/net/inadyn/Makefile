#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12159 2008-08-05 20:50:12Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=inadyn
PKG_VERSION:=v1.96.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=http://www.inatech.eu/inadyn/
PKG_MD5SUM:=702a127f13b96eee38d60c6f30b20efa

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/inadyn
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Inadyn is a free DynDNS  client
  URL:=http://www.inatech.eu/inadyn/
  DEPENDS:=+efixo-libnbd
endef

define Package/inadyn/description
	inadyn - a client for open DNS servers. 
endef

MAKE_VARS += ARCH_SPECIFIC_LIBS="-L$(STAGING_DIR)/usr/lib -lnbu -lnbd"

define Package/inadyn/install	
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linux/inadyn $(1)/usr/sbin
endef

$(eval $(call BuildPackage,inadyn))
