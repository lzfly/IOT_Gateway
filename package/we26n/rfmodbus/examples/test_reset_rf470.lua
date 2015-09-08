#!/usr/bin/env lua

require "ubus"

conn = ubus.connect( nil, 3000 );
ret = conn:call( "we26n_stm32ctrl", "reset", { target="rf433" } );
print( ret );

