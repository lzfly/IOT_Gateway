#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=web
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/web
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=theme 
  MAINTAINER:=duanweizheng <weizheng.duan@jianluyou.com>
  DEPENDS:=+lua +lua-cjson +luasocket
endef

define Package/web/description
  theme
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/web/install
	
	$(INSTALL_DIR)  $(1)/usr/lib/lua/luci/controller/admin/
	$(INSTALL_DIR)  $(1)/www/
	$(INSTALL_DIR) 	$(1)/www/luci-static/resources/
	$(INSTALL_DIR) 	$(1)/www/luci-static/bootstrap/
	$(INSTALL_DIR) 	$(1)/usr/lib/lua/luci/view/themes/bootstrap/
	$(INSTALL_DIR)  $(1)/usr/lib/lua/luci/view/
	$(INSTALL_DIR)  $(1)/usr/lib/lua/luci/view/thems/
	$(INSTALL_DIR)  $(1)/usr/lib/lua/luci/view/admin_system/
	$(INSTALL_DIR)  $(1)/etc/hotplug.d/block/
	$(INSTALL_BIN) ./files/newweb.lua  $(1)/usr/lib/lua/luci/controller/admin/newweb.lua
	$(INSTALL_BIN) ./files/jianyou_set.lua  $(1)/usr/lib/lua/luci/controller/admin/jianyou_set.lua
	$(INSTALL_BIN) ./files/cascade.css  $(1)/www/luci-static/bootstrap/cascade.css
	$(INSTALL_BIN) ./files/index.htm  $(1)/www/index.htm
	$(CP) ./files/resources/  $(1)/www/luci-static/
	$(CP) ./files/newweb/  $(1)/usr/lib/lua/luci/view/
	$(CP) ./files/bootstrap/header.htm $(1)/usr/lib/lua/luci/view/themes/bootstrap/header.htm
	$(CP) ./files/bootstrap/footer.htm $(1)/usr/lib/lua/luci/view/themes/bootstrap/footer.htm
	$(CP) ./files/sysauth.htm $(1)/usr/lib/lua/luci/view/sysauth.htm
	$(CP) ./files/flashops.htm $(1)/usr/lib/lua/luci/view/admin_system/flashops.htm
	$(CP) ./files/upgrade.htm $(1)/usr/lib/lua/luci/view/admin_system/upgrade.htm
	$(CP) ./files/newweb/index.htm $(1)/usr/lib/lua/luci/view/newweb/index.htm
	$(CP) ./files/newweb/wifi.htm $(1)/usr/lib/lua/luci/view/newweb/wifi.htm
	$(CP) ./files/newweb/set_sam.htm $(1)/usr/lib/lua/luci/view/newweb/set_sam.htm
	$(CP) ./files/30-usbmount  $(1)/etc/hotplug.d/block/

endef

$(eval $(call BuildPackage,web))
