#!/bin/sh /etc/rc.common

START=100

start_service() {
    start-stop-daemon -b -S -x datareport.lua
    start-stop-daemon -b -S -x commanddispatcher.lua
}

