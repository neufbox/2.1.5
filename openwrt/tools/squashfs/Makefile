# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=squashfs
PKG_VERSION:=3.4

PKG_SOURCE:=$(PKG_NAME)$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/squashfs
PKG_MD5SUM:=2a4d2995ad5aa6840c95a95ffa6b1da6
PKG_CAT:=zcat

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)/squashfs-tools \
		LzmaC=$(STAGING_DIR_HOST)/lib \
		LzmaAlone=$(STAGING_DIR_HOST)/lib \
		Sqlzma=$(STAGING_DIR_HOST)/include/sqlma
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/mksquashfs $(STAGING_DIR_HOST)/bin/mksquashfs-lzma
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/unsquashfs $(STAGING_DIR_HOST)/bin/unsquashfs-lzma
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mksquashfs-lzma
	rm -f $(STAGING_DIR_HOST)/bin/unsquashfs-lzma
endef

$(eval $(call HostBuild))
