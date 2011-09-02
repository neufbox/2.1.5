#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=neufbox4-efixo-libestuff
PKG_VERSION:=2.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://packages.efixo.net/
PKG_MD5SUM:=f6f2a0a33fbcf79d336272ffa6322e57

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/neufbox4-efixo-libestuff
  SECTION:=neufbox4
  CATEGORY:=neufbox4 binary packages
  TITLE:=neufbox4-efixo-libestuff package
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=@TARGET_brcm63xx +efixo-libnbd
endef

define Package/neufbox4-efixo-libestuff/description
  neufbox4-efixo-libestuff binary package
endef

define Build/Compile
	true
endef

define Package/neufbox4-efixo-libestuff/install
	$(CP) $(PKG_BUILD_DIR)/install/* $(1)/
	ln -sf libestuff.so.0.0.0 $(1)/usr/lib/libestuff.so.0
	ln -sf libestuff.so.0.0.0 $(1)/usr/lib/libestuff.so
endef

$(eval $(call BuildPackage,neufbox4-efixo-libestuff))
