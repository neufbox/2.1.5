#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=neufbox4-efixo-libecom
PKG_VERSION:=2.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://packages.efixo.net/
PKG_MD5SUM:=e43fc837089a5b6c06499381128867e7

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/neufbox4-efixo-libecom
  SECTION:=neufbox4
  CATEGORY:=neufbox4 binary packages
  TITLE:=neufbox4-efixo-libecom package
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=@TARGET_brcm63xx +efixo-libnbd
endef

define Package/neufbox4-efixo-libecom/description
  neufbox4-efixo-libecom binary package
endef

define Build/Compile
	true
endef

define Package/neufbox4-efixo-libecom/install
	$(CP) $(PKG_BUILD_DIR)/install/* $(1)/
	ln -sf libecom.so.0.0.0 $(1)/usr/lib/libecom.so.0
	ln -sf libecom.so.0.0.0 $(1)/usr/lib/libecom.so
endef

$(eval $(call BuildPackage,neufbox4-efixo-libecom))
