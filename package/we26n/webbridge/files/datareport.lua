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


url = "http://123.57.206.2:8081/v1/deviceattrinfos"
gatewayId = "we26n_78A351097F2E"

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
        url = "http://" .. ip .. ":" .. port .. "/v1/deviceattrinfos";
    end
    print(url)

end


function getGateWayId()
	local mfp, mac;
	
    mfp = io.popen( "eth_mac r wifi" );	
	mac = mfp:read( "*l" );
	mfp:close();
	mac = string.gsub( mac, ":", "" );
	gatewayId = "we26n_" .. mac;
	print( gatewayId );
	
end


-- global init
getWebServerURL()
getGateWayId();



local customMethod = {
	jianyou = {
		report = {
			function(req, msg)
				local deviceId = msg["deviceid"];

				if deviceId == nil then
					print( "device id is NIL" );
					return;
				end

				--local deviceid_str = x:get("devicesid_list_ever", "@devicesid[0]", "id")
				--local deviceid_str = luci.sys.exec("uci get devicesid_list.@devicesid[0].id")
			    -- print(deviceid_str)	
			    -- for key, value in pairs(deviceid_str) do 
			    --local match = string.match(deviceid_str,deviceId)
	            --if  match == nil then
		      --      print( deviceId .. "not in devicelist" );
		       --     return;
		      --  end
                
				
				
		





				queryParam = "{\"attr_value_cur\":";
				local attr_value = msg["data"];
				if attr_value ~= nil then
					queryParam = queryParam  ..  "\"" .. attr_value .. "\"}";
				end

                                local attr_code = msg["attr"];

                               
			        local targetURL = url .. "/" .. deviceId .. ":" .. attr_code 	

				print(queryParam);
                                print(targetURL);

				for i=1,3 do
					--resp, code = http.request(targetUrl);



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
			
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING, attr = ubus.STRING, data = ubus.STRING}
		}
	}
}
conn:add(customMethod);

uloop.run();
