#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hasciicam
PKG_VERSION:=20080922
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=git://git.dyne.org/$(PKG_NAME).git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=262f56eb307f9a0d4bee04d38e0adde7e4257d87

include $(INCLUDE_DIR)/package.mk

define Package/hasciicam
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Live ASCII video feeds
  URL:=http://ascii.dyne.org/
  DEPENDS:= +libaa
endef


CONFIGURE_VARS += \
	LIBS="-lncurses" \

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Package/hasciicam/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/hasciicam $(1)/usr/bin/
endef

$(eval $(call BuildPackage,hasciicam))
