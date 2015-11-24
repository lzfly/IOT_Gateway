module("luci.controller.admin.newweb",package.seeall)

require("uci")
local x = uci.cursor()
local nw = require "luci.model.network".init(x)
local io = require "io"
require "luci.fs"
require "luci.sys"

require "ubus"

conn = ubus.connect();
if not conn then
	error("Failed to connect to ubusd");
end
local macReader = io.popen("eth_mac r lan");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;

--local uci = require "luci.controller.admin.uci"

function index()
    entry({"admin", "newweb"}, alias("admin", "newweb","newweb"), _("new web"), 10).index = true
	--index
	entry({"admin", "newweb", "newweb"}, call("gotoindex"), _("new web"), 0)
    entry({"admin", "newweb", "newweb_index"}, template("newweb/index"), _("new web"), 1)
	--上网带宽管理
    entry({"admin", "newweb", "band_manage"}, template("newweb/band_manage"), _("band_manage"), 2)
	--智能分配
	entry({"admin", "newweb", "qos_enable"}, call("qos_enable"), nil)
	--
	entry({"admin", "newweb", "bandwidth_ensure"}, call("bandwidth_ensure"), nil)
	--绿色节能
    entry({"admin", "newweb", "energy_save"}, template("newweb/energy_save"), _("energy_save"), 3)
	--信号灯设置
		entry({"admin", "newweb", "ls_set"}, call("ls_set"), nil)
		--信号灯时间
		entry({"admin", "newweb", "lt_set"}, call("lt_set"), nil)
		--wifi设置
		entry({"admin", "newweb", "ws_set"}, call("ws_set"), nil)
		--wifi时间
		entry({"admin", "newweb", "wt_set"}, call("wt_set"), nil)
	--设备管理
    entry({"admin", "newweb", "equ_manage"}, template("newweb/equ_manage"), _("equ_manage"), 4)
    entry({"admin", "newweb", "upgade"}, template("newweb/upgrade"), _("upgrade"), 40)
	--设备管理
	entry({"admin", "newweb", "device_manager"}, call("device_manager"), nil)
	entry({"admin", "newweb", "device_refresh"}, call("device_refresh"), nil)
	entry({"admin", "newweb", "traffic"}, call("traffic"), nil)
	--绿色上网通道
    entry({"admin", "newweb", "green_internet"}, template("newweb/green_internet"), _("green_internet"), 5)
		--增加限制访问域名
		entry({"admin", "newweb", "addurl"}, call("addurl"), nil)
		--倒计时设置
		entry({"admin", "newweb", "time_set"}, call("time_set"), nil)
		--删除限制访问域名
		entry({"admin", "newweb", "delurl"}, call("delurl"), nil)
	--宽带上网设置
    entry({"admin", "newweb", "network_set"}, template("newweb/network_set"), _("network_set"), 8)
		--pppoe上网方式
		entry({"admin", "newweb", "pppoe_update"}, call("pppoe_update"), nil)
		--static 上网方式
		entry({"admin", "newweb", "static_update"}, call("static_update"), nil)
		--动态IP上网方式
		entry({"admin", "newweb", "dhcp_update"}, call("dhcp_update"), nil)
		--无线中继上网
		entry({"admin", "newweb", "wds_update"}, call("wds_update"), nil)
	--存储中心
    entry({"admin", "newweb", "store_center"}, template("newweb/store_center"), _("store_center"), 9)
		entry({"admin", "newweb", "store"}, call("store"), nil)
		entry({"admin", "newweb", "prepareup"}, call("prepareup"), nil)
		--entry({"admin", "newweb", "upload_file"}, call("upload_file"), nil)
	
	--无线设置
    entry({"admin", "newweb", "wireless_set"}, template("newweb/wireless_set"), _("wireless_set"), 10)
		--无线设置保存调用
		entry({"admin", "newweb", "wireless_update"}, call("wireless_update"), nil)
	
    -- entry({"admin", "network", "iface_reconnect"}, call("iface_reconnect"), nil)
	--Zigbee设备
    entry({"admin", "newweb", "zigbee"}, template("newweb/zigbee"), _("zigbee"), 11)
	entry({"admin", "newweb", "windows"}, template("newweb/windows"), _("windows"), 12)
	entry({"admin", "newweb", "elewater"}, template("newweb/elewater"), _("elewater"), 13)
	entry({"admin", "newweb", "switch"}, template("newweb/switch"), _("switch"), 14)
	entry({"admin", "newweb", "sensor"}, template("newweb/sensor"), _("sensor"), 15)
	entry({"admin", "newweb", "alertor"}, template("newweb/alertor"), _("alertor"), 16)
	entry({"admin", "newweb", "bluetooth"}, template("newweb/bluetooth"), _("bluetooth"), 17)
	entry({"admin", "newweb", "safe"}, template("newweb/safe"), _("safe"), 18)
	entry({"admin", "newweb", "wifi"}, template("newweb/wifi"), _("wifi"), 19)
	entry({"admin", "admin_system", "safe"}, template("admin_system/safe"), _("safe"), 20)
        entry({"admin","newweb","light_control"},call("light_control"),nil)
        entry({"admin","newweb","entrynet_control"},call("entrynet_control"),nil)
		entry({"admin","newweb","gas_meter_set"},call("gas_meter_set"),nil)
		entry({"admin","newweb","blegas_meter_set"},call("blegas_meter_set"),nil)
		entry({"admin","newweb","password_set"},call("password_set"),nil)
		entry({"admin","newweb","elewater_item"},call("elewater_item"),nil)
		entry({"admin","newweb","bluetooth_item"},call("bluetooth_item"),nil)
		entry({"admin","newweb","wifi_item"},call("wifi_item"),nil)
		entry({"admin","newweb","zigbee_item"},call("zigbee_item"),nil)
		entry({"admin","newweb","windows_item"},call("windows_item"),nil)
		entry({"admin","newweb","switch_item"},call("switch_item"),nil)
		entry({"admin","newweb","sensor_item"},call("sensor_item"),nil)
		entry({"admin","newweb","alertor_item"},call("alertor_item"),nil)
		entry({"admin","newweb","pair470_control"},call("pair470_control"),nil)
		
		entry({"admin","newweb","CB_add"},call("CB_add"),nil)
		entry({"admin","newweb","CB_del"},call("CB_del"),nil)
		entry({"admin","newweb","CB_tem_set"},call("CB_tem_set"),nil)
		

