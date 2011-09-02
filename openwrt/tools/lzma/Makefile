# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=lzma
PKG_VERSION:=443

PKG_SOURCE:=$(PKG_NAME)$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/sevenzip
PKG_MD5SUM:=c4e1b467184c7cffd4371c74df2baf0f

include $(INCLUDE_DIR)/host-build.mk

HOST_UNPACK:=bzcat $(DL_DIR)/$(PKG_SOURCE) | $(TAR) -C $(HOST_BUILD_DIR) $(TAR_OPTIONS)

LIB_DIR=$(HOST_BUILD_DIR)/C/7zip/Compress/LZMA_C
ALONE_DIR=$(HOST_BUILD_DIR)/C/7zip/Compress/LZMA_Alone

define Host/Prepare
	$(call Host/Prepare/Default)
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/include/sqlma
	$(CP) files/*.h $(STAGING_DIR_HOST)/include/sqlma
endef

define Host/Compile
	cd $(LIB_DIR) && $(MAKE) -f sqlzma.mk Sqlzma=$(STAGING_DIR_HOST)/include/sqlma
	cd $(ALONE_DIR) && $(MAKE) -f sqlzma.mk Sqlzma=$(STAGING_DIR_HOST)/include/sqlma
endef

define Host/Install
	$(INSTALL_DATA) $(LIB_DIR)/libunlzma.a $(STAGING_DIR_HOST)/lib/
	$(INSTALL_DATA) $(LIB_DIR)/libunlzma_r.a $(STAGING_DIR_HOST)/lib/
	$(INSTALL_DATA) $(ALONE_DIR)/liblzma_r.a $(STAGING_DIR_HOST)/lib/
	$(INSTALL_BIN) $(ALONE_DIR)/lzma $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f \
		$(STAGING_DIR_HOST)/lib/liblzma.a \
		$(STAGING_DIR_HOST)/bin/lzma
endef

$(eval $(call HostBuild))
