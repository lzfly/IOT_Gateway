#!/usr/bin/lua

require "luci.sys"

url = "http://10.4.44.210:8001/ennconfig";


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

function getWebServerURL()
    local ip = luci.sys.exec("uci get jyconfig.@webserver[0].ip")
    local iplen = string.len(ip)
    local port = luci.sys.exec("uci get jyconfig.@webserver[0].port")
    local portlen = string.len(port)
    ip = string.gsub(ip,"%\r","")
    ip = string.gsub(ip,"%\n","")
    port = string.gsub(port,"%\r","")
    port = string.gsub(port,"%\n","")

    print(ip)
    print(port)
    if iplen ~= 0 and portlen ~= 0 then
        url = "http://" .. ip .. ":" .. port .. "/ennconfig";
    end
    print(url)

end

getWebServerURL()

while true do

    luci.sys.exec("mv /etc/config/ennconfig /etc/config/ennconfig_back")
    local curlcmd = "curl -o /etc/config/ennconfig " .. url;
    luci.sys.exec(curlcmd)
    
    local keeplive = luci.sys.exec("uci get ennconfig.@gateway[0].keeplive")
    local keeplivelen = string.len(keeplive)
    if keeplivelen ~= 0 then
        luci.sys.exec("rm -rf /etc/config/ennconfig_back")
    else
        luci.sys.exec("mv /etc/config/ennconfig_back /etc/config/ennconfig")
    end

    sleep(86400)
end
