#!/bin/bash

check_cmd=`cat /sys/class/net/ap0/operstate`

# cat /sys/class/net/ap0/operstate

if [[ "$check_cmd" == *"down"* ]];then
    sudo /root/shell/start-ap-managed-wifi.sh &
    # echo "down"
# else
    # echo "up"
fi
