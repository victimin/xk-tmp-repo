#!/usr/bin/env python

import serial
import sys
from time import sleep

ser = serial.Serial(
        port='/dev/ttyAMA0',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=3
)

print sys.argv[1]

print "Serial is open: " + str(ser.isOpen())

print "Now Writing"
ser.write(sys.argv[1]+"\r")

sleep(1)

print "Did write, now read"
x = ser.readline()
print "got '" + x + "'"

sleep(1)

ser.close()

