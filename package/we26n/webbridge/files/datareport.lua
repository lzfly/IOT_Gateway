#!/usr/bin/env lua
require "luci.sys"
require "ubus"
require "uloop"
local x = require("luci.model.uci").cursor()
uloop.init();

local conn = ubus.connect();
if not conn then
	error("Failed to connect tot ubus.");
end

http = require "socket.http";


url = "http://10.4.44.210:8001/enngateway/info?"

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
        url = "http://" .. ip .. ":" .. port .. "/enngateway/info?";
    end
    print(url)

end

getWebServerURL()




local customMethod = {
	jianyou = {
		report = {
			function(req, msg)
				local queryParam = "gatewayid=";
				
				local gatewayId = msg["gatewayid"];
				if gatewayId ~= nil then
					queryParam = queryParam .. gatewayId;
				end

				queryParam = queryParam .. "&deviceid=";
				local deviceId = msg["deviceid"];
				if deviceId ~= nil then
					queryParam = queryParam .. deviceId;
				end
				--local deviceid_str = x:get("devicesid_list_ever", "@devicesid[0]", "id")
				local deviceid_str = luci.sys.exec("uci get devicesid_list.@devicesid[0].id")
			    print(deviceid_str)	
			    --for key, value in pairs(deviceid_str) do 
			    local match=string.match(deviceid_str,deviceId)
	                if  match == nil then
		                print(value)
		                return;
		            end
               
				
				queryParam = queryParam .. "&devicetype=";
				local deviceType = msg["devicetype"];
				if deviceType ~= nil then
					queryParam = queryParam .. deviceType;
				end

				queryParam = queryParam .. "&attr=";
				local attr = msg["attr"];
				if attr ~= nil then
					queryParam = queryParam .. attr;
				end

				queryParam = queryParam .. "&data=";
				local data = msg["data"];
				if data ~= nil then
					queryParam = queryParam .. data;
				end

				if queryParam ~= nil then
					local targetUrl = url .. queryParam;

					print(targetUrl);

					for i=1,3 do
						resp, code = http.request(targetUrl);
						if code == 200 then
							conn:reply(req, {code="S00000", message="" .. resp});
							return;
						end
					end

					conn:reply(req, {code="E00002", message="" .. code});
				else
					conn:reply(req, {code="E00001", message="Parameters 'gatewayid、deviceid、devicetype、attr、data' can not be empty."});
				end
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING, attr = ubus.STRING, data = ubus.STRING}
		}
	}
}
conn:add(customMethod);

uloop.run();
