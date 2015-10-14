#!/usr/bin/env lua

require "uci"
require "uloop"
require "ubus"
require "nixio"
cjson = require "cjson"


function  cvt_short( str, idx )
	local  aa,bb = string.byte( str, idx, idx+2 );
	local  ret;
	
	ret = nixio.bit.bor( nixio.bit.lshift(aa, 8), bb );
    return ret;
	
end


function  cvt_longp( str, idx )
	local  aa,bb,cc,dd = string.byte( str, idx, idx+4 );
	local  ret;
	
	ret = nixio.bit.bor( nixio.bit.lshift(aa, 8), bb );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), cc );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), dd );
    
	return ret;
end


function  cvt_longw( str, idx )
	local  cc,dd,aa,bb = string.byte( str, idx, idx+4 );
	local  ret;
	
	ret = nixio.bit.bor( nixio.bit.lshift(aa, 8), bb );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), cc );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), dd );
    
	return ret;
end


function  cvt_long( str, idx )
	local  dd,cc,bb,aa = string.byte( str, idx, idx+4 );
	local  ret;
	
	ret = nixio.bit.bor( nixio.bit.lshift(aa, 8), bb );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), cc );
	ret = nixio.bit.bor( nixio.bit.lshift(ret, 8), dd );
	
	return ret;
end



function  cvt_singleprecision( aa,bb,cc,dd )

	local  exp, frac;
    
	exp = nixio.bit.band( aa, 0x7F );
	exp = nixio.bit.lshift( exp, 1 );
	if bb >= 0x80 then
	    exp = exp + 1;
	end
	
	frac = nixio.bit.band( bb, 0x7F );
	frac = nixio.bit.bor( nixio.bit.lshift(frac, 8), cc );
	frac = nixio.bit.bor( nixio.bit.lshift(frac, 8), dd );

    if exp == 0xff then
	    
		if aa < 0x80 then
		    return math.huge;
		else
		    return -math.huge;
		end
		
	elseif exp == 0 then
	    exp = -126;
	else
	    frac = nixio.bit.bor( 0x80, 0x800000 );
	    exp = exp - 127 - 23;	
	end
    
    	
	if  aa < 0x80 then
		return math.ldexp( frac, exp );
	else
		return -math.ldexp( frac, exp );
	end

end

function  cvt_floatw( str, idx )

	local  cc,dd,aa,bb = string.byte( str, idx, idx+4 );
    return cvt_singleprecision( aa, bb, cc, dd );
	
end



function  cvt_float( str, idx )

	local  dd,cc,bb,aa = string.byte( str, idx, idx+4 );
    return cvt_singleprecision( aa, bb, cc, dd );
	
end



function  rf_get_state( conn )
	local ret;
	
	ret = conn:call( "we26n_rfmodbus", "getstate", {} );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end	
	
	return 0, ret.state;
	
end



function  rf_power_getval( conn, tbus, taddr )
	local  aaa, bbb;
	local  ret;
	
	ret = conn:call( "we26n_rfmodbus", "readreg", { bus=tbus, addr=taddr, reg=0x14, num=0x2} );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end	
	
	
    print( "hex: " .. nixio.bin.hexlify(ret.data) );
	aaa = cvt_longp( ret.data, 1 );
	bbb = (18000000/(250*60));
	return 0,aaa/bbb;
	
end


function  rf_water_getval( conn, tbus, taddr )
	local  aaa, bbb;
	local  ret;
	
	ret = conn:call( "we26n_rfmodbus", "readreg", { bus=tbus, addr=taddr, reg=0x1E, num=0x4} );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end	
    
-- print( "hex: " .. nixio.bin.hexlify(ret.data) );
	aaa = cvt_longw( ret.data, 1 );
	bbb = cvt_floatw( ret.data, 5 );
	return 0,aaa+bbb;
	
end

local  dbgrec = {};
local  prev = 0;

function  dbg_add_hexbuf( idx, val, buf )
	
	dbgrec[idx] = {};
	dbgrec[idx].val = val;
	dbgrec[idx].buf = buf;
	
	if idx == 3 then
        
		local  temp;
		
		temp = val - prev;
		
		if temp > 100 or temp < 0 then
			
    		local f = io.open( "/root/heat.log", "a" );
			str = cjson.encode( dbgrec );
			f:write( str );
			f:close();
			
		end
		
		prev = val;
		
	end
