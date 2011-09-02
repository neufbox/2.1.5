define Efixo/brcm63xx/Install
	$(call Exec,$(CP) $(LINUX_DIR)/vmlinux.elf $(TFTPBOOT_DIR)/vmlinux)
	$(call Exec,$(CP) $(OPENWRT_DIR)/bin/system-$(BOX).xml $(TFTPBOOT_DIR)/openwrt-$(BOX)-config || true)
	$(call Exec,$(CP) $(BIN_DIR)/openwrt-96358VW-squashfs-cfe.bin $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE))
endef

define Efixo/octeon/Install
	$(call Exec,$(CP) $(BIN_DIR)/openwrt-$(BOARD)-vmlinux.elf $(TFTPBOOT_DIR)/vmlinux.64)
	$(call Exec,$(CP) $(OPENWRT_DIR)/bin/system-$(BOX).xml $(TFTPBOOT_DIR)/openwrt-$(BOX)-config || true)
	$(call Exec,$(CP) $(BIN_DIR)/openwrt-$(BOARD)-nb5 $(TFTPBOOT_DIR)/$(FIRMWARE_RELEASE))
endef

define Efixo/Release/Install
	$(call Exec,$(CP) $(TFTPBOOT_DIR)/$(1) $(TFTPBOOT_DIR)/$(BOX)/release-$(RELEASE)/$(2))
	$(call Exec,cd $(TFTPBOOT_DIR)/$(BOX)/release-$(RELEASE)/ && sha256sum $(2) >> $(BOX)-$(RELEASE).sha256sum)
endef

define Firmware/Version
	$(shell strings $(1) | grep $(UBOX))
endef

define Release/Check
	@echo $(call Firmware/Version,$(1)) $(2)
	@echo "subst: $(subst $(call Str/Upper,$(2)),,$(call Firmware/Version,$(1)))"
	$(if $(subst $(2),,$(call Firmware/Version,$(1))),$(error $(1) is not $(2)))
endef

define Efixo/Release
	$(call Exec,mkdir -p $(TFTPBOOT_DIR)/$(BOX)/release-$(RELEASE))
	$(call Exec,echo "# $(BOX) $(RELEASE) release sha256sum..." >  $(TFTPBOOT_DIR)/$(BOX)/release-$(RELEASE)/$(BOX)-$(RELEASE).sha256sum)
	$(call Efixo/Release/Install,openwrt-$(BOX)-bootloader,$(UBOX)-BOOTLOADER-R$($(UBOX)_BOOTLOADER_VERSION))
	$(call Efixo/Release/Install,openwrt-$(BOX)-config,$($(UBOX)_CONFIG_VERSION))
	$(call Efixo/Release/Install,openwrt-$(BOX)-main,$(UBOX)-MAIN-R$($(UBOX)_MAIN_VERSION))
	$(call Efixo/Release/Install,openwrt-$(BOX)-rescue,$(UBOX)-RESCUE-R$($(UBOX)_RESCUE_VERSION))
	$(call Efixo/Release/Install,openwrt-$(BOX)-all,$(UBOX)-ALL-R$($(UBOX)_ALL_VERSION))
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
#	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-bootloader,NB4-$(NB4_BOOTLOADER_VERSION)-CFE)
	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-main,NB4-MAIN-R$(NB4_MAIN_VERSION))
	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-rescue,NB4-RESCUE-R$(NB4_MAIN_RESCUE))
	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-adslphy,$(NB4_ADSLDRIVER_VERSION))
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
	$(call Efixo/Release)


nb5-bootloader:
	$(MAKE_VARS) PATH=$(PATH):$(TOOLCHAIN_DIR_BIN) $(MAKE) -C u-boot clobber
	$(MAKE_VARS) PATH=$(PATH):$(TOOLCHAIN_DIR_BIN) $(MAKE) -C u-boot octeon_neufbox5_config
	$(MAKE_VARS) PATH=$(PATH):$(TOOLCHAIN_DIR_BIN) $(MAKE) -C u-boot
	$(MAKE_VARS) $(MAKE) -C tools/make-bootloader
	$(OPENWRT_DIR)/staging_dir/host/bin/make-bootloader-$(BOX) \
		$(PWD)/u-boot/u-boot-octeon_neufbox5.bin \
		$(TFTPBOOT_DIR)/openwrt-$(BOX)-bootloader \
		NB5-BOOTLOADER-R$(NB5_BOOTLOADER_VERSION)

nb5-release:
#	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-bootloader,NB5-BOOTLOADER-R$(NB5_BOOTLOADER_VERSION))
	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-main,$(call Str/Upper,NB5-MAIN-R$(NB5_MAIN_VERSION)))
	$(call Release/Check,$(TFTPBOOT_DIR)/openwrt-$(BOX)-rescue,$(call Str/Upper,NB5-RESCUE-R$(NB5_RESCUE_VERSION)))
	# fill with ones
	cat /dev/zero | tr '\000' '\377' | dd bs=128k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all count=128 seek=0
	# bootloader
	dd bs=128k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-bootloader seek=0
	# bootloader nvram
	# cat /dev/zero | tr '\000' '\377' | dd bs=128k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all count=1 seek=3
	# main firmware
	dd bs=128k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-main seek=4
	# rescue firmware
	dd bs=128k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all if=$(TFTPBOOT_DIR)/openwrt-$(BOX)-rescue seek=100
	# config
	cat /dev/zero | tr '\000' '\377' | dd bs=128k of=$(TFTPBOOT_DIR)/openwrt-$(BOX)-all count=8 seek=120
	# install release
	$(call Efixo/Release)

nb4-full nb5-full:
	# should build bootloader
	$(MAKE) -C $(PWD) $(BOX)-bootloader
	$(MAKE) -C $(PWD) $(BOX)-main
	$(MAKE) -C $(PWD) $(BOX)-rescue
	$(MAKE) -C $(PWD) $(BOX)-release

