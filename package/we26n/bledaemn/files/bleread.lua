#!/usr/bin/env lua

require "uci"
require "uloop"
require "ubus"
require "nixio"
cjson = require "cjson"



function  cvt_long( str, idx )
	local  dd,cc,bb,aa = string.byte( str, idx, idx+4 );
	local  ret;
	
	ret = nixio.bit.bor( nixio.bit.lshift(aa, 8), bb );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), cc );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), dd );
	
	return ret;
end



function  cvt_float( str, idx )

	local  dd,cc,bb,aa = string.byte( str, idx, idx+4 );
	local  exp, frac;
	
	exp = nixio.bit.band( aa, 0x7F );
	exp = nixio.bit.lshift( exp, 1 );
	exp = nixio.bit.bor( exp, nixio.bit.rshift(bb, 7) );
	exp = exp - 127 - 23;
	
	frac = nixio.bit.bor( 0x80, bb );
	frac = nixio.bit.bor( nixio.bit.lshift(frac, 8), cc );
	frac = nixio.bit.bor( nixio.bit.lshift(frac, 8), dd );
	
	if  aa < 0x80 then
		return math.ldexp( frac, exp );
	else
		return -math.ldexp( frac, exp );
	end

end


function  test_getstate( conn )
	local ret;
	
	ret = conn:call( "we26n_bledaemn", "getstate", {} );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end	
	
	return 0, ret.state;
	
end


function  test_disconnect( conn )
	conn:call( "we26n_bledaemn", "disconnect", {} );
end


function  test_connect( conn, tmac )
	
	local  state;
	local  ret;
	
	ret, state = test_getstate( conn );
	if ret ~= 0 then
		return 1;
	end
	
	if state ~= 5 then
		test_disconnect( conn );
	end
	
	ret = conn:call( "we26n_bledaemn", "connect", { mac=tmac } );
	if ret == nil then
		return 2;
	end
	
	if ret.result ~= 0 then
		return 3;
	end
	
	return 0;
	
end


function  test_getaddr( conn )
	local  ret;
	
	ret = conn:call( "we26n_bledaemn", "exchage", { addr = 0, cmd = 0x22fd } );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end
	
	return 0, string.byte( ret.data, 1 );
	
end


function  test_getdata( conn, addr )
	local  ret;
	
	ret = conn:call( "we26n_bledaemn", "exchage", { addr = addr, cmd = 0x22ec } );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end
	
	
-- print( "data:" .. nixio.bin.hexlify(ret.data) );
	
	aa = cvt_long( ret.data, 7 );
	bb = cvt_float( ret.data, 11 );
	aa = aa + bb;
		
	return 0,aa;
	
end


function  ble_gasmeter_getinterval()
	local  x;
	local  ret;
	x = uci.cursor();
    ret = x:get( "ennconfig.@ble[0].gas_interval" );
	if nil == ret then
		return 3;
	else
	    return tonumber(ret);
	end
end



function  ble_gasmeter_getmac()
	local  x;
	local  ret;
	x = uci.cursor();
        ret = x:get( "jyconfig.@deviceid[0].blegasmeter" );
        x:__gc();
	if nil == ret then
		return nil;
	else
	    return nixio.bin.unhexlify(ret);
	end
end


function  ble_gasmeter_read( tmac )

	conn = ubus.connect( nil, 6000 );
	if conn == nil then
		conn:close();
		return 10;
	end
	
	ret,state = test_connect( conn, tmac );
	if ret ~= 0 then
		conn:close();
		return 20 + ret;
	end


	ret,addr = test_getaddr( conn );
	if ret ~= 0 then
		conn:close();
		return 30 + ret;
	end

    
	ret,data = test_getdata( conn, addr );
	if ret ~= 0 then
		conn:close();
		return 40 + ret;
	end

	ret = test_disconnect( conn );
        conn:close();
	return 0,data;
end


function  ble_gasmeter_cvtname( tmac )
	local str;
	
    str = nixio.bin.hexlify(tmac);
	str = string.upper(str);
	str = "ble_enn_" .. str .. "_20";

	return str;
end


function  ble_gasmeter_report( tmac, data )
	local  rpt = {};
	local  conn = ubus.connect();
	
	rpt.gatewayid = "we26n_78A35106F196";
	rpt.deviceid = ble_gasmeter_cvtname(tmac);
	rpt.devicetype = "20";
	rpt.attr = "1004";
	rpt.data = string.format ( "%f", data );
	
	conn:call( "jianyou", "report", rpt );
	conn:close();
end


function  ble_gasmeter_writes( tmac, data )
	local f = io.open( "/tmp/devices_8.ini", "w" );
	local ttt = {};
	local str;	
	
	if nil == f then
		return;
	end
	
	ttt[1] = {};
	ttt[1].deviceid = ble_gasmeter_cvtname( tmac );
	ttt[1].stauts = "0";
    ttt[1].devicetype = "20";
	ttt[1].data = string.format ( "%f", data );
	
	str = cjson.encode( ttt );
	f:write( str );
	f:close();
	
end


local tmac = ble_gasmeter_getmac();
local intv = ble_gasmeter_getinterval();	
local rytmr = nil;
local rdtmr = nil;


function  period_read()
    local  ret, data;
	
	ret,data = ble_gasmeter_read( tmac );
	if 0 == ret then
		ble_gasmeter_writes( tmac, data );
		ble_gasmeter_report( tmac, data );
		print( "read : " .. string.format("%f",data) );
	else
	    print( "read fail" );
	end
	
	rdtmr:set( 5000 );
	
end


function  retry_get_mac()
	tmac = ble_gasmeter_getmac();
	if nil == tmac then
        rytmr:set( 2000 );
	else
	    rytmr = nil;
        rdtmr = uloop.timer( period_read, 100 );
	end
end


function sdfsdf()
    

   
end


local customMethod = {
	we26n_change_MAC = {
		change_ble_MAC = {
			function(req, msg)
			    if rdtmr ~= nil then
                    rdtmr:cancel();
                end
                nixio.fs.unlink("/tmp/devices_8.ini")
                rytmr = uloop.timer( retry_get_mac, 100 );
			    conn:reply(req, {code="S00000"});			
			end, {}
		
		}
	}
}



uloop.init();
conn = ubus.connect();
if not conn then
	error("Failed to connect to ubusd");
end
conn:add(customMethod);

rytmr = uloop.timer( retry_get_mac, 100 );
uloop.run();



