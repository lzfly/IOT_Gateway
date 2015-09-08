#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=jyconfig
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/jyconfig
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=jianyou config file
  MAINTAINER:=jyconfig
  DEPENDS=
endef

define Package/jyconfig/description
  jianyou config file
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/jyconfig/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/jyconfig  $(1)/etc/config/
	$(INSTALL_BIN) ./files/devicesid_list  $(1)/etc/config/
endef

$(eval $(call BuildPackage,jyconfig))
