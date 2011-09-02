#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=efixo-base-files
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/efixo-base-files/Template
  define Package/efixo-base-files-$(1)
    SECTION:=efixo
    CATEGORY:=Efixo applications
    MAINTAINER:=Efixo
    URL:=http://dev.efixo.net/
    TITLE:=neufbox $(1) base files
    DEPENDS:=$(2)
  endef

  define Package/efixo-base-files-$(1)/description
    This package contains $(1) neufbox base files
  endef

  define Package/efixo-base-files-$(1)/install
    $(CP) ./files/$(1)/* $$(1)
  endef

  $$(eval $$(call BuildPackage,efixo-base-files-$(1)))
endef

define Build/Configure
	true
endef

define Build/Compile
	true
endef

$(eval $(call Package/efixo-base-files/Template,common))
$(eval $(call Package/efixo-base-files/Template,nb4-main,@TARGET_brcm63xx +efixo-base-files-common))
$(eval $(call Package/efixo-base-files/Template,nb4-rescue,@TARGET_brcm63xx +efixo-base-files-common))
$(eval $(call Package/efixo-base-files/Template,backup-3g))
$(eval $(call Package/efixo-base-files/Template,ipv6))
