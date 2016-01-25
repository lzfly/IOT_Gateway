#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtbridge2
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/mtbridge2
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=mtbridge2 test program.
  MAINTAINER:=wangmingdong <wang_leaf@sina.com>
  DEPENDS:=+liblua +libpthread +libubus +libubox +libuv
endef

define Package/mtbridge2/description
  mtbridge2 client test program.
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
	CFLAGS="$(TARGET_CFLAGS) -Wall -g " \
	LDFLAGS="$(TARGET_LDFLAGS) -g -llua -lubox -luv -lpthread -lm" \
	-f test.mk
endef

define Package/mtbridge2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mtbridge2 $(1)/usr/bin/mtbridge2
endef

$(eval $(call BuildPackage,mtbridge2))
