#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=febee
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/febee
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=febee zigbee gateway
  MAINTAINER:=febee
  DEPENDS:=+libpthread
endef

define Package/febee/description
  febee zigbee gateway
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/febee/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/zll  $(1)/usr/bin/zll
	$(INSTALL_BIN) ./files/zlldaemon  $(1)/usr/bin/zlldaemon
endef

$(eval $(call BuildPackage,febee))
