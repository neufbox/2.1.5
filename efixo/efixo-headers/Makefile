#
# Copyright (C) 2006-2010 Efixo
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=efixo-headers
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/efixo-headers
  SECTION:=efixo
  CATEGORY:=Efixo applications
  TITLE:=Neufbox headers
  MAINTAINER:=Efixo
  URL:=http://dev.efixo.net/
endef

define Package/efixo-headers/description
 This package contains all headers to build neufbox firmware.
endef

ifeq ($(strip $(BOX)),nb4)
MACH := bcm63xx
endif

define Build/Prepare
	true
endef

define Build/Compile
	true
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/kernel/linux
	-$(CP) $(LINUX_DIR)/include/linux/sff-8472.h $(STAGING_DIR)/usr/include/kernel/linux/
	-$(CP) $(LINUX_DIR)/include/linux/broadcom-switch.h $(STAGING_DIR)/usr/include/kernel/linux/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/kernel/box
	$(CP) $(LINUX_DIR)/include/asm-mips/mach-$(MACH)/*.h $(STAGING_DIR)/usr/include/kernel/
	$(CP) $(LINUX_DIR)/include/asm-mips/mach-$(MACH)/$(BOX)/box/*.h $(STAGING_DIR)/usr/include/kernel/box/
	-$(CP) $(LINUX_DIR)/include/asm-mips/mach-generic/neufbox/*.h $(STAGING_DIR)/usr/include/kernel/box
	$(call Build/Prepare/Default)
endef

define Package/efixo-headers/install
	true
endef

$(eval $(call BuildPackage,efixo-headers))
