#!/usr/bin/python
import serial
import time
import struct
import glob
import os
import sys

import serial.tools.list_ports as port_list

from xkRadarClass import RadarC 
import xkRadarClass as xk

# import atexit
# atexit.register(xk.runXKSDK())

# xk.killXKSDK()

# print(sys.argv[0][0:sys.argv[0].find('rcvDebugData.py')])
# sys.exit()

LogFilePath = sys.argv[0][0:sys.argv[0].find('rcvDebugData.py')]

r = list()
ports = list(port_list.comports())
if(len(sys.argv)<2):
    print("Please type the 'python3 rcvDebugData.py list' or 'python3 rcvDebugData.py all'.")
    sys.exit()
    
if(sys.argv[1] == 'list'):
    cnt = 0
    for p in ports:
        print(' '+str(cnt)+' =>',p[0],p[1])
        cnt += 1
    sys.exit()

RefTime = time.time()
    
if(sys.argv[1] == 'all'):
    for p in ports:
        if("Prolific"  in p[1] or "Bossa" in p[1] or "USB-Serial" in p[1] or "XeThru" in p[1]):
            RadarC(p,RefTime,LogFilePath)

else:
    if sys.argv[1].isnumeric() == 0:
        print("It is not a number")
        sys.exit()

    p = ports[int(sys.argv[1])]

    RadarC(p,RefTime,LogFilePath)


if(len(sys.argv)>2):
    if(sys.argv[2] == 'off'):
        xk.offAll_exit(1)

    if(sys.argv[2] == 'offnf'):
        xk.offAll_exit(0)

    if(sys.argv[2] == 'on'):
        xk.onAll_exit(1)

    if(sys.argv[2] == 'onnf'):
        xk.onAll_exit(0)

xk.onAll()
time.sleep(0.1)

xk.startAll()
xk.fileManager(LogFilePath)
    