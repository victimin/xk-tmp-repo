#!/bin/python3
# Simple test for NeoPixels on Raspberry Pi
# sudo pip3 install rpi_ws281x adafruit-circuitpython-neopixel
import sys
import time
import board
import neopixel
import math
import digitalio
# import RPi.GPIO as GPIO
import csv

################################################################################################################

import Myblescan
import threading
import bluetooth._bluetooth as bluez
import numpy as np
from socket import *

################################################################################################################

# Choose an open pin connected to the Data In of the NeoPixel strip, i.e. board.D18
# NeoPixels must be connected to D10, D12, D18 or D21 to work.
pixel_pin = board.D18
 
# The number of NeoPixels
num_pixels = 14
 
# The order of the pixel colors - RGB or GRB. Some NeoPixels have red and green reversed!
# For RGBW NeoPixels, simply change the ORDER to RGBW or GRBW.
ORDER = neopixel.GRB
 
pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=1.0, auto_write=False,
                           pixel_order=ORDER)

pixels.fill((255, 255, 255))
StartBrightness = 0.0
CheckFlag = 0
PeriodT = 1.0

OnOff = 1

f = open('/var/log/xk/resLED_off.csv', 'a', encoding='utf-8', newline='')
f.close()  

f = open('/var/log/xk/beaconScan.csv', 'a', encoding='utf-8', newline='')
f.close()  
################################################################################################################

dev_id = 0
res = [0,0]
fRes = [-1]

time.sleep(10)
try:
    sock = bluez.hci_open_dev(dev_id)
    print("ble thread started: " + str(sock))
    if(sock==-1):
        sys.exit(1)
except:
    print("error accessing bluetooth device...")
    sys.exit(1)

Myblescan.hci_le_set_scan_parameters(sock)
Myblescan.hci_enable_le_scan(sock)

PeriodDash = 5
minPeriod = 0.5

colorToggle = 0

################################################################################################################
# print('Get Ready')

# time.sleep(10)

FixedTime = time.time()
FixedTime_1s = FixedTime
StartT = FixedTime
StartT2 = FixedTime
StartT3 = FixedTime

while True:
    CurrT = time.time()
    if(CurrT - StartT > minPeriod):
        StartT = StartT + minPeriod
        if(CurrT - StartT > PeriodT):
            StartT = StartT + PeriodT

        if(PeriodT == 0.0):
            StartBrightness = 1.0
            pixels.brightness = StartBrightness
            StartT2 = CurrT

        elif(CurrT - StartT2 > PeriodT):
            StartBrightness = (StartBrightness+1)%2
            pixels.brightness = StartBrightness

            StartT2 = StartT2 + PeriodT
            if(CurrT - StartT2 > PeriodT):
                StartT2 = StartT2 + PeriodT

            colorToggle = (colorToggle + 1)%2
            # print("testsetset")

        if(OnOff == 0 and fRes is not 10 and fRes is not 11):
            StartBrightness = 0.0
            pixels.brightness = StartBrightness

        pixels.show()

        try:
            f = open('/var/log/xk/resLED.csv', 'r', encoding='utf-8')
            rdr = csv.reader(f)
            for line in rdr:

                fRes = int(line[0])

                if(fRes == 2):
                    pixels.fill((255,0,0))
                    PeriodT = minPeriod
                elif(fRes == 1):
                    pixels.fill((0,0,255))
                    PeriodT = minPeriod
                elif(fRes == 6):
                    pixels.fill((0,0,255))
                    PeriodT = minPeriod*2
                elif(fRes == 10):
                    pixels.fill((0,255,0))
                    PeriodT = 0.0
                elif(fRes == 11):
                    if(colorToggle == 0):
                        pixels.fill((255,0,0))
                        PeriodT = minPeriod
                    elif(colorToggle == 1):
                        pixels.fill((0,0,255))
                        PeriodT = minPeriod
                    StartBrightness = 0
                else:
                    pixels.fill((0,255,0))
                    PeriodT = minPeriod*2
                
                CheckFlag = 5
                break
            f.close()  
        except:
            pass
        
        CheckFlag = CheckFlag - 1
        if CheckFlag<=0:
            CheckFlag = 0
            pixels.fill((255, 255, 255))
            PeriodT = 1
        
        f = open('/var/log/xk/resLED.csv', 'w', encoding='utf-8', newline='')
        f.close()  
        
        f = open('/var/log/xk/resLED_off.csv', 'r', encoding='utf-8', newline='')
        rdr = csv.reader(f)
        for line in rdr:
            OnOff = int(line[0])
            break
        f.close()  

    # print('check1')    
    returnedList = Myblescan.parse_events(sock, 10)
    res[0] = res[0] | returnedList[0]
    res[1] = res[1] | returnedList[1]
    # print('check2')    


    if(CurrT - StartT3 > PeriodDash):
        # print('--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------')
        fp=open('/var/log/xk/beaconScan.csv','w')
        fp.write("%d, %d" %(res[0],res[1]))
        fp.write('\n')
        fp.close()

        res = [0,0]
        StartT3 = StartT3 + PeriodDash

    if(CurrT - FixedTime_1s > 5):
        FixedTime_1s = (CurrT - CurrT%1)
        print('[%d, %d] '%(res[0],res[1]) + 'Result: ' + str(fRes) + ' / Toggle: '+str(colorToggle) + ' / Period:' + str(PeriodT) + ' / t1:' + '%.1f'%(StartT - FixedTime) + ' / t2:' + '%.1f'%(StartT2 - FixedTime) + ' / t3:' + '%.1f'%(StartT3 - FixedTime) + ' / Time: ' + '%.1f/%.1f'%(time.time() - FixedTime,CurrT - FixedTime))

    # time.sleep(0.1)

print('end')
exit()