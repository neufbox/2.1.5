#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
 
include $(TOPDIR)/rules.mk
 
PKG_NAME:=znc
PKG_VERSION:=0.068
PKG_RELEASE:=1
 
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://kent.dl.sourceforge.net/sourceforge/znc
PKG_MD5SUM:=763162d90c67814780de67aa88347c38
 
include $(INCLUDE_DIR)/package.mk
 
define Package/znc
  SUBMENU:=Instant Messaging
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+openssl +uclibcxx
  TITLE:=ZNC
  URL:=http://en.znc.in/
endef
 
define Package/znc/description
	ZNC is an IRC bouncer with many advanced features like detaching, multiple users, per channel playback buffer, SSL, IPv6, transparent DCC bouncing, and c++ module 
	support to name a few.
endef
 
CONFIGURE_VARS += \
	CXX="$(TARGET_CROSS)-g++"
	CXXFLAGS="$(TARGET_CFLAGS) -fno-builtin -fno-rtti -nostdinc++" \
	CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
	LDFLAGS="-nodefaultlibs -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib"
	LIBS="-luClibc++ -lc -lm -lgcc -lssl"

CONFIGURE_ARGS += \
	$(TARGET_CONFIGURE_OPTS) \
	--disable-perl \
	--disable-ipv6 \

define Build/Configure
	$(call Build/Configure/Default,)
	$(call libtool_disable_rpath)
endef
 
define Build/Compile
	$(call Build/Compile/Default,)
endef
 
define Package/znc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/znc $(1)/usr/bin/
endef
 
$(eval $(call BuildPackage,znc))
