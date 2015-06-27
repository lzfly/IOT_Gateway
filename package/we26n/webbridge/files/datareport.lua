#!/usr/bin/env lua

require "ubus"
require "uloop"

uloop.init();

local conn = ubus.connect();
if not conn then
	error("Failed to connect tot ubus.");
end

http = require "socket.http";

local customMethod = {
	jianyou = {
		report = {
			function(req, msg)
				local queryParam = "gatewayid=";
				
				local gatewayId = msg["gatewayid"];
				if gatewayId ~= nil then
					queryParam = queryParam .. gatewayId;
				end

				queryParam = queryParam .. "&deviceid=";
				local deviceId = msg["deviceid"];
				if deviceId ~= nil then
					queryParam = queryParam .. deviceId;
				end
				
				queryParam = queryParam .. "&devicetype=";
				local deviceType = msg["devicetype"];
				if deviceType ~= nil then
					queryParam = queryParam .. deviceType;
				end

				queryParam = queryParam .. "&attr=";
				local attr = msg["attr"];
				if attr ~= nil then
					queryParam = queryParam .. attr;
				end

				queryParam = queryParam .. "&data=";
				local data = msg["data"];
				if data ~= nil then
					queryParam = queryParam .. data;
				end

				if queryParam ~= nil then
					local targetUrl = "http://101.200.1.101:8001/helloapp/info?" .. queryParam;

					print(targetUrl);

					for i=1,3 do
						resp, code = http.request(targetUrl);
						if code == 200 then
							conn:reply(req, {code="S00000", message="" .. resp});
							return;
						end
					end

					conn:reply(req, {code="E00002", message="" .. code});
				else
					conn:reply(req, {code="E00001", message="Parameters 'gatewayid、deviceid、devicetype、attr、data' can not be empty."});
				end
			end, {gatewayid = ubus.STRING, deviceid = ubus.STRING, devicetype = ubus.STRING, attr = ubus.STRING, data = ubus.STRING}
		}
	}
}
conn:add(customMethod);

uloop.run();
