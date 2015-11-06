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
url = "http://10.4.44.210:8001/enngateway/operatelog?gatewayid=" .. macAddr .. "&loglevel=1&logmsg=online";

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
        url = "http://" .. ip .. ":" .. port .. "/enngateway/operatelog?gatewayid=" .. macAddr .. "&loglevel=1&logmsg=online";
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


   		if result ~= nil then
			print("get command ok....");
		else
			print("get command error....");
		end
		
		ttmr:set(1000*60*5);
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
	
