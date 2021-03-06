#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uclibc++
PKG_VERSION:=0.2.2
PKG_RELEASE:=1

PKG_SOURCE:=uClibc++-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://cxx.uclibc.org/src/
PKG_MD5SUM:=1ceef3209cca88be8f1bd9de99735954

PKG_BUILD_DIR:=$(BUILD_DIR)/uClibc++-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/uclibcxx
  NAME:=uclibc++
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C++ library for embedded systems
  URL:=http://cxx.uclibc.org/src/
endef

UCLIBC_TARGET_ARCH:=$(shell echo $(ARCH) | sed -e s'/-.*//' \
	-e 's/i.86/i386/' \
	-e 's/sparc.*/sparc/' \
	-e 's/m68k.*/m68k/' \
	-e 's/ppc/powerpc/g' \
	-e 's/v850.*/v850/g' \
	-e 's/sh64/sh/' \
	-e 's/sh[234].*/sh/' \
	-e 's/mips.*/mips/' \
	-e 's/mipsel.*/mips/' \
	-e 's/cris.*/cris/' \
)

TARGET_CFLAGS += $(FPIC)

MAKE_FLAGS:= \
	TOPDIR="$(PKG_BUILD_DIR)/" \
	$(TARGET_CONFIGURE_OPTS) \
	ARCH_CFLAGS="$(TARGET_CFLAGS)" \
	CROSS="$(TARGET_CROSS)" \
	CP="$(CP)" \
	$(if $(CONFIG_TARGET_avr32),check_as_needed=)

# check_as_needed overrides dependency on libgcc_s

define Build/Configure
	if [ -f ./files/config.$(UCLIBC_TARGET_ARCH) ]; then \
		cp ./files/config.$(UCLIBC_TARGET_ARCH) $(PKG_BUILD_DIR)/.config; \
	else \
		cp ./files/config.default $(PKG_BUILD_DIR)/.config; \
	fi
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(MAKE_FLAGS) \
		DESTDIR="$(PKG_INSTALL_DIR)/usr" \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin $(1)/usr/include/uClibc++ $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/g++-uc $(2)/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/uClibc++/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuClibc++*.{a,so*} $(1)/usr/lib/
	$(SED) 's|-I/include/|-I$(STAGING_DIR)/usr/include/uClibc++/|g' $(2)/bin/g++-uc
	$(SED) 's|-L/lib/|-L$(STAGING_DIR)/usr/lib/|g' $(2)/bin/g++-uc
# add another wrapper which links against both uClibc++ and libstdc++ 
	$(INSTALL_BIN) $(2)/bin/g++-uc $(2)/bin/g++-uc+std
	$(SED) 's|^WRAPPER_INCLUDEDIR=.*||g' $(2)/bin/g++-uc+std
	$(SED) 's|-luClibc++|-Wl,-Bdynamic,-luClibc++,-Bstatic,-lstdc++,-Bdynamic|g' $(2)/bin/g++-uc+std
	$(SED) 's|-nostdinc++||g' $(2)/bin/g++-uc+std
endef

define Package/uclibcxx/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuClibc++.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuClibc++-*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,uclibcxx))
