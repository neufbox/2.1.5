#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dwc_otg
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk


define KernelPackage/dwc_otg
  SUBMENU:=Other modules
  DEPENDS:=+kmod-usb-core
  KCONFIG:=
  TITLE:=Synopsys DesignWare Cores Hi-Speed USB On-The-Go (OTG)
  FILES:=$(PKG_BUILD_DIR)/dwc_otg.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,80,dwc_otg)
endef

define KernelPackage/ucmb/description
	The DWC_otg core is a Dual-Role Device (DRD) controller, which
	supports both device and host functions and is fully compliant with
	the On-The-Go Supplement to the USB 2.0 Specification, Revision 1.0a.
	It can also be configured as a Host-only or Device-only controller,
	fully compliant with the USB 2.0 Specification. DWC_otg USB 2.0
	configurations support high-speed (HS, 480-Mbps), full-speed
	(FS, 12-Mbps), and low-speed (LS, 1.5-Mbps) transfers. Additionally,
	DWC_otg can be configured as a USB 1.1 full-speed/low-speed DRD. The
	DWC_otg core connects to the industry-standard AMBA High-Performance
	Bus (AHB) for communicating with the application and system memory,
	and is fully compliant with the AMBA Specification, Revision 2.0.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./driver/* $(PKG_BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CONFIG_USB_DWC_OTG=m \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="-DDWC_HOST_ONLY" \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,dwc_otg))

