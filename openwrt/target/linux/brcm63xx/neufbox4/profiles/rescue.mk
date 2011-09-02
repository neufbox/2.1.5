#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/rescue
  NAME:=neufbox4 rescue formware
  PACKAGES:=
endef

define Profile/main/Description
  Package set compatible with neufbox4 rescue firmware
endef

$(eval $(call Profile,rescue))

