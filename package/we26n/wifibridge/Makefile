#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifibridge
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/wifibridge
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=test program for wifibridge
  MAINTAINER:=dwz <weizheng.duan@jianluyou.com>
  DEPENDS:=+libpthread +libubox +libubus +libuci +libevent2-core 
endef

define Package/wifibridge/description
  test program for wifibridge 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/obj
	$(CP) ./src  $(PKG_BUILD_DIR)/
	$(CP) ./test.mk  $(PKG_BUILD_DIR)/
	$(CP) ../enn  $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -Wall -I ./enn" \
	LDFLAGS="$(TARGET_LDFLAGS) -lpthread -lubox -lubus -luci -levent_core" \
	-f test.mk
endef

define Package/wifibridge/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifibridge $(1)/usr/bin/wifibridge
endef

$(eval $(call BuildPackage,wifibridge))

