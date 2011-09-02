#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=neufbox4-efixo-api
PKG_VERSION:=2.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://packages.efixo.net/
PKG_MD5SUM:=fad41e87a2b0a8aaef721702a2e3b070

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/neufbox4-efixo-api
  SECTION:=neufbox4
  CATEGORY:=neufbox4 binary packages
  TITLE:=neufbox4-efixo-api package
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=@TARGET_brcm63xx
endef

define Package/neufbox4-efixo-api/description
  neufbox4-efixo-api binary package
endef

define Build/Compile
	true
endef

define Package/neufbox4-efixo-api/install
    $(CP) $(PKG_BUILD_DIR)/install/* $(1)/
endef

$(eval $(call BuildPackage,neufbox4-efixo-api))
