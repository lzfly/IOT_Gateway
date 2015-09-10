#!/usr/bin/lua

require "luci.sys"

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

while true do

    luci.sys.exec("mv /etc/config/ennconfig /etc/config/ennconfig_back")
    luci.sys.exec("curl -o /etc/config/ennconfig 10.4.44.210:8001/ennconfig")
    
    file = file_exists("/etc/config/ennconfig")
    if file then 
        luci.sys.exec("rm -rf /etc/config/ennconfig_back")
    else
        luci.sys.exec("mv /etc/config/ennconfig_back /etc/config/ennconfig")
    end

    sleep(86400)
end