end



function  rf_heat_getval( conn, tbus, taddr )
	
	local  aaa, bbb;
	local  ret;
	
	ret = conn:call( "we26n_rfmodbus", "readreg", { bus=tbus, addr=taddr, reg=0x1C, num=0x4} );
	if ret == nil then
		return 1;
	end
	
	if ret.result ~= 0 then
		return 2;
	end
    
-- print( "hex: " .. nixio.bin.hexlify(ret.data) );
	aaa = cvt_longw( ret.data, 1 );
	bbb = cvt_floatw( ret.data, 5 );
	aaa = aaa + bbb;
	dbg_add_hexbuf( 1, aaa, nixio.bin.hexlify(ret.data) );
	
	ret = conn:call( "we26n_rfmodbus", "readreg", { bus=tbus, addr=taddr, reg=0x59F, num=0x1} );
	if ret == nil then
		return 3;
	end
	
	if ret.result ~= 0 then
		return 4;
	end
    
-- print( "hex: " .. nixio.bin.hexlify(ret.data) );
	bbb = cvt_short( ret.data, 0 );
	dbg_add_hexbuf( 2, bbb, nixio.bin.hexlify(ret.data) );
	
	bbb = math.pow( 10, bbb-4 );
	aaa = aaa * bbb;
	
	dbg_add_hexbuf( 3, aaa, "" );
	return 0,aaa;
	
end


local  conn = nil;
local  rtrytmr = nil;
local  readtmr = nil;
local  mtrec = {};



function  meter_record_getintv()
	local  x;
	local  ret;
	local  temp;
	
	mtrec.power.interval = 1;
	mtrec.water.interval = 1;
	mtrec.heat.interval = 1;
	
	
	x = uci.cursor();
    ret = x:get( "ennconfig.@470m[0].power_interval" );
	if nil ~= ret then
		temp = tonumber(ret);
		if temp > 1 then
	        mtrec.power.interval = temp;
		end
	end
	
    ret = x:get( "ennconfig.@470m[0].water_interval" );
	if nil ~= ret then
		temp = tonumber(ret);
		if temp > 1 then
	        mtrec.water.interval = temp;
		end
	end
		
    ret = x:get( "ennconfig.@470m[0].heat_interval" );
	if nil ~= ret then
		temp = tonumber(ret);
		if temp > 1 then
	        mtrec.heat.interval = temp;
		end
	end
	
	mtrec.power.interval = mtrec.power.interval * 60;
	mtrec.water.interval = mtrec.water.interval * 60;
	mtrec.heat.interval = mtrec.heat.interval * 60;
	
end



function  meter_record_report( meter )
    local  temp;	
	local  rpt = {};
	
-- print( "report begin, " .. meter );
    
    temp = os.time();
-- print( "report clock, " .. temp );
	
    if mtrec[meter].tstamp == nil or (temp - mtrec[meter].tstamp) >= mtrec[meter].interval then
        mtrec[meter].tstamp = temp;
        
		if not mtrec[meter].valid then
			return;
		end
			
		rpt.gatewayid = "xxxx";
		rpt.deviceid = mtrec[meter].deviceid;
		rpt.devicetype = mtrec[meter].devicetype;
		rpt.attr = "1004";
		rpt.data = string.format( "%f", mtrec[meter].data );
		
		conn:call( "jianyou", "report", rpt );
		
		print( ":::report, " .. meter .. "  " .. temp );
	end
	
end


function  meter_record_flush()

	local  f = io.open( "/tmp/devices_7.ini", "w" );
	local  ttt = {};
	local  str;	
	
	if nil == f then
		return;
	end
	
	ttt[1] = mtrec.power;
	ttt[2] = mtrec.water;
	ttt[3] = mtrec.heat;
	
	str = cjson.encode( ttt );
	f:write( str );
	f:close();
    
end