end

function CB_tem_set()
   local temp=luci.http.formvalue("data")
   local result = conn:call("we26n_CB","ctrlcmd",{gatewayid = "we26n_78A351097F44",deviceid = "CB_controller_5EC5D4B5009540000001",devicetype = "0032",attr = "1014",data = temp})
     luci.http.redirect(luci.dispatcher.build_url("admin/newweb/wifi"))
 end



function CB_add()
    local deviceid = luci.http.formvalue("deviceid");
    local devicetype=luci.http.formvalue("devicetype")
    local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
     luci.http.redirect(luci.dispatcher.build_url("admin/newweb/wifi"))
end

function CB_del()
    local deviceid = luci.http.formvalue("deviceid");
    local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
     luci.http.redirect(luci.dispatcher.build_url("admin/newweb/wifi"))
end


function gotoindex()
    luci.http.redirect(luci.dispatcher.build_url("admin/newweb/newweb_index"))
end

function elewater_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/elewater"))


   end
   
function bluetooth_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/bluetooth"))


   end
   
function wifi_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/wifi"))


   end
   
function zigbee_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/zigbee"))


   end
   
function windows_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/windows"))


   end
   
function switch_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/switch"))


   end

    
function sensor_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/sensor"))


   end  
   
function alertor_item()
  local deviceid=luci.http.formvalue("devId")
  local devicetype=luci.http.formvalue("devicetype")
  local status=luci.http.formvalue("status")
  --local device ="&devieid="..deviceid.."&devicetype="..devicetype
  if status == "-1" then
  local result=conn:call("deviceid_add_del", "add_deviceid", { deviceid = deviceid, devicetype = devicetype});
  else
  local result=conn:call("deviceid_add_del", "del_deviceid", { deviceid = deviceid});
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/alertor"))

   end

   
function light_control()
  
  luci.http.redirect(luci.dispatcher.build_url("admin/newweb/zigbee"))
