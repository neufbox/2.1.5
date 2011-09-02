#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=efixo-libnbd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/efixo-libnbd
  SECTION:=efixo
  CATEGORY:=Efixo applications
  TITLE:=neufbox NBD library
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=+efixo-headers +efixo-libnbu
endef

define Package/efixo-libnbd/description
 This package contains nbd library
endef

PKG_BUILD_DEPENDS_neufbox4:=+neufbox4-efixo-libnbd
PKG_BUILD_DEPENDS:= \
	$(foreach depend,$(PKG_BUILD_DEPENDS_neufbox4),TARGET_brcm63xx:$(depend))

define Build/Prepare
	true
endef

define Build/Compile
	true
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/
	$(CP) include/* $(STAGING_DIR)/usr/include/
endef

define Package/efixo-libnbd/install
	true
endef

$(eval $(call BuildPackage,efixo-libnbd))
