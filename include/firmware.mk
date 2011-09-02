
profile		:= $(strip $(or $(profile), $(call Str/Str,$(MAKECMDGOALS),refurbishing)))
profile		:= $(strip $(or $(profile), $(call Str/Str,$(MAKECMDGOALS),openwrt)))
profile		:= $(strip $(or $(profile), $(call Str/Str,$(MAKECMDGOALS),mainipv6)))
profile		:= $(strip $(or $(profile), $(call Str/Str,$(MAKECMDGOALS),main)))
profile		:= $(strip $(or $(profile), $(call Str/Str,$(MAKECMDGOALS),rescue)))

ifneq ($(profile),)
PROFILE=$(profile)
endif

BOX		:= $(strip $(or $(BOX), $(call Str/Str,$(MAKECMDGOALS),cibox)))
BOX		:= $(strip $(or $(BOX), $(call Str/Str,$(MAKECMDGOALS),nb4)))

UPROFILE	:= $(strip $(call Str/Upper,$(PROFILE)))
UBOX		:= $(strip $(call Str/Upper,$(BOX)))
FIRMWARE	:= $(BOX)-$(PROFILE)
FIRMWARE_RELEASE:= $(UBOX)-$(UPROFILE)-R$(RELEASE)

OLD_FIRMWARE:=$(shell cat .firmware 2>/dev/null)

#
# Broadcom board
#

# cibox arch
ifeq ($(BOX),cibox)
BOARD		:= brcm63xx
endif

# nb4 arch
ifeq ($(BOX),nb4)
BOARD		:= brcm63xx
endif



#
# Octeon board
#


.PHONY: .firmware
ifneq ($(OLD_FIRMWARE),$(FIRMWARE))
.firmware: include/packages.mk
	@echo "change firmware from $(OLD_FIRMWARE) to $(FIRMWARE)"
	$(call Exec,rm -rf $(OPENWRT_DIR)/tmp)
	@echo $(FIRMWARE) > $@
else
.firmware: include/packages.mk
endif

cibox-main-kernel-compile nb4-main-kernel-compile nb4-openwrt-kernel-compile: 
	$(call OpenWRT/Build,target/install)


firmware: .firmware
	$(call OpenWRT/Build)
	$(call Efixo/$(BOARD)/Install)
	$(if $(INITRAMFS),,$(call Exec,cp $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE) $(TFTPBOOT_DIR)/openwrt-$(FIRMWARE)))
	$(if $(INITRAMFS),,$(call Trace,"tftp -b 65000 -g -r openwrt-$(FIRMWARE) 192.168.22.71"))

cibox-main: firmware
nb4-openwrt nb4-main nb4-mainipv6 nb4-rescue: firmware


#
# kernel config rules
#
kernel-menuconfig: .firmware
	$(call Exec,$(CP) $(PROFILE_CONFIG) $(OPENWRT_DIR)/.config)
	$(call Exec,$(CP) $(KERNEL_CONFIG) $(TARGET_DIR)/config-$(LINUX_VERSION))
	$(call Exec,$(MAKE_VARS) $(MAKE) -C $(OPENWRT_DIR) kernel_menuconfig)
	$(call Exec,$(CP) $(LINUX_DIR)/linux-$(LINUX_VERSION_RELEASE)/.config $(KERNEL_CONFIG))

cibox-main-kernel-menuconfig: kernel-menuconfig
nb4-openwrt-kernel-menuconfig nb4-mainipv6-kernel-menuconfig nb4-main-kernel-menuconfig nb4-rescue-kernel-menuconfig: kernel-menuconfig


#
# firmware config rules
#
menuconfig: .firmware
	$(call Exec,$(OPENWRT_DIR)/scripts/feeds update -i)
	$(call Exec,$(CP) $(PROFILE_CONFIG) $(OPENWRT_DIR)/.config)
	$(call Exec,$(MAKE) -C $(OPENWRT_DIR) menuconfig)
	$(call Exec,$(CP) $(OPENWRT_DIR)/.config $(PROFILE_CONFIG))

cibox-main-menuconfig: menuconfig
nb4-openwrt-menuconfig nb4-mainipv6-menuconfig nb4-main-menuconfig nb4-rescue-menuconfig: menuconfig

#
# Firmware clean rules
#
clean: .firmware
	$(call OpenWRT/Build,clean)
	$(RM) $(OPENWRT_DIR)/staging_dir/target-$(ARCH)*$(FIRMWARE)

cibox-main-clean: clean
nb4-rescue-clean nb4-main-clean nb4-mainipv6-clean nb4-openwrt-clean: clean

