#!/usr/bin/env lua

require "ubus"
require "luci.sys"


local macReader = io.popen("eth_mac r wifi");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;

http = require "socket.http";
url = "http://101.200.1.101:8001/enngateway/getstate?gatewayid=" .. macAddr;

conn = ubus.connect();
if not conn then
	error("Failed to connect to ubusd");
end

function getCommand()
	resp, code = http.request(url);
	if code == 200 then
		return resp;
	end

	return nil;
end

function getFromZigbeeJianyou(gatewayid_value, deviceid_value, attr_value)
--[[
	local result = conn:call("we26n_zigbee_febee", "getstatecmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value });
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("getFromZigbeeJianyou return nil.");
	end
]]
end

function getFromZigbeeFbee(gatewayid_value, deviceid_value, attr_value)
	local result = conn:call("we26n_zigbee_febee", "getstatecmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value });
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("getFromZigbeeFbee return nil.");
	end
end

function getFrom433Jianyou(gatewayid_value, deviceid_value, attr_value)
--[[
	local result = conn:call("we26n_zigbee_febee", "getstatecmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value });
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("getFrom433Jianyou return nil.");
	end
]]
end

function getStateDispatcher(data)
	
	local cjson = require "cjson";
	local jobj = cjson.decode(data);

	for idx, value in ipairs(jobj) do
		if value ~= nil then
			local devId = value["deviceid"]
			if devId ~= nil then
				local gatewayId = value["gatewayid"];
				local attr = value["attr"];

				local devIdLower = string.lower(devId);
				if string.match(devIdLower, "zigbee_jianyou") == "zigbee_jianyou" then
					getFromZigbeeJianyou(gatewayId, devId, attr);
				elseif string.match(devIdLower, "zigbee_fbee") == "zigbee_fbee" then
					getFromZigbeeFbee(atewayId, devId, attr);
				elseif string.match(devIdLower, "433_jianyou") == "433_jianyou" then
					getFrom433Jianyou(gatewayId, devId, attr);
				end
			end
		end
	end

end

function getWebServerURL()
    local ip = luci.sys.exec("uci get jyconfig.@webserver[0].ip")
    local iplen = string.len(ip)
    local port = luci.sys.exec("uci get jyconfig.@webserver[0].port")
    local portlen = string.len(port)
    ip = string.gsub(ip,"%\r","")
    ip = string.gsub(ip,"%\n","")
    port = string.gsub(port,"%\r","")
    port = string.gsub(port,"%\n","")

    print(ip)
    print(port)
    if iplen ~= 0 and portlen ~= 0 then
        url = "http://" .. ip .. ":" .. port .. "/enngateway/getstate?gatewayid=" .. macAddr;
    end
    print(url)

end


local processing = false;
local t = os.time();

socket = require("socket");

getWebServerURL()

while true do
	local time = os.time();
	if time - t >= 1 and not processing then
		processing = true;

		local result = getCommand();
		if result ~= nil then
			if not pcall(getStateDispatcher, result) then
				print("dispatch command error....");
			end
		else
			print("get command error....");
		end

		processing = false;
		t = time;

		socket.sleep(1);
	end
end
