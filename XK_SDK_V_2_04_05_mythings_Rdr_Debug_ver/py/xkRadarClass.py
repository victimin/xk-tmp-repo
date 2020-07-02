import serial
import time
import struct
import glob
import os
import sys
import csv
import atexit
import serial.tools.list_ports as port_list
from datetime import datetime
from threading import Thread
import zipfile
import socket
import socket


def check_inet():
     try:
          ipadd = socket.gethostbyname('xkcorp.com')
          return 1
     except:
          return 0
def check_myts_at():
    ser = serial.Serial(
            port='/dev/ttyAMA0',
            baudrate=9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=3
    )

    ser.write("AT-U=2\t584B\x1a\r".encode('ascii'))
    
    RefT = time.time()

    time.sleep(5)

    x = ser.readline()
    while(time.time() - RefT < 10):
        x += ser.readline()
        if "\r\n0\r\n" in x.decode():
#            print("OK")
            ser.close()
            return 1
            # break

    time.sleep(1)

    ser.close()
    return 0

inetFlag = check_inet()
mytsFlag = check_myts_at()

print(mytsFlag)
print(inetFlag)

# Hour
XK_LOG_SEGLEN_H = 20/60
XK_LOG_DELPERIOD_H = 24
# XK_LOG_SEGLEN_H = 12/3600 # for Debugging
# XK_LOG_DELPERIOD_H = 24/3600

PRINT_T = 30 # sencond
LED_ACK_T = 30 # sencond

class RadarC(Thread):
    dev = []
    fileHdrList = []

    def __init__(self,p,refTime,LogFilePath):
        print(p[0],p[1])
            
        RadarPort = serial.Serial(
            port=p[0],
            baudrate=115200,
            timeout=3,
            )
        RadarPort.setDTR(1)
        self.dev.append(self)
        self.RadarPort = RadarPort
        self.refTime = refTime
        self.timeout = 6
        # self.relayData = [0]
        # self.Mode = 'none'
        portStr = getFileName(self.RadarPort.port)
        self.fileheader = LogFilePath + "Debug_Log_"+portStr+"_"
        self.fileHdrList.append(self.fileheader)

        Thread.__init__(self)
        
    def logFileInit(self):
        fileTime = time.strftime('%Y%m%d_%H%M%S',time.localtime(self.refTime))
        filename = self.fileheader + fileTime+".bin"

        self.fp = open(filename,'a+b')

    def logFileDeinit(self):
        self.fp.close()
        
    def debug_Off(self):
        self.RadarPort.baudrate = 921600
        self.sendDataByte([4028.0, 0])
        
    def debug_On(self):
        self.sendDataByte([4028.0, 1])
        self.RadarPort.baudrate = 115200
        self.sendDataByte([4028.0, 1])
        self.RadarPort.baudrate = 921600
        
    def debug_Off_flash(self):
        self.RadarPort.baudrate = 921600
        self.sendDataByte([5128.0, 0])

    def debug_On_flash(self):
        self.sendDataByte([5028.0])
        self.RadarPort.baudrate = 115200
        self.sendDataByte([5028.0])
        self.RadarPort.baudrate = 921600
        
    def sendDataByte(self,value):
        cmd_f_pack = struct.pack('f'*len(value), *value)

        self.RadarPort.write(cmd_f_pack)

    def logStart(self):
        self.daemon = True
        self.start()

    def run(self):
        self.logFileInit()
        timeOneMin = 0
        CheckMissData = 0
        
        timeForLED1 = 0
        timeForLED2 = 0

        pastSyncFlag = 1

        SyncData = self.RadarPort.read(4)
        while True:
            if 'XAKA' in str(SyncData):
                pastSyncFlag = 0
                RadarID = struct.unpack('<i',self.RadarPort.read(4))[0]
                LenSig = struct.unpack('<i',self.RadarPort.read(4))[0]
                signal = self.RadarPort.read(4*LenSig)
                sigLen = struct.pack('f',LenSig)

                curtime = struct.pack('d',time.time())
                data = curtime + sigLen + signal
                # csvwriter.writerow(signal)
                self.fp.write(data)
                
                SyncData = self.RadarPort.read(4)

                if time.time() - timeOneMin > PRINT_T:
                    timeOneMin = time.time()
                    print('[' + self.RadarPort.port + ']' + 'ID: ' + str(RadarID) + ' LEN: ' + str(LenSig) + ' MISS: ' + str(CheckMissData))

                        
                if timeForLED2==1:
                    timeForLED2 = 0
                    if(inetFlag==1 or mytsFlag==1):
                        self.sendDataByte([5242])
                        
                if time.time() - timeForLED1 > LED_ACK_T:
                    timeForLED1 = time.time()
                    timeForLED2 = 1
                    self.sendDataByte([6610])

            else:
                SyncData = SyncData[1:4] + self.RadarPort.read(1) ## too slow line??
                if(pastSyncFlag==0):
                    CheckMissData = CheckMissData + 1
                pastSyncFlag = 1


            if time.time() - self.refTime > XK_LOG_SEGLEN_H*3600:
                self.logFileDeinit()
                self.refTime = time.time()
                self.logFileInit()

def fileManager(LogFilePath='./'):
    path_target = LogFilePath
    second_elapsed = XK_LOG_DELPERIOD_H*3600
    second_elapsed_forZip = 5
    devList = RadarC.fileHdrList

    FMrefTime = time.time()
    while True:
        for f in os.listdir(path_target):
            f = os.path.join(path_target, f)
            hdrStr = f[2:-19]
            if os.path.isfile(f) and "Debug_Log_" in f and any(hdrStr in wd for wd in devList):
                timestamp_now = datetime.now().timestamp() 
                is_old = os.stat(f).st_mtime < timestamp_now - (second_elapsed - second_elapsed_forZip)
                is_old2 = os.stat(f).st_mtime < timestamp_now - (second_elapsed_forZip)

                if is_old:
                    try:
                        os.remove(f)
                        print(f, 'is deleted')
                        continue
                    except OSError: 
                        print(f, 'can not delete')
                        
                if is_old2 and ".bin" in f:
                        # print(f[:-4])
                        fileComp = zipfile.ZipFile(f[:-4] + '.zip', 'w')
                        fileComp.write(f, compress_type=zipfile.ZIP_DEFLATED)
                        fileComp.close()
                        os.remove(f)

        if time.time() - FMrefTime > PRINT_T:
            FMrefTime = time.time()
            print('----------')

        time.sleep(5)

def startAll():
    for r in RadarC.dev:
        r.logStart()

def offAll_exit(nf=0):
    offAll(nf)
    sys.exit()
    
def offAll(nf=0):
    for r in RadarC.dev:
        time.sleep(0.1)
        if(nf==0):
            r.debug_Off()
        elif(nf==1):
            r.debug_Off_flash()
        
def onAll_exit(nf=0):
    onAll(nf)
    sys.exit()

def onAll(nf=0):
    for r in RadarC.dev:
        time.sleep(0.1)
        if(nf==0):
            r.debug_On()
        elif(nf==1):
            r.debug_On_flash()

def killXKSDK():
    os.system("sudo killall -2 xksdk")

def runXKSDK():
    os.system("sudo xksdk -f")

def getFileName(portStr):
    a = portStr.find('/')
    while portStr[a+1:].find('/') != -1:
        a = portStr[a+1:].find('/') + a + 1
    portStr = portStr[a+1:]
    return portStr

