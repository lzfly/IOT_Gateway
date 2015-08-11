#
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stm32ctrl
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/stm32ctrl
  SECTION:=we26n
  CATEGORY:=WE26N
  TITLE:=stm32ctrl, send command to stm32 chip.
  MAINTAINER:=wangmingdong <wang_leaf@sina.com>
  DEPENDS:=+libpthread +libubus +libubox
endef

define Package/stm32ctrl/description
  stm32ctrl, send command to stm32 chip.
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
	LDFLAGS="$(TARGET_LDFLAGS) -lubox -lubus -lpthread" \
	-f test.mk
endef

define Package/stm32ctrl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/stm32ctrl $(1)/usr/bin/stm32ctrl
endef

$(eval $(call BuildPackage,stm32ctrl))
