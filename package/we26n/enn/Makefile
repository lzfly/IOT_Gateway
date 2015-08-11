#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ennconfig
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/ennconfig
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=enn config file
  MAINTAINER:=ennconfig
  DEPENDS=
endef

define Package/ennconfig/description
  enn config file
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/ennconfig/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/ennconfig  $(1)/etc/config/
endef

$(eval $(call BuildPackage,ennconfig))
