#
# native toolchain
#
# REAL_GNU_TARGET_NAME: configure defines $(OPENWRT_DIR)/include/site/$(REAL_GNU_TARGET_NAME)
# GNU_TARGET_NAME: configure target name --target=$(GNU_TARGET_NAME) --host=$(GNU_TARGET_NAME)
# TARGET_CROSS: cross tools prefix. e.g. CC:=$(TARGET_CROSS)gcc
#

ARCH:=mips
OPTIMIZE_FOR_CPU:=$(ARCH)
LIBC:=uClibc
LIBC_SO_VERSION:=0.9.29
GCC_VERSION:=4.2.4
TARGET_SUFFIX:=uclibc
TOOLCHAIN_DIR:=$(TOOLS_DIR)/toolchains/toolchain-$(BOARD)-$(TARGET_SUFFIX)-gcc-$(GCC_VERSION)
TOOLCHAIN_DIR_BIN:=$(TOOLCHAIN_DIR)/bin
ifneq ($(shell grep "^CONFIG_TARGET_brcm63xx_generic=y" $(PROFILE_CONFIG)),)
LINUX_DIR=$(OPENWRT_DIR)/build_dir/linux-$(BOARD)_generic_$(FIRMWARE)
endif
ifneq ($(shell grep "^CONFIG_TARGET_brcm63xx_neufbox4=y" $(PROFILE_CONFIG)),)
LINUX_DIR=$(OPENWRT_DIR)/build_dir/linux-$(BOARD)_neufbox4_$(FIRMWARE)
endif

TOOLCHAIN_VARS := \
	TOOLCHAIN_DIR=$(TOOLCHAIN_DIR) \
	TOOLCHAIN_DIR_BIN=$(TOOLCHAIN_DIR_BIN) \
	LIBC=$(LIBC) \
	LIBC_SO_VERSION=$(LIBC_SO_VERSION) \
	TARGET_SUFFIX=$(TARGET_SUFFIX) \
	REAL_GNU_TARGET_NAME=$(OPTIMIZE_FOR_CPU)-openwrt-linux-$(TARGET_SUFFIX) \
	GNU_TARGET_NAME=$(OPTIMIZE_FOR_CPU)-linux \
	TARGET_CROSS=$(OPTIMIZE_FOR_CPU)-linux-$(TARGET_SUFFIX)-

ifeq ($(FIRMWARE),nb4-openwrt)
 define Efixo/brcm63xx/Install
  $(call Exec,$(CP) openwrt/bin/openwrt-96358VW-squashfs-cfe.bin $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE))
 endef
else
 ifeq ($(INITRAMFS),)
  define Efixo/brcm63xx/Install
   $(call Exec,$(LINUX_DIR)/mkfirmware \
	$(LINUX_DIR)/last.main $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE) ftth)
   @mv -f $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE) $(TFTPBOOT_DIR)/openwrt-$(FIRMWARE)-ftth
   $(call Exec,$(LINUX_DIR)/mkfirmware \
	   $(LINUX_DIR)/last.main $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE) adsl)
   $(call Exec,rm -f $(LINUX_DIR)/last.main)
   endef
 else
 define Efixo/brcm63xx/Install
  $(call Exec,$(CP) $(LINUX_DIR)/vmlinux.elf $(TFTPBOOT_DIR)/vmlinux)
 endef
 endif
endif
