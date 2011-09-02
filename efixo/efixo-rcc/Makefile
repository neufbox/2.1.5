#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=efixo-rcc
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/efixo-rcc
  SECTION:=efixo
  CATEGORY:=Efixo applications
  TITLE:=neufbox rcc
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
  DEPENDS:=+efixo-libnbd
endef

define Package/efixo-libnbd/description
 This package contains rcc
endef

PKG_BUILD_DEPENDS_neufbox4:=+neufbox4-efixo-rcc
PKG_BUILD_DEPENDS:= \
	$(foreach depend,$(PKG_BUILD_DEPENDS_neufbox4),TARGET_brcm63xx:$(depend))

define Build/Prepare
	true
endef

define Build/Compile
	true
endef

define Package/efixo-rcc/install
	true
endef

$(eval $(call BuildPackage,efixo-rcc))
