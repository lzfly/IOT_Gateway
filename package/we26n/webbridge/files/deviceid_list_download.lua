#!/usr/bin/env lua

require "ubus"
require "luci.sys"
require "uloop"
socket = require("socket");
require("uci")
local x = uci.cursor()
local macReader = io.popen("eth_mac r wifi");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;
uloop.init();
http = require "socket.http";
url = "http://10.4.44.210:8001/enngateway/getdeviceslist?gatewayid=" .. macAddr;
--url = "http://10.4.44.210:8001/enngateway/getdeviceslist?gatewayid=we26n_78A35106F1A8";
conn = ubus.connect();
if not conn then
	error("Failed to connect to ubusd");
end

function sleep(n)
	os.execute("sleep " .. n)
end

function file_exists(filename)
    local file = io.open(filename,"rb")
    if file then
        file:close()
        return file
    end
    return nil
end

function Split(szFullString, szSeparator)  
	local nFindStartIndex = 1  
	local nSplitIndex = 1  
	local nSplitArray = {}  
	while true do  
	   local nFindLastIndex = string.find(szFullString, szSeparator, nFindStartIndex)  
	   if not nFindLastIndex then  
		nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, string.len(szFullString))  
		break  
	   end  
	   nSplitArray[nSplitIndex] = string.sub(szFullString, nFindStartIndex, nFindLastIndex - 1)  
	   nFindStartIndex = nFindLastIndex + string.len(szSeparator)  
	   nSplitIndex = nSplitIndex + 1  
	end  
	return nSplitArray  
end     

function getCommand()
    print(url);
	resp, code = http.request(url);
	if code == 200 then
		return resp;
	end

	return nil;
end

function  write_id_to_config(meterstr)
    local meterid = string.gsub(meterstr,"wifi_gas_","")
    
    luci.sys.exec("uci set jyconfig.@deviceid[0].gasmeter="..meterid)
    x:commit("jyconfig")
end

function StringManipulation(data)

    local find = string.match(data, "\"deviceslist\":")
    if find == nil then
        print(data)
        return 1
    end

	
	data = string.gsub(data,"{\"deviceslist\":","")     
    data = string.gsub(data,"%[","")                     
    data = string.gsub(data,"]}","")  
    print(data)    
    local config = Split(data,",")
    local size = table.getn(config)
   -- local if_exit = x:get("test","@devices[0].id")
    --if  not if_exit then
      --  x:add("test", "@devices[0]","id","0000")
        --print("in if")
    --end
    --print(if_exit)
    if file_exists("/etc/config/devicesid_list") == nil then
    
        luci.sys.exec("touch /etc/config/devicesid_list");
        local f =io.open("/etc/config/devicesid_list","w")
        f:write("config devicesid \n")
        f:close()
    
    end
     luci.sys.exec("uci delete devicesid_list.@devicesid[0].id")
    for j = 1 ,size,1 do
    print(config[j]);
    luci.sys.exec("uci add_list devicesid_list.@devicesid[0].id=" ..config[j])
    local gas_meter_seach = string.match(config[j],"wifi_gas_")
        if gas_meter_seach then
            write_id_to_config(config[j])
        end
    end 
    x:commit("devicesid_list")
    return 1;
   
end



function require_socket()

	local result = getCommand();
		print(result);
		if result ~= nil then
            --dispatchCommand(result);
		 	if not pcall(StringManipulation, result) then
				print("StringManipulation error....");
			end
		else
			print("get command error....");
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
        url = "http://" .. ip .. ":" .. port .. "/enngateway/getdeviceslist?gatewayid=" .. macAddr;
    end
    print(url)

end

getWebServerURL();

require_socket();

local customMethod = {
	we26n_download_list = {
		 download_devicesid_list= {
				function(req, msg)
					conn:reply(req, {code="S00000", message="devicesid download ok"});	
					uloop.timer( require_socket, 1 );
				end, {}
				}	
		}
}
conn:add(customMethod);

uloop.run();		







