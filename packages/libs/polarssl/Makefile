# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=polarssl
PKG_VERSION:=0.11.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-gpl.tgz
PKG_SOURCE_URL:=http://polarssl.org/code/releases/
PKG_MD5SUM:=885d70f15d31eb736975e5473d8ea7ec

include $(INCLUDE_DIR)/package.mk

define Package/polarssl/Default
  TITLE:=Embedded SSL
  URL:=http://polarssl.org/
endef

define Package/libpolarssl/Default/description
 The aim of the PolarSSL project is to provide a quality, open-source 
 cryptographic library written in C and targeted at embedded systems.
endef

define Package/libpolarssl
$(call Package/polarssl/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (library)
endef

define Package/polarssl-progs
$(call Package/polarssl/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libpolarssl
  TITLE+= (programs)
endef

define Build/Configure
endef

TARGET_CFLAGS += $(FPIC)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		OFLAGS="$(TARGET_CFLAGS)" \
		all
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)/usr" \
		install
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/polarssl $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpolarssl.a $(1)/usr/lib/
endef

define Package/libpolarssl/install
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libpolarssl.so $(1)/usr/lib/
endef

define Package/polarssl-progs/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/polarssl_* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libpolarssl))
$(eval $(call BuildPackage,polarssl-progs))
