#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=base-files
PKG_RELEASE:=20

PKG_FILE_DEPENDS:=$(PLATFORM_DIR)/ $(GENERIC_PLATFORM_DIR)/base-files/

include $(INCLUDE_DIR)/package.mk

ifneq ($(DUMP),1)
  TARGET:=-$(BOARD)
  ifneq ($(wildcard $(PLATFORM_DIR)/base-files-$(PROFILE) $(PLATFORM_SUBDIR)/base-files-$(PROFILE)),)
    TARGET:=$(TARGET)-$(PROFILE)
  endif
  LIBGCC_VERSION:=$(GCC_VERSION)
  ifeq ($(ARCH),x86_64)
    LIB_SUFFIX:=64
  endif
  ifeq ($(ARCH),mips64)
    LIB_SUFFIX:=64
  endif
  LIBC_VERSION:=$(LIBCV)
else
  LIBC_VERSION:=<LIBC_VERSION>
  LIBGCC_VERSION:=<LIBGCC_VERSION>
endif

LIB_NATIVE:=$(if $(CONFIG_NATIVE_TOOLCHAIN),$(LIB_SUFFIX))

CONFIG_PACKAGE_base-files$(TARGET):=$(CONFIG_PACKAGE_base-files)

define Package/base-files$(TARGET)
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Base filesystem for OpenWrt
  URL:=http://openwrt.org/
  VERSION:=$(PKG_RELEASE)-$(REVISION)
  $(call Config,network.lan.proto,string,static,LAN Protocol)
  $(call Config,network.lan.ipaddr,ip,192.168.1.1,LAN IP Address)
  $(call Config,network.lan.netmask,netmask,255.255.255.0,LAN Network Mask)
  $(call Config,network.lan.gateway,ip,,LAN Gateway)
  $(call Config,network.lan.dns,ip,,LAN DNS server)
endef

define Package/base-files$(TARGET)/conffiles
/etc/banner
/etc/group
/etc/passwd
/etc/profile
/etc/sysctl.conf
$(call $(TARGET)/conffiles)
endef

define Package/base-files$(TARGET)/description
 This package contains a base filesystem and system scripts for OpenWrt.
endef

define Package/gcc/Default
  SECTION:=libs
  CATEGORY:=Base system
  DEPENDS:=
  URL:=http://gcc.gnu.org/
  VERSION:=$(LIBGCC_VERSION)-$(PKG_RELEASE)
endef

define Package/libgcc
$(call Package/gcc/Default)
  TITLE:=GCC support library
  DEPENDS+=@!TARGET_avr32
endef

define Package/libssp
$(call Package/gcc/Default)
  DEPENDS+=@SSP_SUPPORT
  TITLE:=GCC support library
endef

define Package/libstdcpp
$(call Package/gcc/Default)
  NAME:=libstdc++
  TITLE:=GNU Standard C++ Library v3
  DEPENDS+=@INSTALL_LIBSTDCPP
endef

define Package/libc/Default
  SECTION:=libs
  CATEGORY:=Base system
  DEPENDS:=
  VERSION:=$(LIBC_VERSION)-$(PKG_RELEASE)
  URL:=$(LIBC_URL)
endef

define Package/libc
$(call Package/libc/Default)
  TITLE:=C library
endef

define Package/libpthread
$(call Package/libc/Default)
  TITLE:=POSIX thread library
  DEPENDS:= +librt
endef

define Package/librt
$(call Package/libc/Default)
  TITLE:=POSIX.1b RealTime extension library
endef

