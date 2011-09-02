
NETWORK_DEVICES_MENU:=Network Devices

define KernelPackage/broadcom-xtmrt
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Broadcom XTMRT driver
  FILES:=$(LINUX_DIR)/drivers/net/bcmxtmrt.$(LINUX_KMOD_SUFFIX)
  KCONFIG:=CONFIG_BCM_XTMRT
  DEPENDS:=@LINUX_2_6 @TARGET_brcm63xx
endef

define KernelPackage/broadcom-xtmrt/description
 Kernel modules for Broadcom XTMRT.
endef
$(eval $(call KernelPackage,broadcom-xtmrt))

