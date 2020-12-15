#!/bin/bash

while true; do
    pid=`ps -ef | grep "xksdk" | grep -v 'grep' | awk '{print $2}'`
    if [ -z $pid ];then
        count=1
        if [ $count -lt 10 ];then
            echo "xksdk start"
            xksdk -f &
        fi
    # else
        # echo "xksdk is already running now!"
    fi
    sleep 2
done