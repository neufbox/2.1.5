

include $(TOPDIR)/rules.mk

PKG_NAME:=wide-dhcpv6
PKG_VERSION:=20080615
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_MD5SUM:=1011e165850fe40d3e332dad2bfd30b3

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/wide-dhcpv6/Default
  SECTION:=ipv6
  CATEGORY:=IPv6
  TITLE:=IPv6 DHCPv6 implementaion for BSD and Linux 
  URL:=http://sourceforge.net/projects/wide-dhcpv6
endef

define Package/wide-dhcpv6/description
  WIDE-DHCPv6 is an open-source implementation of Dynamic Host Configuration 
  Protocol for IPv6 (DHCPv6) originally developed by the KAME project. 
  The implementation mainly conforms to the following standards: RFC3315,3319,
  3633,3646,4075,4272,etc.
endef

define Package/wide-dhcpv6-client
	$(call Package/wide-dhcpv6/Default)
	TITLE+= client
endef

define Package/wide-dhcpv6-server
	$(call Package/wide-dhcpv6/Default)
	TITLE+= server
endef

define Package/wide-dhcpv6-relay
	$(call Package/wide-dhcpv6/Default)
	TITLE+= relay
endef

define Package/wide-dhcpv6-ctl
	$(call Package/wide-dhcpv6/Default)
	TITLE+= ctl
endef

define Package/wide-dhcpv6-client/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dhcp6c $(1)/sbin
endef

$(eval $(call BuildPackage,wide-dhcpv6-client))
$(eval $(call BuildPackage,wide-dhcpv6-server))
$(eval $(call BuildPackage,wide-dhcpv6-relay))
$(eval $(call BuildPackage,wide-dhcpv6-ctl))