end
function entrynet_control()

	local result = conn:call("we26n_zigbee_febee", "ctrlcmd", { gatewayid =macAddr , deviceid = "zigbee_fbee_entrynet_ffffffffffffffff_99", attr = "9999", data = "0" });
        luci.http.redirect(luci.dispatcher.build_url("admin/newweb/zigbee"))
end

function pair470_control()

	local result = conn:call("we26n_rfmodbus", "startpair", {});
	
        luci.http.redirect(luci.dispatcher.build_url("admin/newweb/elewater"))
end

function get_470_state()

local ret = conn:call( "we26n_rfmodbus", "getstate", {} );
	if ret.result == "0" and ret.state == "4" then
	    luci.http.prepare_content("text/plain")
	    luci.http.write("pairing ok")
	else
	    luci.http.prepare_content("text/plain")
	    luci.http.write("pair fail")
	end

end


function gas_meter_set()

	local c_gas_id=luci.http.formvalue("id")
	
	
	luci.sys.exec("uci set jyconfig.@deviceid[0].gasmeter="..c_gas_id)

	x:commit("jyconfig")
		
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/wifi"))
end

function blegas_meter_set()

	local c_blegas_id=luci.http.formvalue("id")
	
	luci.sys.exec("uci set jyconfig.@deviceid[0].blegasmeter="..c_blegas_id)

	x:commit("jyconfig")
	
	local result = conn:call("we26n_change_MAC", "change_ble_MAC", {});
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/bluetooth"))
end

function password_set()

	local v1=luci.http.formvalue("text2")
	local v2=luci.http.formvalue("text3")
	
	if v1 and v2 and #v1 > 0 and #v2 > 0 then
                if v1 == v2 then
                        if luci.sys.user.setpasswd(luci.dispatcher.context.authuser, v1) == 0 then
                                
                        else
                               
                        end
                else
                        
                end
        end

		
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/safe"))
end

function wireless_update()
	local ssid = luci.http.formvalue("wl_ssid")
	if not ssid or ssid == "" then 
	    return
	end
    local key = luci.http.formvalue("wl_wpa_psk")
    local start = luci.http.formvalue("dhcp_start")
    local limit = luci.http.formvalue("dhcp_end")
    local ipaddr = luci.http.formvalue("lan_ipaddr") 
	local winame = luci.http.formvalue("winame")
   --[[ 
    local slegal = start
    slegal = slegal .. "."
    local sslegal =  Split(slegal , ".")
    if sslegal[1] =="" or ssslegal[1] >= 255 then
        return;
    end 
    ]]--
	if not winame or winame == "" then
		winame = x:add("wireless", "wifi-iface")
		x:set("wireless", winame, "device", "mt7620")
		x:set("wireless", winame, "network", "lan")
		x:set("wireless", winame, "mode", "ap")
	end
	x:set("wireless", winame, "ssid", ssid)
	if not key or key == "" then
		x:set("wireless", winame, "encryption", "none")
		x:delete("wireless", winame, "key")
	else
		x:set("wireless", winame, "encryption", "psk-mixed")
		x:set("wireless", winame, "key", key)
	end
	x:commit("wireless")

    x:set("network", "lan", "ipaddr", ipaddr)
	x:commit("network")
    x:set("dhcp", "lan", "start", start)
    x:set("dhcp", "lan", "limit", limit)
	x:commit("dhcp")
	
	luci.sys.call("/etc/init.d/network restart >/dev/null")
    luci.http.redirect(luci.dispatcher.build_url("admin/newweb/wireless_set"))
end

function pppoe_update()
	local newname = luci.http.formvalue("wan_pppoe_username")
    local pwd = luci.http.formvalue("wan_pppoe_passwd")
    local bandwidth = luci.http.formvalue("pppoe_total")	

   	x:set("network", "wan", "username", newname)
	x:set("network", "wan", "password", pwd)
	x:set("network","wan","proto", "pppoe")
	x:delete("network","wan","ipaddr") 
	x:delete("network","wan","netmask") 
	x:delete("network","wan","gateway") 
	x:delete("network","wan","dns")
	x:delete("network", "wwan")
	x:commit("network")

   	x:set("band","band_manage","bandwidth", bandwidth)
	x:set("band","lan_set","lan_mode", 1)
	x:commit("band")

	local winame = ""
	x:foreach("wireless", "wifi-iface",
		function(s)
			if s['mode'] == "sta" then
				winame = s['.name']
				return false
			end
		end)
	if winame ~= "" then 
		x:delete("wireless", winame)
		x:commit("wireless")
	end
	
	luci.sys.call("/etc/init.d/network restart >/dev/null")
    luci.http.redirect(luci.dispatcher.build_url("admin/newweb/network_set"))
