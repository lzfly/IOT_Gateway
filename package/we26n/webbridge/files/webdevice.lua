#!/usr/bin/env lua

require "ubus"

require("uci")
local x = uci.cursor()
local nw = require "luci.model.network".init(x)
local io = require "io"
require "luci.fs"
require "luci.sys"
local LAMP_TYPE="1";
local WINDOWS_TYPE="2";
local SENSOR_TYPE="3";
local SWITCH_TYPE="4";
local ALERT_TYPE="5";
local METER_TYPE="10";



conn = ubus.connect();
if not conn then
	error("Failed to connect to ubusd");
end


function getdevices(type)

	local result = conn:call("we26n_zigbee_febee", "getdevicescmd", { devicetype = type});
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end
		return result[type];

	else
		print("getdevices return nil.");	
	end
end


function getflowerdevices(type)

	local result = conn:call("we26n_flowmeter", "getdevicescmd", { devicetype = type});
	if result ~= nil then
		for k, v in pairs(result) do
			print("key=" .. k .. " value=" .. tostring(v));
		end

	else
		print("getdevices return nil.");	
	end
	
	return result;
end



function dispatchCommand(device_type,data)
	
	local cjson = require "cjson";
	local jobj = cjson.decode(data);
	local name="/tmp/devices_" .. device_type .. ".ini"; 
	local file = io.open(name,"w");
	file:write(tostring(data));
	file:close();

	for idx, value in ipairs(jobj) do
		if value ~= nil then
			local devId = value["deviceid"]
			if devId ~= nil then
				local status = value["status"];
				print("deviceid=" .. devId .. " status=" .. status);
			end
		end
	end

end


function dispatchElecWaterCommand(device_type,data)
	
	local cjson = require "cjson";
	local jobj = cjson.encode(data);
	local name="/tmp/devices_" .. device_type .. ".ini"; 
	local file = io.open(name,"w");
	print(tostring(jobj));
	file:write(tostring(jobj));
	file:close();

--[[	for idx, value in ipairs(jobj) do
		if value ~= nil then
			local devId = value["deviceid"]
			if devId ~= nil then
				local status = value["status"];
				print("deviceid=" .. devId .. " status=" .. status);
			end
		end
	end
]]

end

function processCommand(device_type)

	local result = getdevices(device_type);
	print("devicetype=" .. device_type ..",result=" .. tostring(result));
	if result ~= nil then
		if not pcall(dispatchCommand, device_type,result) then
			print("dispatch command error....,devicetype=" .. device_type);
		end
	else
		print("get command device states error....,devicetype=" .. device_type);
	end	
end


function processElecWater(device_type)
	local result = getflowerdevices(device_type);
	print("devicetype=" .. device_type ..",result=" .. tostring(result));
	if result ~= nil then
		if not pcall(dispatchElecWaterCommand, device_type,result) then
			print("dispatch command error....,devicetype=" .. device_type);
		end
	else
		print("get command device states error....,devicetype=" .. device_type);
	end	


end

local processing = false;
local t = os.time();

socket = require("socket");

while true do
	local time = os.time();
	if time - t >= 1 and not processing then
		processing = true;

		print("11111111111111:");
	do
		--[{"deviceid":"zigbee_fbee_05402d04004b1200_15","status":"1"},{"deviceid":"zigbee_fbee_05402d04004b1200_14","status":"1"}]
	

	processCommand(LAMP_TYPE);
	socket.sleep(2);
	processCommand(WINDOWS_TYPE);
	socket.sleep(2);
	processCommand(SENSOR_TYPE);
	socket.sleep(2);
	processCommand(SWITCH_TYPE);
	socket.sleep(2);
	processCommand(ALERT_TYPE);
	socket.sleep(2);
	processElecWater(METER_TYPE);

	end
		processing = false;
		t = time;

		socket.sleep(13);
	end
end



--[[	
	local total="{";
	total=tostring(total)..tostring("\"2\"")..":"..tostring(result).."}";
	print("total=" .. tostring(total));
]]

--[[			local file = io.open("/tmp/devices.ini","r");
			assert(file);
			local datad = file:read("*a"); -- 读取所有内容
			file:close();
			print("read data:"..tostring(datad));
]]

