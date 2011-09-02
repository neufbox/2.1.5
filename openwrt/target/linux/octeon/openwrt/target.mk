ARCH:=mips
BOARDNAME:=Cavium Octeon
FEATURES:=squashfs jffs2 usb atm pci
LINUX_VERSION:=2.6.30
ifeq ($(TARGET_BUILD),1)
FILES_DIR:=files
endif
CFLAGS:=-Os -pipe -funit-at-a-time

DEFAULT_PACKAGES += hostapd-mini kmod-switch kmod-ath9k

define Target/Description
	(Cavium Octeon platform)
endef

$(eval $(call BuildTarget))
