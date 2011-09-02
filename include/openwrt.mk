
OPENWRT_RELEASE	:= 15916

OPENWRT_DIR	:= $(PWD)/openwrt
BIN_DIR		:= $(OPENWRT_DIR)/bin

MAKE_VARS	+= OPENWRT_RELEASE=$(OPENWRT_RELEASE)

TARGET_DIR	:= $(OPENWRT_DIR)/target/linux/$(or $(BOARD))

define OpenWRT/SetInitramfs
	$(SED) 's/# CONFIG_TARGET_ROOTFS_INITRAMFS is not set/CONFIG_TARGET_ROOTFS_INITRAMFS=y/' $(OPENWRT_DIR)/.config
	$(SED) 's/CONFIG_TARGET_ROOTFS_SQUASHFS=y/# CONFIG_TARGET_ROOTFS_SQUASHFS is not set/' $(OPENWRT_DIR)/.config
	mv $(TARGET_DIR)/config-$(LINUX_VERSION) $(TARGET_DIR)/config-$(LINUX_VERSION).old
	grep -v INITRAMFS $(TARGET_DIR)/config-$(LINUX_VERSION).old > $(TARGET_DIR)/config-$(LINUX_VERSION)
	$(SED) 's/# CONFIG_BLK_DEV_INITRD is not set/CONFIG_BLK_DEV_INITRD=y/' $(TARGET_DIR)/config-$(LINUX_VERSION)
endef



define OpenWRT/Build
	$(call Trace,"** Build $(FIRMWARE) $(if $(1),$(1),firmware) ...")
	$(call Trace,"   [Efixo release $(SVN_RELEASE) / OpenWRT release $(OPENWRT_RELEASE)]")
	$(call Trace,"")
	$(call Exec,$(CP) $(PROFILE_CONFIG) $(OPENWRT_DIR)/.config)
	$(call Exec,$(CP) $(KERNEL_CONFIG) $(TARGET_DIR)/config-$(LINUX_VERSION))
	$(if $(INITRAMFS), $(call OpenWRT/SetInitramfs))
	$(call Exec,$(MAKE_VARS) $(MAKE) -s $(MAKE_FLAGS) -C $(OPENWRT_DIR) $(1))
endef

