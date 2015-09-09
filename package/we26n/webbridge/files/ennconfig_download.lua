#!/usr/bin/lua

require "luci.sys"

function sleep(n)
	os.execute("sleep " .. n)
end

while true do
    luci.sys.exec("rm -rf /etc/config/ennconfig")
    luci.sys.exec("curl -o /etc/config/ennconfig 10.4.44.210:8001/ennconfig")
    sleep(86400)
end