-- 换表需要重新启动.
function  meter_record_update( meter, status, value )
    local  temp;
	
    if nil == mtrec[meter] then
	    mtrec[meter] = {};
	end
	
	if mtrec[meter].valid ~= nil and mtrec[meter].valid then
	    -- check diff
		temp = value - mtrec[meter].data;
		if temp < -1 or temp > 10000 then
			return;
		end
	end
	
	mtrec[meter].status = status;
	
	if status ~= 0 then
		mtrec[meter].valid = true;
	    mtrec[meter].data = value;
	end
	
end


function  meter_record_init()
	local x;
	local fp = io.popen( "eth_mac r wifi" );
	local tmac = fp:read( "*l" );
	fp:close();
	
	tmac = string.gsub( tmac, ":", "" );
	
	mtrec.power = {};
	mtrec.power.deviceid = "rf470_power_" .. tmac .. "_07";
	mtrec.power.devicetype = "0007";

	mtrec.water = {};
	mtrec.water.deviceid = "rf470_water_" .. tmac .. "_08";
	mtrec.water.devicetype = "0008";

	mtrec.heat = {};
	mtrec.heat.deviceid = "rf470_heat_" .. tmac .. "_21";
	mtrec.heat.devicetype = "0021";
	
	
	x = uci.cursor();
    ret = x:get( "jyconfig.@deviceid[0].powermeter" );
	if nil ~= ret then
		mtrec.power.deviceid = "rf470_power_" .. ret .. "_07";
	end
	
    ret = x:get( "jyconfig.@deviceid[0].watermeter" );
	if nil ~= ret then
		mtrec.water.deviceid = "rf470_water_" .. ret .. "_08";
	end
		
    ret = x:get( "jyconfig.@deviceid[0].heatmeter" );
	if nil ~= ret then
		mtrec.heat.deviceid = "rf470_heat_" .. ret .. "_21";
	end
	
end


function  meter_record_clear()
	
	mtrec.power.valid = false;
	mtrec.power.status = 0;
	mtrec.power.data = 0;
	
	mtrec.water.valid = false;
	mtrec.water.status = 0;
	mtrec.water.data = 0;
	
	mtrec.heat.valid = false;
	mtrec.heat.status = 0;
	mtrec.heat.data = 0;
	
--write file
    
end




function  period_read_meter()

	ret,val = rf_power_getval( conn, 1, 1 );
	print( "power ret: " .. ret );
	if 0 ~= ret then
		meter_record_update( "power", 0, 0 );
    else
	    meter_record_update( "power", 1, val );
		meter_record_report( "power" );
	end
	
	--print( "val: " .. val );

	ret,val = rf_water_getval( conn, 1, 2 );
	print( "water ret: " .. ret );
	if 0 ~= ret then
		meter_record_update( "water", 0, 0 );
    else
	    meter_record_update( "water", 1, val );
		meter_record_report( "water" );
    end

	ret,val = rf_heat_getval( conn, 1, 3 );
	print( "heat ret: " .. ret );
	if 0 ~= ret then
		meter_record_update( "heat", 0, 0 );
    else
	    meter_record_update( "heat", 1, val );
		meter_record_report( "heat" );
    end
    
	meter_record_flush();
	readtmr:set( 10000 );
	
end


function  retry_get_state()
	ret,state = rf_get_state( conn );
	if ret == 0 and state == 4 then
        
		rtrytmr:cancel();
		rtrytmr = nil;
		
		meter_record_getintv();
		readtmr = uloop.timer( period_read_meter, 1000 );
    else
	    rtrytmr:set( 500 );
	end
end



local customMethod = {
	we26n_rfreader = {
		change_peer = {
			function(req, msg)

			    if readtmr ~= nil then
                    readtmr:cancel();
		    readtmr = nil;
                end
				
                nixio.fs.unlink( "/tmp/devices_7.ini" );
                rtrytmr = uloop.timer( retry_get_state, 500 );
			    conn:reply( req, {code="S00000"} );
			end, {}
		}
	}
}



meter_record_init();
meter_record_getintv();

print( "power interval : " .. mtrec.power.interval );
print( "heat interval : " .. mtrec.water.interval );
print( "heat interval : " .. mtrec.heat.interval );


uloop.init();
conn = ubus.connect( nil, 3000 );
if not conn then
	error( "Failed to connect to ubusd" );
end

conn:add( customMethod );
rtrytmr = uloop.timer( retry_get_state, 500 );

uloop.run();



