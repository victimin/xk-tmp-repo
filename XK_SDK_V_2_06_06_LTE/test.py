#!/usr/bin/env python

import serial
from time import sleep

ser = serial.Serial(
        port='/dev/ttyACM3',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=3
)

print "Serial is open: " + str(ser.isOpen())

print "Now Writing"
ser.write("ATI\r")

sleep(1)

print "Did write, now read"
x = ser.readline()
print "got '" + x + "'"

sleep(1)

ser.close()

