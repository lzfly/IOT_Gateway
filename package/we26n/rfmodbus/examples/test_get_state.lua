#!/usr/bin/env lua

require "ubus"
require "nixio"

conn = ubus.connect( nil, 3000 );
ret = conn:call( "we26n_rfmodbus", "getstate", {} );
print( ret );
print( "result = " .. ret.result );
print( "state = " .. ret.state );
print( "ecode = " .. ret.ecode );

if ret.addr ~= nil then
  print( "taddr = " .. nixio.bin.hexlify(ret.addr) );
end

