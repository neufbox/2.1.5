# 
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dnsmasq
PKG_VERSION:=2.47
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://thekelleys.org.uk/dnsmasq
PKG_MD5SUM:=4524081e56d0b935717d493e8e8d3e11

PKG_BUILD_DEPENDS+=+efixo-libnbd

include $(INCLUDE_DIR)/package.mk

define Package/dnsmasq
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=A lightweight DNS and DHCP server
  URL:=http://www.thekelleys.org.uk/dnsmasq/
endef

define Package/dnsmasq/description
 It is intended to provide coupled DNS and DHCP service to a LAN.
endef

#define Package/dnsmasq/conffiles
#/etc/config/dhcp
#endef

TARGET_CFLAGS+=$(if $(CONFIG_PACKAGE_efixo-libnbd),$(TARGET_CPPFLAGS) -DHAVE_LIBNBD)
EXTRA_LIBS+=$(if $(CONFIG_PACKAGE_efixo-libnbd),-lnbu -lnbd -L$(STAGING_DIR)/usr/lib)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -DHAVE_ISC_READER=1" \
		LIBS="$(EXTRA_LIBS)" \
		BINDIR="/usr/sbin" MANDIR="/usr/man" \
		AWK="awk" \
		all
endef

define Package/dnsmasq/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/dnsmasq $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(if $(CONFIG_PACKAGE_efixo-rcc),,$(INSTALL_DATA) ./files/dhcp.conf $(1)/etc/config/dhcp)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(if $(CONFIG_PACKAGE_efixo-rcc),,$(INSTALL_BIN) ./files/dnsmasq.init $(1)/etc/init.d/dnsmasq)
endef

$(eval $(call BuildPackage,dnsmasq))
