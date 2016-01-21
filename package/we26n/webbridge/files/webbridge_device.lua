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
        url = "http://" .. ip .. ":" .. port .. "/v1";
    end
    print(url)

end
getWebServerURL()

local customMethod = {
	webbridge_device = {
		add_device = {
			function(req, msg)
			    
				local queryParam = "{\"gateway_sn\":" .. "\"" .. macAddr .. "\"";
				
				queryParam = queryParam .. ",\"device_sn\":";
				local deviceId = msg["deviceid"];
				if deviceId ~= nil then
					queryParam = queryParam  ..  "\"" .. deviceId .. "\"";
				end
			    
			    
				queryParam = queryParam .. ",\"type_code\":";
				local deviceType = msg["devicetype"];
				if deviceType ~= nil then
					queryParam = queryParam  .. deviceType;
				end
                queryParam = queryParam  .. "}";
                               
			    local targetURL = url .. "/devices"

				print(queryParam);
                print(targetURL);

				for i=1,3 do
					--resp, code = http.request(targetUrl);



                                        local response_body = {}  
                                        --local post_data = '{"device_sn":"ttttt", "attr_code":666, "attr_value":567, "source":"user", "gateway_sn":"ffffff"}';  
                                        res, code = http.request{  
                                            url = targetURL,  
                                            method = "POST",  
                                            headers =   
                                            {  
                                                ["Content-Type"] = "application/x-www-form-urlencoded",  
                                                ["Content-Length"] = #queryParam,  
                                            },  
                                            source = ltn12.source.string(queryParam),  
                                            sink = ltn12.sink.table(response_body)  
                                        }  


					if code == 200 then
						conn:reply(req, {code="S00000", message=""});
						return;
					end
				end

				conn:reply(req, {code="E00002", message="" .. code});
				
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING}
		},
		del_device = {
		   	 function(req, msg)
				local queryParam = "";

				local deviceId = msg["deviceid"];
                               
			    local targetURL = url .. "/devices" .. "/" .. deviceId 	

				print(queryParam);
                print(targetURL);

				for i=1,3 do
					--resp, code = http.request(targetUrl);



                                        local response_body = {}  
                                        --local post_data = '{"device_sn":"ttttt", "attr_code":666, "attr_value":567, "source":"user", "gateway_sn":"ffffff"}';  
                                        res, code = http.request{  
                                            url = targetURL,  
                                            method = "DELETE",  
                                            headers =   
                                            {  
                                                ["Content-Type"] = "application/x-www-form-urlencoded",  
                                                ["Content-Length"] = #queryParam,  
                                            },  
                                            source = ltn12.source.string(queryParam),  
                                            sink = ltn12.sink.table(response_body)  
                                        }  


					if code == 200 then
						conn:reply(req, {code="S00000", message=""});
						return;
					end
				end

				conn:reply(req, {code="E00002", message="" .. code});
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING}
		},
		add_device_attr = {
			function(req, msg)
			    
				local queryParam = "{\"gateway_sn\":" .. "\"" .. macAddr .. "\"";
				
				queryParam = queryParam .. ",\"device_sn\":";
				local deviceId = msg["deviceid"];
				if deviceId ~= nil then
					queryParam = queryParam ..  "\"" .. deviceId .. "\"";
				end
			    
			    
				queryParam = queryParam .. ",\"attr_code\":";
				local deviceAttr = msg["deviceattr"];
				if deviceAttr ~= nil then
					queryParam = queryParam  ..deviceAttr;
				end
				
				queryParam = queryParam .. ",\"attr_permission\":";
				local attr_permission = msg["attrpermission"];
				if attr_permission ~= nil then
					queryParam = queryParam  ..  "\"" .. attr_permission .. "\"";
				end
				
                queryParam = queryParam  .. "}";
                               
			    local targetURL = url .. "/deviceattrinfos"	

				print(queryParam);
                print(targetURL);

				for i=1,3 do
					--resp, code = http.request(targetUrl);



                                        local response_body = {}  
                                        --local post_data = '{"device_sn":"ttttt", "attr_code":666, "attr_value":567, "source":"user", "gateway_sn":"ffffff"}';  
                                        res, code = http.request{  
                                            url = targetURL,  
                                            method = "POST",  
                                            headers =   
                                            {  
                                                ["Content-Type"] = "application/x-www-form-urlencoded",  
                                                ["Content-Length"] = #queryParam,  
                                            },  
                                            source = ltn12.source.string(queryParam),  
                                            sink = ltn12.sink.table(response_body)  
                                        }  


					if code == 200 then
						conn:reply(req, {code="S00000", message=""});
						return;
					end
				end

				conn:reply(req, {code="E00002", message="" .. code});
				
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING, deviceattr = ubus.STRING, attrpermission = ubus.STRING}
		},
		device_attr_report = {
			function(req, msg)
				local deviceId = msg["deviceid"];

				if deviceId == nil then
					print( "device id is NIL" );
					return;
				end

				queryParam = "{\"attr_value_cur\":";
				local attr_value = msg["data"];
				if attr_value ~= nil then
					queryParam = queryParam  ..  "\"" .. attr_value .. "\"}";
				end

                local attr_code = msg["deviceattr"];

                               
			    local targetURL = url .. "/deviceattrinfos" .. "/" .. deviceId .. ":" .. attr_code 	

				print(queryParam);
                print(targetURL);

				for i=1,3 do

                                        local response_body = {}  
                                        --local post_data = '{"device_sn":"ttttt", "attr_code":666, "attr_value":567, "source":"user", "gateway_sn":"ffffff"}';  
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
						conn:reply(req, {code="S00000", message=""});
						return;
					end
				end

				conn:reply(req, {code="E00002", message="" .. code});
			
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING, deviceattr = ubus.STRING, data = ubus.STRING}
		}
	}
}
conn:add(customMethod);

uloop.run();
