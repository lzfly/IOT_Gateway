#!/usr/bin/env lua

require "ubus"
require "luci.sys"
require "uloop"

uloop.init();



local macReader = io.popen("eth_mac r lan");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;

http = require "socket.http";
url = "http://10.4.44.210:8001/enngateway/ctrl?gatewayid=" .. macAddr;

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

function sendToZigbeeJianyou(gatewayid_value, deviceid_value, attr_value, data_value)
--[[
	local result = conn:call("we26n_zigbee_febee", "ctrlcmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value, data = data_value });
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("sendToZigbeeJianyou return nil.");
	end
]]
end

function sendToZigbeeFbee(gatewayid_value, deviceid_value, attr_value, data_value)
	local result = conn:call("we26n_zigbee_febee", "ctrlcmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value, data = data_value });
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("sendToZigbeeFbee return nil.");
	end

	local result = conn:call("we26n_zigbee_febee", "getstatecmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value});
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("sendToZigbeeFbee return nil.");
	end
end

function sendTo433Jianyou(gatewayid_value, deviceid_value, attr_value, data_value)
--[[
	local result = conn:call("we26n_zigbee_febee", "ctrlcmd", { gatewayid = gatewayid_value, deviceid = deviceid_value, devicetype = "0012", attr = attr_value, data = data_value });
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
	else
		print("sendTo433Jianyou return nil.");
	end
]]
end

function dispatchCommand(data)
	
	local cjson = require "cjson";
	local jobj = cjson.decode(data);

	for idx, value in ipairs(jobj) do
		if value ~= nil then
			local devId = value["deviceid"]
			if devId ~= nil then
				local gatewayId = value["gatewayid"];
				local attr = value["attr"];
				local data = value["data"];

				local devIdLower = string.lower(devId);
				if string.match(devIdLower, "zigbee_jianyou") == "zigbee_jianyou" then
					sendToZigbeeJianyou(gatewayId, devId, attr, data);
				elseif string.match(devIdLower, "zigbee_fbee") == "zigbee_fbee" then
					sendToZigbeeFbee(atewayId, devId, attr, data);
				elseif string.match(devIdLower, "433_jianyou") == "433_jianyou" then
					sendTo433Jianyou(gatewayId, devId, attr, data);
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
        url = "http://" .. ip .. ":" .. port .. "/enngateway/ctrl?gatewayid=" .. macAddr;
    end
    print(url)

end

local processing = false;
local t = os.time();

socket = require("socket");

getWebServerURL()

function  commanddispather()
		    local result = getCommand();
		    print(result);

		        --result = '[{ "gatewayid":"we26n_xxxxxx", "deviceid":"zigbee_fbee_xxxxx", "attr":"001", "data":"989797897897897" },           { "gatewayid":"we26n_xxxxxx", "deviceid":"zigbee_jianyou_xxxxx", "attr":"001", "data":"989797897897897" }]';

   		if result ~= nil then
			if not pcall(dispatchCommand, result) then
				print("dispatch command error....");
			end
		else
			print("get command error....");
		end
		
		ttmr:set(3000);
end		

ttmr = uloop.timer( commanddispather, 3000 );
	
local customMethod = {
	we26n_commanddispather = {
		    command_dispather= {
		    function(req, msg)
		    print("in ubus")
		    conn:reply(req, {code="S00000", message="" .. resp});
		    ttmr:set(1);
		end,{}
       }
      }
}

conn:add(customMethod);

uloop.run();  
	
