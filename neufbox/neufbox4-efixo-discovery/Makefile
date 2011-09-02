#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=neufbox4-efixo-discovery
PKG_VERSION:=2.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://packages.efixo.net/
PKG_MD5SUM:=4d197481675e760d7137ec5cbbfc9f1a

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/neufbox4-efixo-discovery
  SECTION:=neufbox4
  CATEGORY:=neufbox4 binary packages
  TITLE:=neufbox4-efixo-discovery package
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=+efixo-libnbd +efixo-libnbu
endef

define Package/neufbox4-efixo-discovery/description
  neufbox4-efixo-discovery binary package
endef

define Build/Compile
	true
endef

define Package/neufbox4-efixo-discovery/install
	$(CP) $(PKG_BUILD_DIR)/install/* $(1)/
endef

$(eval $(call BuildPackage,neufbox4-efixo-discovery))
