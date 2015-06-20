#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=flowmeter
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/flowmeter
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=flowmeter reader (using uart to 433M module)
  MAINTAINER:=wangmingdong <wang_leaf@sina.com>
  DEPENDS:=+libpthread +libubus +libubox
endef

define Package/flowmeter/description
  flowmeter reader (using uart to 433M module)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/obj
	$(CP) ./src  $(PKG_BUILD_DIR)/
	$(CP) ./test.mk  $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -Wall" \
	LDFLAGS="$(TARGET_LDFLAGS) -lubox -lubus" \
	-f test.mk
endef

define Package/flowmeter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/flowmeter $(1)/usr/bin/flowmeter
endef

$(eval $(call BuildPackage,flowmeter))
