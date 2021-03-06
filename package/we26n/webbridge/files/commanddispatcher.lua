#!/usr/bin/env lua

require "ubus"
require "luci.sys"
require "uloop"

uloop.init();



local macReader = io.popen("eth_mac r wifi");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;
macAddr = string.gsub(macAddr,"%\r","")
macAddr = string.gsub(macAddr,"%\n","")


http = require "socket.http";
url = "http://123.57.206.2:8081/v1/deviceattrinfos?query=gateway_sn:eq:" .. macAddr .. ",is_control:eq:1";
url2 = "http://123.57.206.2:8081/v1/deviceattrinfos";

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
function setFlag(deviceId, attr_code)
        queryParam = "{\"is_control\":0}";

        local targetURL = url2 .. "/" .. deviceId .. ":" .. attr_code;

        print(queryParam);
        print(targetURL);

        for i=1,3 do

            local response_body = {}
            res, code = http.request{
                url = targetURL,
                method = "PUT",
                headers =
                {
                    ["Content-Type"] = "application/x-www-form-urlencoded",
                    ["Content-Length"] = #queryParam,
                },
                source = ltn12.source.string(queryParam),
                sink = ltn12.sink.table(response_body)
            }
            if code == 200 then
                return;
            end

        end
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
        local dobj = jobj.deviceattrinfos;
        --print(dobj);
	for idx, value in ipairs(dobj) do
		if value ~= nil then
			local devId = value["device_sn"]
			if devId ~= nil then
				local gatewayId = value["gateway_sn"];
				local attr = string.format("%d", value["attr_code"]);
				local data = value["attr_value_ctrl"];
				local devIdLower = string.lower(devId);
				if string.match(devIdLower, "zigbee_jianyou") == "zigbee_jianyou" then
					sendToZigbeeJianyou(gatewayId, devId, attr, data);
				elseif string.match(devIdLower, "zigbee_fbee") == "zigbee_fbee" then
					sendToZigbeeFbee(gatewayId, devId, attr, data);
				elseif string.match(devIdLower, "433_jianyou") == "433_jianyou" then
					sendTo433Jianyou(gatewayId, devId, attr, data);
				end
                                setFlag(devId, attr);
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
        url = "http://" .. ip .. ":" .. port .. "/v1/deviceattrinfos?query=gateway_sn:eq:" .. macAddr .. ",is_control:eq:1";
        url2 = "http://" .. ip .. ":" .. port .. "/v1/deviceattrinfos";

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
		
		ttmr:set(1000 * 60);
end		

ttmr = uloop.timer( commanddispather, 1000*3 );
	
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
	
