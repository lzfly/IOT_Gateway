#!/usr/bin/env lua

require "ubus"
require "nixio"

conn = ubus.connect( nil, 3000 );
ret = conn:call( "we26n_rfmodbus", "startpair", {} );
print( ret );
print( "result = " .. ret.result );

