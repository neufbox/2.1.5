# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 4439 2006-08-04 17:14:36Z nbd $

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME := neufbox-mkfirmware
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

all: install

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	touch $@

$(PKG_BUILD_DIR)/mkfirmware: $(PKG_BUILD_DIR)/.prepared
	KERNEL_DIR=$(LINUX_DIR) \
		$(MAKE) -C $(PKG_BUILD_DIR)

download: 
prepare: $(PKG_BUILD_DIR)/.prepared
compile: $(PKG_BUILD_DIR)/mkfirmware

ifneq ($(TARGET),)
install: compile
	$(INSTALL_DIR) $(TARGET)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mkfirmware $(TARGET)
endif

clean:
	rm -rf $(PKG_BUILD_DIR)
