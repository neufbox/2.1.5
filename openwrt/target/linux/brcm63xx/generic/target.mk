BOARDNAME:=Broadcom BCM63xx
FEATURES:=squashfs jffs2 usb atm pci
LINUX_VERSION:=2.6.27.22
ifeq ($(TARGET_BUILD),1)
FILES_DIR:=files
endif

DEFAULT_PACKAGES += hostapd-mini kmod-switch

define Target/Description
	Build firmware images for Broadcom based xDSL/routers
	currently supports BCM6348 and BCM6358 based devices.
	(e.g. Inventel Livebox, Siemens SE515, neufbox4)
endef

$(eval $(call BuildTarget))
