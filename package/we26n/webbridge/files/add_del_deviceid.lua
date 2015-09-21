#!/usr/bin/env lua
require "luci.sys"
require "ubus"
require "uloop"

uloop.init();

local conn = ubus.connect();
if not conn then
	error("Failed to connect tot ubus.");
end

local macReader = io.popen("eth_mac r wifi");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;

macAddr = string.gsub(macAddr,"%\r","")
macAddr = string.gsub(macAddr,"%\n","")


http = require "socket.http";

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
        url = "http://" .. ip .. ":" .. port .. "/enngateway/";
    end
    print(url)

end
getWebServerURL()

local customMethod = {
	deviceid_add_del = {
		add_deviceid = {
			function(req, msg)
				local queryParam = "gatewayid=";
			    queryParam = queryParam .. macAddr;
			    
				queryParam = queryParam .. "&deviceid=";
				local deviceId = msg["deviceid"];
				if deviceId ~= nil then
					queryParam = queryParam .. deviceId;
				end
			    
			    local deviceid_str = luci.sys.exec("uci get devicesid_list.@devicesid[0].id")
			    print(deviceid_str)	
			    --for key, value in pairs(deviceid_str) do 
			    local match=string.match(deviceid_str,deviceId)
	                if  match == nil then
		                luci.sys.exec("uci add_list devicesid_list.@devicesid[0].id="..deviceId)
				        luci.sys.exec("uci commit devicesid_list")
		            else
		                print("Already in devicesid_list")
		            end
			    
				queryParam = queryParam .. "&devicetype=";
				local deviceType = msg["devicetype"];
				if deviceType ~= nil then
					queryParam = queryParam .. deviceType;
				end


				if queryParam ~= nil then
					--local targetUrl = "http://10.4.44.210:8001/enngateway/adddevice?" .. queryParam;
					local targetUrl = url .."adddevice?" .. queryParam;
					print(targetUrl);

					for i=1,3 do
						resp, code = http.request(targetUrl);
						if code == 200 then
							local result = conn:call("we26n_mtbridge","notice",{module = "G",message = "ADDDEV|"..deviceId});
							conn:reply(req, {code="S00000", message="" .. resp});
							return;
						end
					end

					conn:reply(req, {code="E00002", message="" .. code});
				else
					conn:reply(req, {code="E00001", message="Parameters 'gatewayid¡¢deviceid¡¢devicetype' can not be empty."});
				end
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING}
		},
		del_deviceid = {
		   	 function(req, msg)
				local queryParam = "gatewayid=";
			    queryParam = queryParam .. macAddr;
			    
				queryParam = queryParam .. "&deviceid=";
				local deviceId = msg["deviceid"];
				if deviceId ~= nil then
					queryParam = queryParam .. deviceId;
				end
				luci.sys.exec("uci del_list devicesid_list.@devicesid[0].id="..deviceId)
				luci.sys.exec("uci commit devicesid_list")
                --local targetUrl = "http://10.4.44.210:8001/enngateway/deldevice?";
                local targetUrl = url .."deldevice?" .. queryParam;
                print(targetUrl);
                 --queryParam = targetUrl .. queryParam;
				if targetUrl ~= nil then
					--print(queryParam);
					for i=1,3 do
						resp, code = http.request(targetUrl);
						if code == 200 then
							local result = conn:call("we26n_mtbridge","notice",{module = "G",message = "DELDEV|"..deviceId});
							conn:reply(req, {code="S00000", message="" .. resp});
							return;
						end
					end

					conn:reply(req, {code="E00002", message="" .. code});
				else
					conn:reply(req, {code="E00001", message="Parameters 'gatewayid¡¢str' can not be empty."});
				end
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING}
		}
	}
}
conn:add(customMethod);

uloop.run();
