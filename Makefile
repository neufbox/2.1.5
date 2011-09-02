
define Usage

--

usage:


    <firmware>-kernel-menuconfig   - Kenrel configure <firmware>
    <firmware>-menuconfig          - Configure <firmware>
    <firmware>                     - Build <firmware>
    <firmware>-<package>-compile   - Build <firmware> <package>
    <firmware>-<package>-clean     - Clean <firmware> <package>
    world-clean	                   - Clean everything !

    Options:
      V=99: Output to stdout build in single thread
      V=2: Output to openwrt/logs/*

    build-blame:
      print last built packages

    firmware: nb4-main, nb4-rescue, cibox-main
	(experimental firmware: nb4-mainipv6)

                                                                             --

endef


ifneq ($(words $(MAKECMDGOALS)),1)
$(error $(call Usage))
endif

help:
	$(error $(call Usage))


#
# Helpers
#
define Str/Upper
	$(shell echo $(1) | tr "[:lower:]" "[:upper:]")
endef

define Str/Str
	$(if $(findstring $(2),$(1)),$(2))
endef

BLACK	:= \\033[30m
RED	:= \\033[31m
GREEN	:= \\033[32m
BROWN	:= \\033[33m
BLUE	:= \\033[34m
PURPLE	:= \\033[35m
CYAN	:= \\033[36m
GREY	:= \\033[37m
COLOR	?= $(GREEN)
DEFAULT	:= \\033[0m
define Trace
	@printf "$(COLOR) %s $(DEFAULT)\n" $(1)
endef

define Exec
	$(call Trace,"$(1)")
	@$(1)
endef

CP	:= cp -fpR
SED	:= sed -i -e
RM	:= rm -rf

PWD		:= $(shell pwd)
CONFIG_DIR	:= $(PWD)/config
TOOLS_DIR	:= $(PWD)/tools
TFTPBOOT_DIR	?= /tftpboot



#
# main/rescue firmware
#
SVN_RELEASE	:= 14414
RELEASE		:= 2.1.5-R$(SVN_RELEASE)

include include/firmware.mk


CONFIG_VERSION		= CONFIG-R52.0
MAIN_VERSION		:= $(RELEASE)
RESCUE_VERSION		:= $(RELEASE)
ALL_VERSION		:= $(RELEASE)

CIBOX_CONFIG_VERSION	= $(CONFIG_VERSION)
CIBOX_MAIN_VERSION	:= $(RELEASE)

NB4_BOOTLOADER_VERSION	:= NB4-05-CFE
NB4_CONFIG_VERSION	= NB4-$(CONFIG_VERSION)
NB4_MAIN_VERSION	:= $(MAIN_VERSION)
NB4_RESCUE_VERSION	:= $(RESCUE_VERSION)
NB4_ADSLDRIVER_VERSION  := NB4-A2pB025f1
NB4_ALL_VERSION		:= $(ALL_VERSION)
export NB4_CONFIG_VERSION

NB5_BOOTLOADER_VERSION	:= 1.0.0
NB5_CONFIG_VERSION	= NB5-$(CONFIG_VERSION)
NB5_MAIN_VERSION	:= $(MAIN_VERSION)
NB5_RESCUE_VERSION	:= 1.0.10
NB5_ALL_VERSION		:= $(ALL_VERSION)
export NB5_CONFIG_VERSION



PROFILE_CONFIG	:= $(if $(PROFILE),$(CONFIG_DIR)/$(BOX)/$(PROFILE).config)
KERNEL_CONFIG	:= $(if $(PROFILE),$(CONFIG_DIR)/$(BOX)/$(PROFILE)-kernel.config)

ifneq ($(PROFILE),)
ifneq ($(shell grep "^CONFIG_LINUX_2_6_21=y" $(PROFILE_CONFIG)),)
LINUX_VERSION		:= 2.6.21
LINUX_VERSION_RELEASE	:= $(LINUX_VERSION).7
endif
ifneq ($(shell grep "^CONFIG_LINUX_2_6_27=y" $(PROFILE_CONFIG)),)
LINUX_VERSION		:= 2.6.27
LINUX_VERSION_RELEASE	:= $(LINUX_VERSION).22
endif
ifneq ($(shell grep "^CONFIG_LINUX_2_6_29=y" $(PROFILE_CONFIG)),)
LINUX_VERSION		:= 2.6.29
LINUX_VERSION_RELEASE	:= $(LINUX_VERSION).2
endif

# Board defines
-include include/$(BOARD).mk

MAKE_VARS += \
	BOX=$(BOX) UBOX=$(UBOX) \
	SVN_RELEASE=$(SVN_RELEASE) FIRMWARE_RELEASE=$(FIRMWARE_RELEASE) \
	$(if $(shell grep "^CONFIG_NATIVE_TOOLCHAIN=y" $(PROFILE_CONFIG)),$(TOOLCHAIN_VARS))
endif

NCPUS=$(shell echo $$((`cut -d '-' -f 2 /sys/devices/system/cpu/present`+2)))

ifeq ($(V),)
MAKE_FLAGS += -j$(NCPUS)
MAKE_VARS += BUILD_LOG=1
endif

include include/openwrt.mk

define Efixo/Release/Install
	$(call Exec,$(CP) $(TFTPBOOT_DIR)/$(1) $(TFTPBOOT_DIR)/$(BOX)/release-$(RELEASE)/$(2))
	$(call Exec,cd $(TFTPBOOT_DIR)/$(BOX)/release-$(RELEASE)/ && sha256sum $(2) >> $(BOX)-$(RELEASE).sha256sum)
endef

nb4-bootloader:
	$(MAKE) -C cfe/
	$(MAKE_VARS) $(MAKE) -C tools/make-bootloader
	$(OPENWRT_DIR)/staging_dir/host/bin/make-bootloader-$(BOX) \
		$(PWD)/cfe/images/cfe6358.bin \
		$(PWD)/cfe/images/$(NB4_BOOTLOADER_VERSION) \
		$(NB4_BOOTLOADER_VERSION)

	$(CP) $(PWD)/cfe/images/$(NB4_BOOTLOADER_VERSION) \
		$(TFTPBOOT_DIR)/openwrt-$(BOX)-bootloader

nb4-main-adslphy:
	# Build adsl firmware
	$(MAKE_VARS) $(MAKE) -C tools/make-adsl-firmware
	$(OPENWRT_DIR)/staging_dir/host/bin/make-adsl-firmware \
		$(LINUX_DIR)/broadcom-adsl/driver/adsl_phy.bin \
		$(TFTPBOOT_DIR)/openwrt-$(BOX)-adslphy \
		$(NB4_ADSLDRIVER_VERSION)


nb4-release:
	# fill with ones
	cat /dev/zero | tr '\000' '\377' | dd bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all count=128 seek=0
	# bootloader
	dd bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-bootloader seek=0
	# main firmware
	dd bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-main seek=1
	# config
	# cat /dev/zero | tr '\000' '\377' | dd bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all count=10 seek=86
	# rescue firmware
	dd bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-rescue seek=96
	# adsl phy
	dd bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-adslphy seek=120
	# bootcounter
	dd if=/dev/zero bs=64k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all count=1 seek=127
	# install release

nb4-full:
	# should build bootloader
	$(MAKE) -C $(PWD) $(BOX)-bootloader
	$(MAKE) -C $(PWD) $(BOX)-main
	$(MAKE) -C $(PWD) $(BOX)-rescue
	$(MAKE) -C $(PWD) $(BOX)-release



world-clean:
	$(MAKE) -C $(OPENWRT_DIR)/ config-clean
	@$(RM) $(TARGET_DIR)/config-$(LINUX_VERSION)
	@$(RM) $(OPENWRT_DIR)/build_dir/ \
		$(OPENWRT_DIR)/staging_dir/ \
		$(OPENWRT_DIR)/logs/ \
		$(OPENWRT_DIR)/tmp/ \
		$(OPENWRT_DIR)/bin \
		$(OPENWRT_DIR)/feeds/*.tmp \
		$(OPENWRT_DIR)/feeds/*.index
	@$(RM) $(OPENWRT_DIR)/.config*
	@$(RM) include/packages.mk
	@-$(RM) $(OPENWRT_DIR)/:

build-blame:
	@find openwrt/logs/ -type f | xargs ls -ult | head

#
# Packages rules
#

include/packages.mk:
	./tools/scripts/cowabunga-subtarget.py

-include include/packages.mk
