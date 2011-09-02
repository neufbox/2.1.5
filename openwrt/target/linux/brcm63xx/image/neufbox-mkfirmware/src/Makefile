
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

all: mkfirmware

HOST_CFLAGS += -D_GNU_SOURCE
HOST_CFLAGS += -I$(LINUX_DIR)/include/asm-mips/mach-bcm63xx
HOST_CFLAGS += -I$(LINUX_DIR)/include/asm-mips/mach-bcm63xx/nb4

mkfirmware: mkfirmware.c
	$(HOSTCC) $@.c $(HOST_CFLAGS) -lz -o $@
