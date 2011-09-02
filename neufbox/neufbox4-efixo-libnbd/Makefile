#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=neufbox4-efixo-libnbd
PKG_VERSION:=2.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://packages.efixo.net/
PKG_MD5SUM:=2b6bcf8aa784f26cfac9d46fa732af18

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/neufbox4-efixo-libnbd
  SECTION:=neufbox4
  CATEGORY:=neufbox4 binary packages
  TITLE:=neufbox4-efixo-libnbd package
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=@TARGET_brcm63xx
endef

define Package/neufbox4-efixo-libnbd/description
  neufbox4-efixo-libnbd binary package
endef

define Build/Compile
	true
endef

define Build/InstallDev
	$(CP) $(PKG_BUILD_DIR)/install/* $(1)/
endef

define Package/neufbox4-efixo-libnbd/install
	$(CP) $(PKG_BUILD_DIR)/install/* $(1)/
endef

$(eval $(call BuildPackage,neufbox4-efixo-libnbd))