end

function static_update()
	local s_ip = luci.http.formvalue("wan_ipaddr")
    local s_netmask = luci.http.formvalue("wan_netmask")
    local s_gateway = luci.http.formvalue("wan_gateway")	
    local s_dns = luci.http.formvalue("wan_dns0")
    local s_bandwidth = luci.http.formvalue("network_bandwidth_total")

   	x:set("network", "wan", "ipaddr", s_ip)
	x:set("network", "wan", "netmask", s_netmask)
	x:set("network", "wan", "gateway", s_gateway)
	x:set("network", "wan", "dns", s_dns)
	x:set("network","wan","proto", "static")
	x:delete("network", "wwan")
	x:commit("network")

   	x:set("band","band_manage","bandwidth", s_bandwidth)
	x:set("band","lan_set","lan_mode", 2)
	x:commit("band")
	
	local winame = ""
	x:foreach("wireless", "wifi-iface",
		function(s)
			if s['mode'] == "sta" then
				winame = s['.name']
				return false
			end
		end)
	if winame ~= "" then 
		x:delete("wireless", winame)
		x:commit("wireless")
	end

	luci.sys.call("/etc/init.d/network restart >/dev/null")
    luci.http.redirect(luci.dispatcher.build_url("admin/newweb/network_set"))
end

function dhcp_update()
    local d_bandwidth = luci.http.formvalue("network_bandwidth_total")
   	x:set("band","band_manage","bandwidth", d_bandwidth)
	x:set("band","lan_set","lan_mode", 3)
	x:commit("band")

	x:set("network","wan","proto", "dhcp")
	x:delete("network","wan","ipaddr") 
	x:delete("network","wan","netmask") 
	x:delete("network","wan","gateway") 
	x:delete("network","wan","dns")
	x:delete("network", "wwan")
	x:commit("network")

	local winame = ""
	x:foreach("wireless", "wifi-iface",
		function(s)
			if s['mode'] == "sta" then
				winame = s['.name']
				return false
			end
		end)
	if winame ~= "" then 
		x:delete("wireless", winame)
		x:commit("wireless")
	end
	
	luci.sys.call("/etc/init.d/network restart >/dev/null")
    luci.http.redirect(luci.dispatcher.build_url("admin/newweb/network_set"))
end

function wds_update()
	local d_name = luci.http.formvalue("relay_username")
    local d_pwd = luci.http.formvalue("wan_relay_passwd")
    local d_bandwidth = luci.http.formvalue("network_bandwidth_total")
	x:set("band","band_manage","bandwidth", d_bandwidth)
	x:set("band","lan_set","lan_mode", 4)
	x:commit("band")

	x:set("network", "wwan", "interface")
	x:set("network", "wwan", "proto", "dhcp")
	x:commit("network")

	local winame = ""
	x:foreach("wireless", "wifi-iface",
		function(s)
			if s['mode'] == "sta" then
				winame = s['.name']
				return false
			end
		end)
	if winame == "" then 
		winame = x:add("wireless", "wifi-iface")
	end
	x:set("wireless", winame, "device", "radio0")
	x:set("wireless", winame, "network", "wwan")
	x:set("wireless", winame, "mode", "sta")
	x:set("wireless", winame, "ssid", d_name)

	if not d_pwd or d_pwd == "" then
		x:set("wireless", winame, "encryption", "none")
		x:delete("wireless", winame, "key")
	else
		x:set("wireless", winame, "encryption", "psk-mixed")
		x:set("wireless", winame, "key", d_pwd)
	end
	x:commit("wireless")

	luci.sys.call("/etc/init.d/network restart >/dev/null")
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/network_set"))
end

