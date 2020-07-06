#!/bin/bash

pid=`ps -ef | grep "xksdk" | grep -v 'grep' | awk '{print $2}'`
if [ -z $pid ]; then
	echo $(date)
	echo "    restarted"
	sudo xksdk -f &
fi
