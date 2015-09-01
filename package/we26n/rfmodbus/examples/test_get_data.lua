#!/usr/bin/env lua

require "ubus"
require "nixio"

conn = ubus.connect( nil, 3000 );
ret = conn:call( "we26n_rfmodbus", "readreg", { bus=0, addr=1, reg=0, num=0 } );
print( ret );
print( "result = " .. ret.result );

if ret.data ~= nil then
  print( "data = " .. nixio.bin.hexlify(ret.data) );
end

