#!/usr/bin/env lua

require "ubus"
require "luci.sys"
socket = require("socket");
require("uci")
local x = uci.cursor()
local macReader = io.popen("eth_mac r lan");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = "we26n_" .. macAddr;

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

		--result = '[{ "gatewayid":"we26n_xxxxxx", "deviceid":"zigbee_fbee_xxxxx", "attr":"001", "data":"989797897897897" },{ "gatewayid":"we26n_xxxxxx", "deviceid":"zigbee_jianyou_xxxxx", "attr":"001", "data":"989797897897897" }]';

		if result ~= nil then
            --dispatchCommand(result);
		 	if not pcall(StringManipulation, result) then
				print("StringManipulation error....");
			end
		else
			print("get command error....");
		end
        sleep(10);

end


local count = 1

while true do
    print(count)
    if(count == 1) then
	    require_socket();
	    count = 0;
	end
	print(count)
	sleep(30)
end





