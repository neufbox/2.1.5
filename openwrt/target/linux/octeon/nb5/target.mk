ARCH:=mips64
BOARDNAME:=neufbox5
LINUX_VERSION:=2.6.21.7
ifeq ($(TARGET_BUILD),1)
FILES_DIR:=files-2.6.21
endif
DEFAULT_PACKAGES+=

INITRAMFS_EXTRA_FILES:=$(TOPDIR)/target/linux/$(BOARD)/image/initramfs-base-files.txt

define Target/Description
        Build firmware for neufbox5
endef