function time_set()
	local c_time=luci.http.formvalue("count_time")
	local s_time=luci.http.formvalue("st_time")
	local e_time=luci.http.formvalue("en_time")
	local c_switch=luci.http.formvalue("count_switch")
	local n_switch=luci.http.formvalue("net_switch")
	local end2 = os.date("%Y%m%d%H%M", os.time() + tonumber(c_time)*3600)
	
	x:set("green","switch","start", s_time)
	x:set("green","switch","end", e_time)
	x:set("green","switch","t0", c_time)
	x:set("green","switch","c_switch", c_switch)
	x:set("green","switch","n_switch", n_switch)
	x:set("green","switch","end2", end2)
	x:commit("green")
		
	x:set("notify", "notify", "green_rule_changed", "1")
	x:commit("notify")
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/green_internet"))
end

function addurl()
	local n_addr=luci.http.formvalue("greenwall_network_addr")
	luci.sys.exec("uci add_list green.addrlist.net_addr=" ..n_addr)
	
	x:commit("green")

	x:set("notify", "notify", "ban_list_changed", "1")
	x:commit("notify")

	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/green_internet"))
end

function delurl()
	local d_addr=luci.http.formvalue("greenwall_network_deleate_addr")
	luci.sys.exec("uci del_list green.addrlist.net_addr=" ..d_addr)
	x:commit("green")

	x:set("notify", "notify", "ban_list_changed", "1")
	x:commit("notify")

	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/green_internet"))
end

function qos_enable()
	local q_enable=luci.http.formvalue("smart_qos_enable")
	x:set("band","band_manage","qos_enable", q_enable)
	x:commit("band")

	x:set("notify", "notify", "band_manage_changed", "1")
	x:commit("notify")

	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/band_manage"))
end

function bandwidth_ensure()
	local net_band=luci.http.formvalue("network_bandwidth_ensure")
	local n_band=Split(net_band,",")
	local size=(table.getn(n_band)-1)
	luci.sys.exec("uci delete band.mac_list.mac")
	for i=1,size,1 do
		local new_band=n_band[i]
		new_band=string.trim(new_band)
		if new_band ~= "" then
    		luci.sys.exec("uci add_list band.mac_list.mac=" ..new_band)
		end
    end
	--x:set("band","mac_list","mac", net_band)
	x:commit("band")

	x:set("notify", "notify", "band_manage_changed", "1")
	x:commit("notify")

	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/band_manage"))
end

function  device_manager()	
	local dev=luci.http.formvalue("device_info")
	local device=Split(dev,",")
	local size=(table.getn(device)-1)
	luci.sys.exec("uci delete devices.devices.device")
	for i=1,size,1 do
		local new_device=device[i]
		new_device=string.trim(new_device)
		luci.sys.exec("uci add_list devices.devices.device=" ..new_device)
    end
	--x:set("band","mac_list","mac", net_band)
	x:commit("devices")
	
	x:set("notify", "notify", "device_group_changed", "1")
	x:commit("notify")

	--luci.http.redirect(luci.dispatcher.build_url("admin/newweb/equ_manage"))
end

function  device_refresh()
	local name_list=luci.sys.exec("uci get devices.devices.device")
	name_list = string.trim(name_list)

	luci.http.prepare_content("text/plain")
	luci.http.write(name_list)

	--luci.http.redirect(luci.dispatcher.build_url("admin/newweb/equ_manage"))
end

function  traffic()
	--luci.http.redirect(luci.dispatcher.build_url("admin/newweb/equ_manage"))
    local s = require "luci.tools.status"
	local devs = s.dhcp_leases() or {}
	local traffic = ""
	for key, value in pairs(devs) do
	    traffic = traffic .. value.macaddr .. ",0,0#"
	end
	luci.http.prepare_content("text/plain")
	luci.http.write(traffic)
end

function ls_set()
	local led_switch=luci.http.formvalue("green_energy_led_switch")	
	x:set("energy","green_energy","led_switch", led_switch)
	x:commit("energy")
	if led_switch == "0" then
		x:set("system", "led_power", "default", 0)

		x:set("system", "led_internet", "trigger", "none")
		x:delete("system", "led_internet", "dev")
		x:delete("system", "led_internet", "mode")
		x:set("system", "led_internet", "default", 0)

		x:set("system", "led_wifi_led", "trigger", "none")
		x:delete("system", "led_wifi_led", "dev")
		x:delete("system", "led_wifi_led", "mode")
		x:set("system", "led_wifi_led", "default", 0)
	else
		x:set("system", "led_power", "default", 1)

		x:set("system", "led_internet", "trigger", "netdev")
		x:set("system", "led_internet", "dev", "eth0.2")
		x:set("system", "led_internet", "mode", "link tx rx")

		x:set("system", "led_wifi_led", "trigger", "netdev")
		x:set("system", "led_wifi_led", "dev", "wlan0")
		x:set("system", "led_wifi_led", "mode", "link tx rx")
	end
	x:commit("system")
	luci.sys.call("/etc/init.d/led restart >/dev/null")
	
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/energy_save"))
end

