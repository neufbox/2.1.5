#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/main
  NAME:=neufbox4 main firmware
  PACKAGES:=
endef

define Profile/main/Description
	Package set for neufbox4 main firmware
endef

$(eval $(call Profile,main))
