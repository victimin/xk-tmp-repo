#!/bin/python3
import sys

OnOff = sys.argv[1]

f = open('/var/log/xk/resLED_off.csv', 'w', encoding='utf-8', newline='')
f.write(OnOff + '\n1')
f.close()  

if(int(OnOff) == 1):
    print('ON!')
elif(int(OnOff) == 0):
    print('OFF!')