function lt_set()
	local led_time=luci.http.formvalue("green_energy_led_off_time")	
	x:set("energy","green_energy","led_time", led_time)
	x:commit("energy")
	
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/energy_save"))
end

function ws_set()
	local wifi_switch=luci.http.formvalue("green_energy_wifi_signal")	
	x:set("energy","green_energy","wifi_switch", wifi_switch)
	x:commit("energy")
	if wifi_switch == "0" then
		x:set("wireless", "radio0", "txpower", 0)
	elseif wifi_switch == "1" then
		x:set("wireless", "radio0", "txpower", 7)
	elseif wifi_switch == "2" then
		x:set("wireless", "radio0", "txpower", 14)
	else
		x:set("wireless", "radio0", "txpower", 20)
	end
	local disabled = "1"
	if wifi_switch ~= "0" then
		disabled = "0"
	end
	x:foreach("wireless", "wifi-iface",
		function(s)
			x:set("wireless", s['.name'], "disabled", disabled)
		end)
	x:commit("wireless")
		
	luci.sys.call("/etc/init.d/network restart >/dev/null")
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/energy_save"))
end

function wt_set()
	local wifi_time=luci.http.formvalue("green_energy_wifi_off_time")	
	x:set("energy","green_energy","wifi_time", wifi_time)
	x:commit("energy")
	luci.http.redirect(luci.dispatcher.build_url("admin/newweb/energy_save"))
end



function store()
	--print("hi")
	--local s_dir=x:get("store","store_fs","start_dir")
	--local filename = s_dir .. luci.http.formvalue("filename");
	--[[local filename="/usr/ooo/"
	local file_name="/mnt/usr/"	
	local file="/usr/www/+/usr/mnt/+"]]--
	local action = luci.http.formvalue("action");
	local filename=luci.http.formvalue("filename");
	
	--filename=filename.."+"
	--local filenam="/usr/*"
	if action == "delete" then 
		os.remove(filename)
	elseif action == "new" then
		os.execute("mkdir " ..filename)
	elseif action == "rename" then		
		filename=filename.."+"
		local files=Split(filename,"+")
		os.rename(files[1], files[2])
		--luci.http.prepare_content("text/plain")
		--luci.http.write(filename .. ":" .. files[1] .. ":" .. files[2])
	elseif action == "move" then
		filename=filename.."+"
		local files_m=Split(filename,"+")
		os.rename(files_m[1],files_m[2])
		os.remove(files_m[1])
	elseif action == "copy" then 
		filename=filename.."+"
		local files_c=Split(filename,"+")
		local files_c=Split(filename,"+")		
		--os.rename(files_c[1],files_c[2])
		luci.sys.exec("cp -a "..files_c[1].." "..files_c[2])
		luci.http.prepare_content("text/plain")
		luci.http.write(files_c[1] .. ":" .. files_c[2])
	elseif action == "list"  then
		local filena=""
		local name_l={}
		filenam=string.gsub(filename,"*","")
		local filearra=""
	    dir=luci.sys.exec("ls -l "..filenam)
		--dir="/usr/mmm/,/usr/nnn/,"
		--local dir=string.trim(dir)
		--print(dir)
		d=string.gsub(dir,"\n",",")
		--print(d)
		local dir_l=Split(d,",")
		local name=Split(d,",")
		local dirs_l={}
		local size_d={}
		local time_d={}
		local l_type={}
		local filearray=""	
		--[[file=io.open("dir","r")
			if file~=nil then
			for l in file:line() do
			print(l)
			end
			end

		file:close()]]--
		local size_l=(table.getn(dir_l)-1)
		--print(size_l)
		for j=1,size_l,1 do
			tmp=dir_l[j]
			l_type[j]=string.sub(tmp,0,1)
			name[j]=string.sub(tmp,58,string.len(tmp))
			--name[j]=string.gsub(name_l[j],"|","\ ")
			dir_l[j]=string.sub(tmp,30,57)
			dirs_l[j]=name[j]..dir_l[j]
			
			--print(name[j])
			--time_d[j]=string.sub(tmp,45,57)
			--path_l=filename..name[j]
			time_d[j]=getTimeStamp(luci.fs.mtime(filenam..name[j]))
			
			size_d[j]=string.sub(tmp,30,44)
			size_d[j]=string.gsub(size_d[j], "^%s*(.-)%s*$", "%1")
			--print(name[j].." ".."time="..time_d[j].." ".."size="..size_d[j])
			--print(dir_l[j])
			--print(dirs_l[j])
			filearray=filearray.."/"..l_type[j]..filenam..name[j].."*"..size_d[j].."*"..time_d[j].."#"
			--print(filearray)
			--print(filenam)
		
			--print(111)
		end
		
		 luci.http.prepare_content("text/plain")
		 luci.http.write(filearray)


