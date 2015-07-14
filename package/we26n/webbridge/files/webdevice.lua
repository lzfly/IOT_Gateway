#!/usr/bin/env lua

require "ubus"

require("uci")
local x = uci.cursor()
local nw = require "luci.model.network".init(x)
local io = require "io"
require "luci.fs"
require "luci.sys"
local type_array={"2"};


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
--	return result;
end





function dispatchCommand(data)
	
	local cjson = require "cjson";

print("1234");

	local jobj = cjson.decode(data);


	local file = io.open("/tmp/devices.ini","w");
	file:write(tostring(data));
	file:close();


	for idx, value in ipairs(jobj) do
		if value ~= nil then
			local devId = value["deviceid"]
			if devId ~= nil then
				local status = value["status"];
				print("deviceid=" .. devId .. " status=" .. status);



--[[				if string.match(devIdLower, "zigbee_jianyou") == "zigbee_jianyou" then
					sendToZigbeeJianyou(gatewayId, devId, attr, data);
				elseif string.match(devIdLower, "zigbee_fbee") == "zigbee_fbee" then
					sendToZigbeeFbee(atewayId, devId, attr, data);
				elseif string.match(devIdLower, "433_jianyou") == "433_jianyou" then
					sendTo433Jianyou(gatewayId, devId, attr, data);

				end
]]
			end
		end
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
		local total="{";
--		for( t in pairs(type_array)) do
			local result = getdevices("2");
			print("result=" .. tostring(result));
			total=tostring(total)..tostring("\"2\"")..":"..tostring(result).."}";
			print("total=" .. tostring(total));

			--[{"deviceid":"zigbee_fbee_05402d04004b1200_15","status":"1"},{"deviceid":"zigbee_fbee_05402d04004b1200_14","status":"1"}]

		
			if result ~= nil then
				if not pcall(dispatchCommand, result) then
					print("dispatch command error....");
				end
			else
				print("get command device states error....");
			end

			local file = io.open("/tmp/devices.ini","r");
			assert(file);
			local datad = file:read("*a"); -- 读取所有内容
			file:close();
			print("read data:"..tostring(datad));

--		end

	end
		processing = false;
		t = time;

		socket.sleep(13);
	end
end
