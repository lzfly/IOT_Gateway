#!/usr/bin/env lua

require "ubus"
require "luci.sys"
socket = require("socket");
require("uci")
local x = uci.cursor()
local macReader = io.popen("eth_mac r wifi");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;

http = require "socket.http";
url = "http://10.4.44.210:8001/enngateway/getuserid?gatewayid=" .. macAddr;
--url = "http://10.4.44.210:8001/enngateway/getuserid?gatewayid=we26n_78A35106F1A8";
conn = ubus.connect();
if not conn then
	error("Failed to connect to ubusd");
end

function sleep(n)
	os.execute("sleep " .. n)
end


function getCommand()
    print(url);
	resp, code = http.request(url);
	if code == 200 then
		return resp;
	end

	return nil;
end

function  write_userid_to_config(userid)
    luci.sys.exec("uci set jyconfig.@mqttclient[0].userid="..userid)
    x:commit("jyconfig")
end

function StringManipulation(data)
    local find = string.match(data, "userid=")
    if find == nil then
        print(data) 
        return 1
    end	

    data = string.gsub(data,"%[{\"userid\":","")     
    data = string.gsub(data,"}%]","")  
    print(data)
    write_userid_to_config(data)

    return 1;
   
end



function require_socket()

	local result = getCommand();
		print(result);


		if result ~= nil then
		 	if not pcall(StringManipulation, result) then
				print("StringManipulation error....");
			end
		else
			print("get command error....");
		end
        sleep(10);

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
        url = "http://" .. ip .. ":" .. port .. "/enngateway/getuserid?gatewayid=" .. macAddr;
    end
    print(url)

end


local count = 1

getWebServerURL()


while true do
    print(count)
    if(count == 1) then
	    require_socket();
	    count = 0;
	end
	print(count)
	sleep(30)
end





