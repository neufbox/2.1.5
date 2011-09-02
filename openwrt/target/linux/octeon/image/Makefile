#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Build/Initramfs
	$(CP) $(BIN_DIR)/openwrt-$(BOARD)-vmlinux.elf $(BIN_DIR)/openwrt-$(BOARD)
	rm -f $(BIN_DIR)/openwrt-$(BOARD).lzma && /usr/bin/lzma $(BIN_DIR)/openwrt-$(BOARD)
	rm -f $(BIN_DIR)/openwrt-$(BOARD)
	$(STAGING_DIR_HOST)/bin/mkfirmware-nb5 \
		$(BIN_DIR)/openwrt-$(BOARD).lzma \
		$(BIN_DIR)/$(FIRMWARE_RELEASE)
	mv -f $(BIN_DIR)/$(FIRMWARE_RELEASE) $(BIN_DIR)/openwrt-$(BOARD)-nb5
endef

$(eval $(call BuildImage))
