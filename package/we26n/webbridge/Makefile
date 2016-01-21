#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webbridge
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/webbridge
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=webbridge app between web and iot module (zigbee and rf433)
  MAINTAINER:=wangxiaoming <softwaiter@126.com>
  DEPENDS:=+lua +lua-cjson +luasocket
endef

define Package/webbridge/description
  webbridge app between web and iot module (zigbee and rf433)
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/webbridge/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/datareport.lua  $(1)/usr/bin/datareport.lua
	$(INSTALL_BIN) ./files/commanddispatcher.lua  $(1)/usr/bin/commanddispatcher.lua
	$(INSTALL_BIN) ./files/syncdevstate.lua  $(1)/usr/bin/syncdevstate.lua
	$(INSTALL_BIN) ./files/webdevice.lua  $(1)/usr/bin/webdevice.lua
	$(INSTALL_BIN) ./files/deviceid_list_download.lua  $(1)/usr/bin/deviceid_list_download.lua
	$(INSTALL_BIN) ./files/ennconfig_download.lua  $(1)/usr/bin/ennconfig_download.lua
	$(INSTALL_BIN) ./files/userid_download.lua  $(1)/usr/bin/userid_download.lua
	$(INSTALL_BIN) ./files/webbridge_device.lua  $(1)/usr/bin/webbridge_device.lua
	$(INSTALL_BIN) ./files/gatewayreport.lua  $(1)/usr/bin/gatewayreport.lua
endef

$(eval $(call BuildPackage,webbridge))