--print(filearray)
	end
	
	

--[[	if action == "list" then
		luci.http.prepare_content("applicatoin/json; charset=UTF-8");
		local dirs = luci.fs.dir(filename);	
		local dirinfo= {}			
		if dirs then		
			local size=(table.getn(dirs)-1);	
			if size>0 then								
				for i=1,size,1 do
					local cur = {}
					if luci.fs.isdirectory(dirs[i]) then
						cur["isdirectory"] = true;		
						cur["filename"] = dirs[i];
						cur["modifytime"] = luci.fs.mtime(dirs[i]);
						cur["filesize"] = 0;
					else 					
						cur["isdirectory"] = false;		
						cur["filename"] = dirs[i];
						cur["modifytime"] = luci.fs.mtime(dirs[i]);
						cur["filesize"] = luci.fs.stat(dirs[i], "size");
					end		
					table.insert(dirinfo, cur);		
				end
				luci.http.write_json(dirinfo);
			end
		end 
	else	
		luci.http.prepare_content("application/x-www-form-urlencoded; charset=UTF-8")
		luci.http.write("1 /test/notlist *32768000*2015-01-01     5: 23#")
    end	
	]]--
	--luci.http.prepare_content("application/x-www-form-urlencoded; charset=UTF-8")
	--luci.http.write("/mnt/mmcblk0p1 *32768000*2015-01-01     5: 23#")
	
	
end


function prepareup()	
	local up_path=luci.http.formvalue("filename");	
	local action=luci.http.formvalue("action")	
	if action == "up"  then
	luci.sys.exec("cp -a "..up_path.." ".."/mnt/")
	end
end




--[[up_path = "";
function prepareup()	
	up_path=luci.http.formvalue("filename");		
	luci.http.prepare_content("application/x-www-form-urlencoded; charset=UTF-8")
	luci.http.write(up_path);	
end

function upload_file()
	luci.http.prepare_content("application/x-www-form-urlencoded; charset=UTF-8")
	luci.http.write("test")
	luci.http.write(up_path)
	
	local fp
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not fp then
				luci.http.write("fp = io.open("..up_path..",\"w\")");
				fp = io.open(up_path, "w");
			end
			if chunk then
				luci.http.write("chunk is write!");
				fp:write(chunk)
			end
			if eof then
				luci.http.write("fp:close()");
				fp:close()
			end
		end
	)	
	
	luci.http.write("upload_file end");
end
]]--
function Split(szFullString, szSeparator)  
	local nFindStartIndex = 1  
	local nSplitIndex = 1  
	local nSplitArray = {}  
	while true do  
	   local nFindLastIndex = string.find(szFullString, szSeparator, nFindStartIndex)  
	   if not nFindLastIndex then  
		nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, string.len(szFullString))  
		break  
	   end  
	   nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, nFindLastIndex - 1)  
	   nFindStartIndex = nFindLastIndex + string.len(szSeparator)  
	   nSplitIndex = nSplitIndex + 1  
	end  
	return nSplitArray  
end     

function getTimeStamp(t)
    return os.date("%Y-%m-%d %H:%M:%S",t)
end
