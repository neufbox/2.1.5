#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=firmwarehotplug
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/linux-hotplug
PKG_MD5SUM:=e5cf352b172f498e27b160167b981f24

include $(INCLUDE_DIR)/package.mk

# workaround for fedora sdcc naming changes
ifneq ($(wildcard /usr/libexec/sdcc),)
  export PATH:=$(PATH):/usr/libexec
endif

define Package/firmwarehotplug
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@USB_SUPPORT +fxload
  TITLE:=Hotplug firmware for EZ-USB microcontrollers
  URL:=http://linux-hotplug.sourceforge.net/
endef

define Build/Compile
	$(call Build/Compile/Default, \
		OPT="$(TARGET_CFLAGS)" \
		-C $(PKG_BUILD_DIR)/ezusb/midi/original \
		DESTDIR="$(PKG_INSTALL_DIR)" \
                all install \
	)
endef

define Package/firmwarehotplug/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/usb
	install -m755 $(PKG_BUILD_DIR)/ezusb/midi/original/Hotplug/ezusbmidi $(1)/etc/hotplug.d/usb
	install -m644 $(PKG_BUILD_DIR)/ezusb/midi/original/Hotplug/ezusbmidi.usermap $(1)/etc/hotplug.d
	$(INSTALL_DIR) $(1)/usr/share/usb/ezusbmidi
	install -m644 $(PKG_BUILD_DIR)/ezusb/midi/original/ezusbmidi1x1.ihx $(1)/usr/share/usb/ezusbmidi
	install -m644 $(PKG_BUILD_DIR)/ezusb/midi/original/ezusbmidi2x2.ihx $(1)/usr/share/usb/ezusbmidi
endef

define Require/working-sdcc
	sdcc -v 2>&1 | awk '($$$$1 == "SDCC") && ($$$$3 == "mcs51/gbz80/z80/avr/ds390/pic16/pic14/TININative/xa51/ds400/hc08") { print "ok" }' | grep ok > /dev/null
endef

$(eval $(call RequireCommand,sdcc, \
        $(PKG_NAME) requires the SDCC Cross Compiler (sdcc). \
))

$(eval $(call Require,working-sdcc, \
        $(PKG_NAME) requires a SDCC Cross Compiler (sdcc) that supports the following targets : \
	mcs51/gbz80/z80/avr/ds390/pic16/pic14/TININative/xa51/ds400/hc08. You might consider \
	installing the non-free version. \
))

$(eval $(call BuildPackage,firmwarehotplug))

