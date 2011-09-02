# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

LOADADDR = 0x80010000		# RAM start + CFE SIZE 
KERNEL_ENTRY = $(LOADADDR)	# Newer kernels add a jmp to the kernel_entry at the start of the binary
RAMSIZE = 0x00800000		# 32MB

FLASH_START = 0xBFC00000
FIRMWARE_OFFSET = 0x10000

ifeq ($(CONFIG_TARGET_brcm63xx_neufbox4_rescue),y) 
FIRMWARE_OFFSET = "-n 0x600000"
endif
ifeq ($(CONFIG_TARGET_brcm63xx_neufbox4),y) 
BLOCKSIZE="-k 1"
endif

define Image/Build/CFE
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag -i $(KDIR)/vmlinux.lzma.cfe -f $(KDIR)/root.$(1) \
		-o $(BIN_DIR)/openwrt-$(2)-$(1)-cfe.bin \
		-b $(2) -c $(3) -e $(LOADADDR) -l $(LOADADDR) \
		-s $(FLASH_START) $(FIRMWARE_OFFSET) $(BLOCKSIZE) -v 6 \
		$(if $(FIRMWARE_RELEASE),-N $(FIRMWARE_RELEASE))
#		-b $(2) -c $(3) -e $(KERNEL_ENTRY) -l $(LOADADDR)

	$(CP) $(BIN_DIR)/openwrt-$(2)-$(1)-cfe.bin $(KDIR)/last.main
endef

ifeq ($(BOX),nb4) 
  define Build/Clean/neufbox
#	$(MAKE) -C neufbox-lzma clean
	$(MAKE) -C neufbox-mkfirmware clean
  endef

  define Image/Prepare/neufbox
#	$(MAKE) -C neufbox-lzma \
#		BUILD_DIR="$(KDIR)" TARGET="$(KDIR)" \
#		all install

	# Build the neufbox mkfirmware
	$(MAKE) -C neufbox-mkfirmware \
		BUILD_DIR="$(KDIR)" \
		TARGET="$(KDIR)" \
		clean install

  endef
else
  define Image/Prepare/neufbox
  endef

  define Build/Clean/neufbox
  endef
endif

define Image/Build/CFEAGPF
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag -i $(KDIR)/vmlinux.lzma.cfe -f $(KDIR)/root.$(1) \
		-o $(BIN_DIR)/openwrt-$(2)-$(1)-cfe.bin \
		-b $(2) -c $(3) -e $(LOADADDR) -l $(LOADADDR) \
		-v 8 -m IMAGE -k 0x20000 -n $(4) -p alice
endef

define Image/Build/RedBoot
	cp $(KDIR)/vmlinux.elf $(BIN_DIR)/openwrt-$(1)-vmlinux.elf
	gzip -9 -c $(KDIR)/vmlinux > $(KDIR)/vmlinux.bin.gz
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux $(KDIR)/vmlinux.bin.l7
	dd if=$(KDIR)/vmlinux.bin.l7 of=$(BIN_DIR)/openwrt-$(1)-vmlinux.lzma bs=65536 conv=sync
	dd if=$(KDIR)/vmlinux.bin.gz of=$(BIN_DIR)/openwrt-$(1)-vmlinux.gz bs=65536 conv=sync
endef

define Image/Build/CFEOLD
	$(TOPDIR)/scripts/brcmImage.pl -t -p	\
		-b $(2) -c $(3)			\
		-k $(KDIR)/vmlinux.lzma.cfe	\
		-r $(KDIR)/root.$(1)		\
		-o $(BIN_DIR)/openwrt-$(2)-$(1)-cfe.bin
endef

define Build/Clean
	$(call Build/Clean/neufbox)
endef

define Image/Prepare
	# CFE is a LZMA nazi! It took me hours to find out the parameters!
	# Also I think lzma has a bug cause it generates different output depending on
	# if you use stdin / stdout or not. Use files instead of stdio here, cause
	# otherwise CFE will complain and not boot the image.
	$(STAGING_DIR_HOST)/bin/lzma e -d22 -fb64 -a1 $(KDIR)/vmlinux $(KDIR)/vmlinux.lzma.tmp

	# Strip out the length, CFE doesn't like this
	dd if=$(KDIR)/vmlinux.lzma.tmp of=$(KDIR)/vmlinux.lzma.cfe bs=5 count=1
	dd if=$(KDIR)/vmlinux.lzma.tmp of=$(KDIR)/vmlinux.lzma.cfe ibs=13 obs=5 skip=1 seek=1 conv=notrunc
	rm -f $(KDIR)/vmlinux.lzma.tmp

	$(call Image/Prepare/neufbox)

	rm -f $(KDIR)/fs_mark
	touch $(KDIR)/fs_mark
	$(call prepare_generic_squashfs,$(KDIR)/fs_mark)
endef

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-root.$(1) bs=128k conv=sync
	# Neufbox4
	$(call Image/Build/CFE,$(1),96358VW,6358)
endef

$(eval $(call BuildImage))
