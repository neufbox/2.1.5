#
# Copyright (C) 2008 anthony.viallard@efixo.net - OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samba2
PKG_VERSION:=2.0.10
PKG_RELEASE:=1
PKG_MD5SUM:=54870482fe036b7e69dd48c90661eec6

PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://se.samba.org/pub/samba/stable \
	ftp://ftp.easynet.be/samba/stable
PKG_BUILD_DIR:=$(BUILD_DIR)/samba-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/samba2
 SECTION:=net
 CATEGORY:=Network
 TITLE:=Samba - NetBIOS/SMB implementation
 URL:=http://www.samba.org/
endef

define Package/samba2/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for unix systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef

CONFIGURE_ARGS += \
	--program-prefix="" \
	--program-suffix="" \
	--prefix=/usr \
	--exec-prefix=/usr \
	--bindir=/usr/bin \
	--datadir=/usr/share \
	--includedir=/usr/include \
	--infodir=/usr/share/info \
	--libdir=/usr/lib \
	--libexecdir=/usr/lib \
	--localstatedir=/var/log/samba \
	--mandir=/usr/share/man \
	--sbindir=/usr/sbin \
	--sysconfdir=/var/smbd \
	$(DISABLE_LARGEFILE) \
	$(DISABLE_NLS) \
	--with-lockdir=/var/run \
	--with-privatedir=/var/smbd

CONFIGURE_VARS += \
	ac_cv_sizeof_int=4 \
	ac_cv_sizeof_long=8 \
	ac_cv_sizeof_short=2 \
	samba_cv_HAVE_FCNTL_LOCK=yes \
	samba_cv_FTRUNCATE_NEEDS_ROOT=no \
	samba_cv_HAVE_BROKEN_FCNTL64_LOCKS=no \
	samba_cv_HAVE_TRUNCATED_SALT=no \
	samba_cv_HAVE_UNION_SEMUN=no \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_NEED_SGI_SEMUN_HACK=no \
	samba_cv_USE_SETRESUID=no \
	samba_cv_HAVE_SYSV_IPC=no \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_USE_SETEUID=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	CFLAGS="$$$$CFLAGS -D_GNU_SOURCE -DNDEBUG"

define Package/samba2/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source/bin/nmbd $(1)/usr/sbin/nmbd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source/bin/smbd $(1)/usr/sbin/smbd
#	$(INSTALL_DIR) $(1)/usr/share/samba
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/samba/codepage.{437,850} $(1)/usr/share/samba/
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/samba/unicode_map.{437,850,ISO8859-1,ISO8859-15} $(1)/usr/share/samba/
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/samba/codepage.852 $(1)/usr/share/samba/
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/samba/unicode_map.{852,ISO8859-2} $(1)/usr/share/samba/
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/samba/codepage.{866,1251} $(1)/usr/share/samba/
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/samba/unicode_map.{866,ISO8859-5,KOI8-R,1251} $(1)/usr/share/samba/
endef

#define Build/Configure
#  $(call Build/Configure/Default,,,source/)
#endef

define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)

# make make codepage utility for current host
	(cd $(PKG_BUILD_DIR)/source ; rm -rf config.{cache,status} ; \
		./configure \
			--program-prefix="" \
			--program-suffix="" \
			--prefix=/usr \
			--exec-prefix=/usr \
			--bindir=/usr/bin \
			--datadir=/usr/share \
			--includedir=/usr/include \
			--infodir=/usr/share/info \
			--libdir=/usr/lib \
			--libexecdir=/usr/lib \
			--localstatedir=/var/log/samba \
			--mandir=/usr/share/man \
			--sbindir=/usr/sbin \
			--sysconfdir=/etc/samba \
			$(DISABLE_LARGEFILE) \
			$(DISABLE_NLS) \
			--with-lockdir=/var/run/samba \
			--with-privatedir=/etc/samba \
	);
	$(MAKE) -C $(PKG_BUILD_DIR)/source \
		bin/make_smbcodepage bin/make_unicodemap

# save make_smbcodepage and make_unicodemap
	(cd $(PKG_BUILD_DIR)/source/bin ; \
		mv -f make_smbcodepage make_smbcodepage.builder ; \
		mv -f make_unicodemap make_unicodemap.builder ; \
	);

# clean
	$(MAKE) -C $(PKG_BUILD_DIR)/source \
		clean

# make crosscompiled smbd and nmbd
	(cd $(PKG_BUILD_DIR)/source; rm -rf config.{cache,status}; \
		$(CONFIGURE_VARS) \
		./configure \
		$(CONFIGURE_ARGS) \
	);
	$(MAKE) -C $(PKG_BUILD_DIR)/source \
		CODEPAGEDIR="/usr/share/samba" \
		all

# restore make_smbcodepage and make_unicodemap
	(cd $(PKG_BUILD_DIR)/source/bin ; \
		cp -f make_smbcodepage.builder make_smbcodepage ; touch make_smbcodepage ; \
		cp -f make_unicodemap.builder make_unicodemap ; touch make_unicodemap ; \
	);

# install codepages
#	mkdir -p $(PKG_INSTALL_DIR)/usr/share/samba
#	mkdir -p $(PKG_INSTALL_DIR)/var/log
#	$(MAKE) -C $(PKG_BUILD_DIR)/source \
#		BASEDIR="$(PKG_INSTALL_DIR)/usr" \
#		BINDIR="$(PKG_INSTALL_DIR)/usr/bin" \
#		SBINDIR="$(PKG_INSTALL_DIR)/usr/bin" \
#		LIBDIR="$(PKG_INSTALL_DIR)/usr/lib" \
#		VARDIR="$(PKG_INSTALL_DIR)/var/log/samba" \
#		MANDIR="$(PKG_INSTALL_DIR)/usr/share/man" \
#		CONFIGDIR="$(PKG_INSTALL_DIR)/etc/samba" \
#		PRIVATEDIR="$(PKG_INSTALL_DIR)/etc/samba" \
#		SWATDIR="$(PKG_INSTALL_DIR)/usr/swat" \
#		LOCKDIR="$(PKG_INSTALL_DIR)/var/run/samba" \
#		SAMBABOOK="$(PKG_INSTALL_DIR)/usr/swat/using_samba" \
#		CODEPAGEDIR="$(PKG_INSTALL_DIR)/usr/share/samba" \
#		install
endef

$(eval $(call BuildPackage,samba2))