define Package/ldd
$(call Package/libc/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=LDD trace utility
endef

define Package/ldconfig
$(call Package/libc/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Shared library path configuration
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

ifeq ($(CONFIG_TARGET_brcm63xx_neufbox4)$(CONFIG_TARGET_cavium_octeon_neufbox5),)
  define Package/base-files$(TARGET)/install/copy
	$(CP) ./files/* $(1)/
  endef
else
  define Package/base-files$(TARGET)/install/copy
	mkdir -p $(1)/etc
	$(CP) ./files/etc/banner $(1)/etc/
	$(CP) ./files/etc/passwd $(1)/etc/
	$(CP) ./files/etc/group $(1)/etc/
	$(CP) ./files/etc/protocols $(1)/etc/
	$(CP) ./files/etc/profile $(1)/etc/
	$(CP) ./files/etc/sysctl.conf $(1)/etc/
	mkdir -p $(1)/etc/init.d
	$(CP) ./files/etc/init.d/rcS $(1)/etc/init.d
	$(CP) ./files/etc/init.d/usb $(1)/etc/init.d
	$(CP) ./files/etc/init.d/watchdog $(1)/etc/init.d
	$(CP) ./files/etc/init.d/sysctl $(1)/etc/init.d
	$(CP) ./files/etc/init.d/umount $(1)/etc/init.d
	$(CP) ./files/etc/init.d/nfs $(1)/etc/init.d
	$(CP) ./files/etc/init.d/nfs $(1)/etc/init.d
	$(if $(CONFIG_PACKAGE_hotplug2), $(CP) ./files/etc/hotplug2-common.rules $(1)/etc/)
	$(if $(CONFIG_PACKAGE_hotplug2), $(CP) ./files/etc/hotplug2-init.rules $(1)/etc/)
	mkdir -p $(1)/etc/hotplug.d
	mkdir -p $(1)/etc/hotplug.d/net
	$(CP) ./files/etc/hotplug.d/net/10-tun $(1)/etc/hotplug.d/net/
	mkdir -p $(1)/etc/hotplug.d/usb
	$(CP) ./files/etc/hotplug.d/usb/20-usb-printer $(1)/etc/hotplug.d/usb/
	mkdir -p $(1)/etc/hotplug.d/block
	$(CP) ./files/etc/hotplug.d/block/10-mount $(1)/etc/hotplug.d/block/
	mkdir -p $(1)/etc/hotplug.d/sound
	$(CP) ./files/etc/hotplug.d/sound/10-usb-speaker $(1)/etc/hotplug.d/sound/
	echo $(CONFIG_HOSTNAME) > $(1)/etc/hostname
  endef
endif

define Package/base-files$(TARGET)/install
	$(call Package/base-files$(TARGET)/install/copy,$(1))
	if [ -d $(GENERIC_PLATFORM_DIR)/base-files/. ]; then \
		$(CP) $(GENERIC_PLATFORM_DIR)/base-files/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/base-files/. ]; then \
		$(CP) $(PLATFORM_DIR)/base-files/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/base-files-$(PROFILE)/. ]; then \
		$(CP) $(PLATFORM_DIR)/base-files-$(PROFILE)/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/$(PROFILE)/base-files/. ]; then \
		$(CP) $(PLATFORM_DIR)/$(PROFILE)/base-files/* $(1)/; \
	fi
	$(if $(filter-out $(PLATFORM_DIR),$(PLATFORM_SUBDIR)), \
		if [ -d $(PLATFORM_SUBDIR)/base-files/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/base-files/* $(1)/; \
		fi; \
		if [ -d $(PLATFORM_SUBDIR)/base-files-$(PROFILE)/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/base-files-$(PROFILE)/* $(1)/; \
		fi; \
		if [ -d $(PLATFORM_SUBDIR)/$(PROFILE)/base-files/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/$(PROFILE)/base-files/* $(1)/; \
		fi \
	)
# Form valid /etc/shadow entries, by copying the user:pw pairs
# from /etc/passwd, and adding on pw change data
	$(if $(CONFIG_BUSYBOX_CONFIG_FEATURE_SHADOWPASSWDS), \
		$(CP) ./files/etc/shadow $(1)/etc/shadow; \
	)
	$(SED) 's,$$$$R,$(OPENWRT_RELEASE),g' $(1)/etc/banner
	$(SED) 's,$$$$E,$(FIRMWARE_RELEASE),g' $(1)/etc/banner
	$(SED) 's,$$$$S,$(SVN_RELEASE),g' $(1)/etc/banner
	mkdir -p $(1)/CONTROL
	mkdir -p $(1)/dev
	$(if $(CONFIG_PACKAGE_efixo-base-files-common),,mkdir -p $(1)/etc/crontabs)
	$(if $(CONFIG_PACKAGE_efixo-base-files-common),,mkdir -p $(1)/jffs)
	mkdir -p $(1)/lib/firmware
	$(if $(LIB_SUFFIX),rm -f $(1)/lib$(LIB_SUFFIX))
	$(if $(LIB_SUFFIX),ln -s lib $(1)/lib$(LIB_SUFFIX))
	$(if $(CONFIG_PACKAGE_efixo-base-files-nb4-main),ln -sf /tmp/mnt $(1)/mnt,mkdir -p $(1)/mnt)
	mkdir -p $(1)/proc
	mkdir -p $(1)/tmp
	mkdir -p $(1)/usr/lib
	$(if $(LIB_SUFFIX),rm -f $(1)/usr/lib$(LIB_SUFFIX))
	$(if $(LIB_SUFFIX),ln -s lib $(1)/usr/lib$(LIB_SUFFIX))
	mkdir -p $(1)/usr/bin
	mkdir -p $(1)/sys
	mkdir -p $(1)/www
	mkdir -p $(1)/root
	ln -sf /proc/mounts $(1)/etc/mtab
	rm -f $(1)/var
	ln -sf /tmp $(1)/var
	mkdir -p $(1)/etc
	$(if $(CONFIG_PACKAGE_efixo-base-files-common),ln -sf /tmp/resolv.conf $(1)/etc/,ln -sf /tmp/resolv.conf /tmp/fstab /tmp/TZ $(1)/etc/)
	$(call Package/base-files/install-target,$(1))
	for conffile in $(1)/etc/config/*; do \
		if [ -f "$$$$conffile" ]; then \
			grep "$$$$conffile" $(1)/CONTROL/conffiles || \
				echo "$$$${conffile##$(1)}" >> $(1)/CONTROL/conffiles; \
		fi \
	done
endef

define Package/libgcc/install
	$(INSTALL_DIR) $(1)/lib
	$(if $(CONFIG_USE_UCLIBC)$(CONFIG_USE_EGLIBC), \
		$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_SUFFIX)/libgcc_s.so.* $(1)/lib/, \
		$(CP) $(TOOLCHAIN_DIR)/usr/lib$(LIB_SUFFIX)/libgcc_s.so.* $(1)/lib/ \
	)
endef

define Package/libssp/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_SUFFIX)/libssp.so.* $(1)/lib/
endef

define Package/libstdcpp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(if $(wildcard $(TOOLCHAIN_DIR)/lib/libstdc++.so*), \
		$(CP) $(TOOLCHAIN_DIR)/lib/libstdc++.so* $(1)/usr/lib/, \
		$(CP) $(TOOLCHAIN_DIR)/usr/lib$(LIB_SUFFIX)/libstdc++.so.* $(1)/usr/lib/ \
	)
endef

use_libutil=$(if $(CONFIG_USE_GLIBC)$(CONFIG_EGLIBC_OPTION_EGLIBC_UTMP),libutil)
use_libnsl=$(if $(CONFIG_USE_GLIBC)$(CONFIG_EGLIBC_OPTION_EGLIBC_NIS),libnsl)
use_nsswitch=$(if $(CONFIG_USE_GLIBC)$(CONFIG_EGLIBC_OPTION_EGLIBC_NSSWITCH),libnss_dns libnss_files)

define Package/glibc/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/ld*.so.* $(1)/lib/
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/ld-$(LIBC_SO_VERSION).so $(1)/lib/
	for file in libanl libc libcidn libcrypt libdl libm $(use_libnsl) $(use_nsswitch) libresolv $(use_libutil); do \
		$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/$$$$file.so.* $(1)/lib/; \
		$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/$$$$file-$(LIBC_SO_VERSION).so $(1)/lib/; \
	done
endef

define Package/eglibc/install
$(call Package/glibc/install,$1)
endef

define Package/uClibc/install
	$(INSTALL_DIR) $(1)/lib
	for file in ld$(LIB_SUFFIX)-uClibc libc libcrypt libdl libm libnsl libresolv libuClibc libutil; do \
		$(CP) $(TOOLCHAIN_DIR)/lib/$$$$file.so.* $(1)/lib/; \
		$(CP) $(TOOLCHAIN_DIR)/lib/$$$$file-$(LIBC_SO_VERSION).so $(1)/lib/; \
	done
endef

define Package/libc/install
$(call Package/$(LIBC)/install,$1)
endef

define Package/libpthread/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/libpthread.so.* $(1)/lib/
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/libpthread-$(LIBC_SO_VERSION).so $(1)/lib/
endef

define Package/librt/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/librt.so.* $(1)/lib/
	$(CP) $(TOOLCHAIN_DIR)/lib$(LIB_NATIVE)/librt-$(LIBC_SO_VERSION).so $(1)/lib/
endef

define Package/ldd/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(TOOLCHAIN_DIR)/usr/bin/ldd $(1)/usr/bin/
endef

define Package/ldconfig/install
	$(INSTALL_DIR) $(1)/sbin/
	$(CP) $(TOOLCHAIN_DIR)/sbin/ldconfig $(1)/sbin/
endef

ifneq ($(DUMP),1)
  -include $(PLATFORM_DIR)/base-files.mk
endif

$(eval $(call BuildPackage,base-files$(TARGET)))
$(eval $(call BuildPackage,libc))
$(eval $(call BuildPackage,libgcc))
$(eval $(call BuildPackage,libssp))
$(eval $(call BuildPackage,libstdcpp))
$(eval $(call BuildPackage,libpthread))
$(eval $(call BuildPackage,librt))
$(eval $(call BuildPackage,ldd))
$(eval $(call BuildPackage,ldconfig))
