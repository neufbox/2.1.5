#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=osiris
PKG_VERSION:=4.2.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://osiris.shmoo.com/data/
PKG_MD5SUM:=1951c7dc0fe729af9ffaf58910340d12

include $(INCLUDE_DIR)/package.mk

define Package/osirisd
  SECTION:=admin
  CATEGORY:=Administration
  DEPENDS:=+libopenssl +libpthread
  TITLE:=Host integrity monitoring system (scanning agent)
  URL:=http://www.hostintegrity.com/osiris
endef

CONFIGURE_ARGS += \
	--with-osiris-user=root \
	--with-root-dir=/var/lib/osiris \
	--with-readline=no, \
	ac_cv_openssldir="$(STAGING_DIR)/usr"

MAKE_FLAGS += \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		INTERACTIVE=0 \
		all install

define Package/osirisd/install	
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/osirisd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/osirisd.init $(1)/etc/init.d/osirisd
endef

$(eval $(call BuildPackage,osirisd))
