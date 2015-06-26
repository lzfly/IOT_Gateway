#!/usr/bin/env lua

require "ubus"

http = require "socket.http";
url = "http://192.168.1.7:8081";

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

local processing = false;
local t = os.time();

while true do
	local time = os.time();
	if time - t >= 1 and not processing then
		processing = true;

		local result = getCommand();

		--result = '[{ "gatewayid":"we26n_xxxxxx", "deviceid":"zigbee_fbee_xxxxx", "attr":"001", "data":"989797897897897" },{ "gatewayid":"we26n_xxxxxx", "deviceid":"zigbee_jianyou_xxxxx", "attr":"001", "data":"989797897897897" }]';

		if result ~= nil then
			if not pcall(dispatchCommand, result) then
				print("dispatch command error....");
			end
		else
			print("get command error....");
		end

		processing = false;
		t = time;
	end
end