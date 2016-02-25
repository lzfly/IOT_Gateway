#!/usr/bin/env lua

require "uci"
require "ubus"
require "uloop"
cjson = require "cjson"
http = require "socket.http";


local conn = nil;
local updatetmr = nil;


local macReader = io.popen("eth_mac r wifi");
local macAddr = macReader:read("*all");
macAddr = string.gsub(macAddr, ":", "");
macAddr = string.gsub(macAddr,"%\r","")
macAddr = string.gsub(macAddr,"%\n","")
macAddr = "we26n_" .. macAddr;
macReader:close();


http = require "socket.http";

function  get_web_url()

	local  x;
	local  ssip;
	local  port;
	
	x = uci.cursor();
    ssip = x:get( "jyconfig.@webserver[0].ip" );
    port = x:get( "jyconfig.@webserver[0].port" );
    
    return "http://" .. ssip .. ":" .. port .. "/v1";
end



local  web_url = get_web_url();
print( web_url );
print( macAddr );



function  update_ifttt()

	local response_body = {}  
	local targetURL = web_url .. "/logicentitys?query=gateway_sn:eq:" .. macAddr .. "&limit=20";
	local queryParam = "";
	local retable;
	
	res, code = http.request {
	    url = targetURL,  
	    method = "GET",
	    headers = 
	    {
	        ["Content-Type"] = "application/x-www-form-urlencoded",  
	        ["Content-Length"] = #queryParam,  
	    },
	    source = ltn12.source.string(queryParam),
	    sink = ltn12.sink.table(response_body)
	}
	
	if code ~= 200 then
		print( "http fail : " .. code );
		return;
	end
	
	
	retable = cjson.decode( table.concat( response_body ) );

	conn:call( "we26n_mtbridge", "clr_ifttt", {} );
	
	for  _, ifttt in ipairs(retable.logicentitys) do
	
--		if ifttt.username == "18500197967" then
			code = {};
			
			code[1] = "while true do"
			code[2] = "value = datac.wait_change( \"" .. ifttt.if_device_sn .. "\", " ..  ifttt.if_attr_code .. " ); ";
			
			if ifttt.operate_code == "=" then
				ifttt.operate_code = "==";
			elseif ifttt.operate_code == "²»µÈÓÚ" then
				ifttt.operate_code = "~=";
			end
			
			code[3] = "if value " .. ifttt.operate_code .. " " .. ifttt.if_attr_value .. " then";
			code[4] = "datac.set_attr( \"" .. ifttt.th_device_sn .. "\", " ..  ifttt.th_attr_code .. ", " .. ifttt.th_attr_value .. " ); ";
			code[5] = "end";
			code[6] = "end";
			
			result = table.concat(code, "\n");
			
			print( "ifttt.id: " .. ifttt.id );
			print( result );
			print();
			
			conn:call( "we26n_mtbridge", "add_ifttt", { name= string.format( "key%d", ifttt.id ), code=result } );
		
--		end
	end
end


function  retry_update_ifttt()
	print( "retry update ifttt" );
	update_ifttt();
	updatetmr = nil;
end


local customMethod = {
	we26n_updateifttt = {
		update = {
			function(req, msg)
				if updatetmr == nil then
                	updatetmr = uloop.timer( retry_update_ifttt, 5 );
                end
                
			    conn:reply( req, {code="S00000"} );
			    
			end, {}
		}
	}
}



uloop.init();

conn = ubus.connect();
if not conn then
	error("Failed to connect tot ubus.");
end

conn:add( customMethod );
updatetmr = uloop.timer( retry_update_ifttt, 5 );

uloop.run();



