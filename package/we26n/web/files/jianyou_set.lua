--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require("uci")
local x = uci.cursor()
local nw = require "luci.model.network".init(x)
local io = require "io"
require "luci.fs"
require "luci.sys"
require "ubus"

module("luci.controller.admin.jianyou_set", package.seeall)

function index()
	entry({"admin", "jianyou_set"}, alias("admin", "jianyou_set", "overview"), _("Jianyou_Set"), 21).index = true
	entry({"admin", "jianyou_set", "overview"}, template("newweb/set_sam"), _("Overview"), 1)
	entry({"admin", "jianyou_set", "set_ble"}, call("set_ble"),nil)
	entry({"admin", "jianyou_set", "set_wifi"}, call("set_wifi"),nil)
	entry({"admin", "jianyou_set", "set_power"}, call("set_power"),nil)
	entry({"admin", "jianyou_set", "set_water"}, call("set_water"),nil)
	entry({"admin", "jianyou_set", "set_heat"}, call("set_heat"),nil)
end

function set_ble()

local tm = luci.http.formvalue("tm")
local check = luci.sys.exec("uci get ennconfig.@ble[0].gas_interval")
if string.len(check) == 0 then

    luci.sys.exec("cp /etc/config/ennconfig_ever /etc/config/ennconfig");

end
    luci.sys.exec("uci set ennconfig.@ble[0].gas_interval="..tm)

	x:commit("ennconfig")
    luci.http.redirect(luci.dispatcher.build_url("admin/jianyou_set/overview"))
end

function set_wifi()
local tm = luci.http.formvalue("tm")
local check = luci.sys.exec("uci get ennconfig.@wifi[0].gas_interval")
if string.len(check) == 0 then
    luci.sys.exec("cp /etc/config/ennconfig_ever /etc/config/ennconfig");
end
    luci.sys.exec("uci set ennconfig.@wifi[0].gas_interval="..tm)
	x:commit("ennconfig")
    luci.http.redirect(luci.dispatcher.build_url("admin/jianyou_set/overview"))
end


function set_power()
local tm = luci.http.formvalue("tm")
    local check = luci.sys.exec("uci get ennconfig.@470[0].power_interval")
    if string.len(check) == 0 then
        luci.sys.exec("cp /etc/config/ennconfig_ever /etc/config/ennconfig");
    end
    luci.sys.exec("uci set ennconfig.@470[0].power_interval="..tm)
	x:commit("ennconfig")
    luci.http.redirect(luci.dispatcher.build_url("admin/jianyou_set/overview"))
end

function set_water()
local tm = luci.http.formvalue("tm")
local check = luci.sys.exec("uci get ennconfig.@470[0].water_interval")
if string.len(check) == 0 then
    luci.sys.exec("cp /etc/config/ennconfig_ever /etc/config/ennconfig");
end
    luci.sys.exec("uci set ennconfig.@470[0].water_interval="..tm)
	x:commit("ennconfig")
    luci.http.redirect(luci.dispatcher.build_url("admin/jianyou_set/overview"))
end


function set_heat()
local tm = luci.http.formvalue("tm")
local check = luci.sys.exec("uci get ennconfig.@470[0].heat_interval")
if string.len(check) == 0 then
    luci.sys.exec("cp /etc/config/ennconfig_ever /etc/config/ennconfig");
end
    luci.sys.exec("uci set ennconfig.@470[0].heat_interval="..tm)
	x:commit("ennconfig")
    luci.http.redirect(luci.dispatcher.build_url("admin/jianyou_set/overview"))
end



