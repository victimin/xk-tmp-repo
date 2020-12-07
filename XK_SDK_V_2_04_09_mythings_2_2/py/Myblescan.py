# BLE iBeaconScanner based on https://github.com/adamf/BLE/blob/master/ble-scanner.py
# JCS 06/07/14

DEBUG = True
# BLE scanner based on https://github.com/adamf/BLE/blob/master/ble-scanner.py
# BLE scanner, based on https://code.google.com/p/pybluez/source/browse/trunk/examples/advanced/inquiry-with-rssi.py

# https://github.com/pauloborges/bluez/blob/master/tools/hcitool.c for lescan
# https://kernel.googlesource.com/pub/scm/bluetooth/bluez/+/5.6/lib/hci.h for opcodes
# https://github.com/pauloborges/bluez/blob/master/lib/hci.c#L2782 for functions used by lescan

# performs a simple device inquiry, and returns a list of ble advertizements 
# discovered device

# NOTE: Python's struct.pack() will add padding bytes unless you make the endianness explicit. Little endian
# should be used for BLE. Always start a struct.pack() format string with "<"

import os
import sys
import struct
import bluetooth._bluetooth as bluez
import math

LE_META_EVENT = 0x3e
LE_PUBLIC_ADDRESS=0x00
LE_RANDOM_ADDRESS=0x01
LE_SET_SCAN_PARAMETERS_CP_SIZE=7
OGF_LE_CTL=0x08
OCF_LE_SET_SCAN_PARAMETERS=0x000B
OCF_LE_SET_SCAN_ENABLE=0x000C
OCF_LE_CREATE_CONN=0x000D

LE_ROLE_MASTER = 0x00
LE_ROLE_SLAVE = 0x01

# these are actually subevents of LE_META_EVENT
EVT_LE_CONN_COMPLETE=0x01
EVT_LE_ADVERTISING_REPORT=0x02
EVT_LE_CONN_UPDATE_COMPLETE=0x03
EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE=0x04

# Advertisment event types
ADV_IND=0x00
ADV_DIRECT_IND=0x01
ADV_SCAN_IND=0x02
ADV_NONCONN_IND=0x03
ADV_SCAN_RSP=0x04


def returnnumberpacket(pkt):
    myInteger = 0
    multiple = 256
    for c in pkt:
        myInteger +=  struct.unpack("B",c)[0] * multiple
        multiple = 1
    return myInteger 

def returnstringpacket(pkt):
    myString = "";
    for c in pkt:
        # myString +=  "%02x" %struct.unpack("B",c)[0]
        myString +=  "%02x" %c
    return myString 

def printpacket(pkt):
    for c in pkt:
        # sys.stdout.write("%02x" % struct.unpack("B",c)[0])
        sys.stdout.write("%02x" % c)

def printpacketdec(pkt):
    for c in pkt:
        sys.stdout.write("%02d" % struct.unpack("B",c)[0])

def get_packed_bdaddr(bdaddr_string):
    packable_addr = []
    addr = bdaddr_string.split(':')
    addr.reverse()
    for b in addr: 
        packable_addr.append(int(b, 16))
    return struct.pack("<BBBBBB", *packable_addr)

def packed_bdaddr_to_string(bdaddr_packed):
    return ':'.join('%02x'%i for i in struct.unpack("<BBBBBB", bdaddr_packed[::-1]))

def hci_enable_le_scan(sock):
    hci_toggle_le_scan(sock, 0x01)

def hci_disable_le_scan(sock):
    hci_toggle_le_scan(sock, 0x00)

def hci_toggle_le_scan(sock, enable):
# hci_le_set_scan_enable(dd, 0x01, filter_dup, 1000);
# memset(&scan_cp, 0, sizeof(scan_cp));
 #uint8_t         enable;
 #       uint8_t         filter_dup;
#        scan_cp.enable = enable;
#        scan_cp.filter_dup = filter_dup;
#
#        memset(&rq, 0, sizeof(rq));
#        rq.ogf = OGF_LE_CTL;
#        rq.ocf = OCF_LE_SET_SCAN_ENABLE;
#        rq.cparam = &scan_cp;
#        rq.clen = LE_SET_SCAN_ENABLE_CP_SIZE;
#        rq.rparam = &status;
#        rq.rlen = 1;

#        if (hci_send_req(dd, &rq, to) < 0)
#                return -1;
    cmd_pkt = struct.pack("<BB", enable, 0x00)
    bluez.hci_send_cmd(sock, OGF_LE_CTL, OCF_LE_SET_SCAN_ENABLE, cmd_pkt)


def hci_le_set_scan_parameters(sock):
    old_filter = sock.getsockopt( bluez.SOL_HCI, bluez.HCI_FILTER, 14)

    SCAN_RANDOM = 0x01
    OWN_TYPE = SCAN_RANDOM
    SCAN_TYPE = 0x01


    
def parse_events(sock, loop_count=100):
    old_filter = sock.getsockopt( bluez.SOL_HCI, bluez.HCI_FILTER, 14)

    # perform a device inquiry on bluetooth device #0
    # The inquiry should last 8 * 1.28 = 10.24 seconds
    # before the inquiry is performed, bluez should flush its cache of
    # previously discovered devices
    flt = bluez.hci_filter_new()
    bluez.hci_filter_all_events(flt)
    bluez.hci_filter_set_ptype(flt, bluez.HCI_EVENT_PKT)
    sock.setsockopt( bluez.SOL_HCI, bluez.HCI_FILTER, flt )
    done = False
    results = []
    myFullList = [0,0]
    for i in range(0, loop_count):
        pkt = sock.recv(512)
        Fullpkt = pkt
        ptype, event, plen = struct.unpack("BBB", pkt[:3])
        #print "--------------" 
        if event == bluez.EVT_INQUIRY_RESULT_WITH_RSSI:
            i = 0
        elif event == bluez.EVT_NUM_COMP_PKTS:
            i = 0 
        elif event == bluez.EVT_DISCONN_COMPLETE:
            i = 0 
        elif event == LE_META_EVENT:
            # print(pkt[3])
            # subevent, = struct.unpack("B", pkt[3])
            subevent = pkt[3]
            pkt = pkt[4:]
            if subevent == EVT_LE_CONN_COMPLETE:
                le_handle_connection_complete(pkt)
            elif subevent == EVT_LE_ADVERTISING_REPORT:
                #print "advertising report"
                # num_reports = struct.unpack("B", pkt[0])[0]
                num_reports = pkt[0]
                report_pkt_offset = 0
                for i in range(0, num_reports):
                #ac:23:3f:a0:1f:a2
#                Adstring = ","
                    if (DEBUG == True):
                        #print "-------------"
                        #print "\tfullpacket: ", printpacket(pkt)
                        #print "\tUDID: ", printpacket(pkt[report_pkt_offset -22: report_pkt_offset - 6])
                        #print "\tMAJOR: ", printpacket(pkt[report_pkt_offset -6: report_pkt_offset - 4])
                        #print "\tMINOR: ", printpacket(pkt[report_pkt_offset -4: report_pkt_offset - 2])
                        #print "\tMAC address: ", packed_bdaddr_to_string(pkt[report_pkt_offset + 3:report_pkt_offset + 9])
                        MACaddr = packed_bdaddr_to_string(pkt[report_pkt_offset + 3:report_pkt_offset + 9])
                        # commented out - don't know what this byte is.  It's NOT TXPower
                        # txpower, = struct.unpack("b", pkt[report_pkt_offset -2])
                        txpower = pkt[report_pkt_offset -2]
                        #print "\t(Unknown):", txpower

                        iBeaconT = returnstringpacket(Fullpkt[23:39])

                        # rssi, = struct.unpack("b", pkt[report_pkt_offset -1])
                        rssi = pkt[report_pkt_offset -1]
                        #print "\tRSSI:", rssi
                        # print "\tUDID: ", printpacket(Fullpkt[23:39])

                        # fpr=open('/home/pi/wrt/MACList.txt','r')
                        # # CheckMAC = fpr.readline()
                        # OkSign = 0
                        # checkii = 0
                        # # print "txt file:", CheckMAC
                        # print('start')
                        # print(str(fpr.readline()))
                        # for cpl in fpr:
                        #     checkii += 1
                        #     if cpl[0:17]==MACaddr[0:17]:
                        #         OkSign = checkii
                        #         # print(MACaddr[0:17])
                        #         # print "OKOK"
                        # fpr.close()
                        # if(OkSign>0 and len(Fullpkt)>35):
                        CheckMAC1 = 'ac:23:3f:58:a6:f0'
                        CheckMAC2 = 'ac:23:3f:58:a6:4c'

                        if(CheckMAC1[0:16]==MACaddr[0:16] or CheckMAC2[0:16]==MACaddr[0:16]):
                            # print ("\tMAC addr: " + MACaddr + ' => ' + returnstringpacket(Fullpkt))
                            if(CheckMAC1[0:16]==MACaddr[0:16]):
                                myFullList[0] = 1
                            else:
                                myFullList[0] = 0
                            if(CheckMAC2[0:16]==MACaddr[0:16]):
                                myFullList[1] = 1
                            else:
                                myFullList[1] = 0

                        # if(CheckMAC[0:16]==MACaddr[0:16] and len(Fullpkt)>35):
                        #     # print (len(Fullpkt))
                        #     # print ("-------------")
                        #     # print ("\tfullpacket: "+ returnstringpacket(pkt))
                        #     # print ("\tfullpacket: "+ returnstringpacket(Fullpkt))
                        #     # print ("\tServiceType: "+ str(Fullpkt[report_pkt_offset +25]))
                        #     # print ("\tUDID: "+ returnstringpacket(pkt[report_pkt_offset -22: report_pkt_offset - 6]))
                        #     print ("------------------------------------------------------------------------------")
                        #     print ("\tfullpacket: " + returnstringpacket(Fullpkt))
                        #     print ("\tServiceType: " + str(Fullpkt[report_pkt_offset +25]))
                        #     print ("\tProductType: " + str(Fullpkt[report_pkt_offset +26]))
                        #     print ("------------------------------------------------------------------------------")
                        #     # ServiceT = struct.unpack("B",Fullpkt[report_pkt_offset +25])[0]
                        #     # ProductT = struct.unpack("B",Fullpkt[report_pkt_offset +26])[0]
                        #     ServiceT = Fullpkt[report_pkt_offset +25]
                        #     ProductT = Fullpkt[report_pkt_offset +26]
                        #     # iBeaconT = struct.unpack("B",Fullpkt[22])[0]
                        #     # iBeaconT = returnstringpacket(Fullpkt[23:39])
                        #     if (ServiceT==16):
                        #         a=1
                        #         # print ("\t\thttps://%s.com" % Fullpkt[28:report_pkt_offset -1])
                        #     elif (ServiceT==00):
                        #         a=1
                        #         # print ("\t\tNS:" + returnstringpacket(Fullpkt[27:37]), "/ Inst:" + returnstringpacket(Fullpkt[report_pkt_offset-7:report_pkt_offset-1]))
                        #     elif (ServiceT==32):
                        #         a=1
                        #         # print ("\t\tTemp:%d.%d" %( Fullpkt[29] , Fullpkt[30]*100/256))
                        #     elif (iBeaconT=="e2c56db5dffb48d2b060d0f5a71096e0"):
                        #         a=1
                        #         # print ("\t\tiBeacon")
                        #     elif (ServiceT==161 and ProductT==1):
                        #         a=1
                        #         # print "\tTnH//fullpacket: ", printpacket(Fullpkt)
                        #         # print("This Service is Temp & Humi")
                        #         # print "\tTnH//fullpacket: ", printpacket(Fullpkt)
                        #         # print ("\tServiceType: " + returnstringpacket(Fullpkt[report_pkt_offset +25]))
                        #         # print ("\tProductType: " + returnstringpacket(Fullpkt[report_pkt_offset +26]))
                        #         # print "\t\tTemp:%d.%d" %( struct.unpack("B",Fullpkt[28])[0] , struct.unpack("B",Fullpkt[29])[0]*100/256)
                        #         # print "\t\tHum:%d.%d" %( struct.unpack("B",Fullpkt[30])[0] , struct.unpack("B",Fullpkt[31])[0]*100/256)
                        #     elif (ServiceT==161 and ProductT==3):
                        #         # print ("\tACC//fullpacket: " + returnstringpacket(Fullpkt))
                        #         # print "\tACC//fullpacket: ", printpacket(Fullpkt[28:30])
                        #         # asdf0 = float(struct.unpack(">h",Fullpkt[28:30])[0])/256
                        #         # asdf1 = float(struct.unpack(">h",Fullpkt[30:32])[0])/256
                        #         # asdf2 = float(struct.unpack(">h",Fullpkt[32:34])[0])/256
                        #         # asdf = asdf0**2 + asdf1**2 + asdf2**2
                        #         # asdf = math.sqrt(asdf)
                        #         # ReturnValue = [OkSign, asdf, asdf0, asdf1, asdf2]
                        #         ReturnValue = [0, 1, 2, 3, 4]
                        #         # Adstring = packed_bdaddr_to_string(pkt[report_pkt_offset + 3:report_pkt_offset + 9])
                        #         # Adstring += ",\tX:%5.2f Y:%5.2f Z:%5.2f Total: %5.2f"  %(asdf0,asdf1,asdf2,asdf)
                        #         myFullList.append(ReturnValue)
                        #         # myFullList.append(Adstring)
                        #         # print "\tAdstring=", Adstring
                        #         # print "\t\tX:%5.2f Y:%5.2f Z:%5.2f" %(asdf0,asdf1,asdf2)
                        #         # if OkSign==5:
                        #             # print "\t\tNo.%d => TT: %5.2f X:%5.2f Y:%5.2f Z:%5.2f" %(OkSign,asdf,asdf0,asdf1,asdf2)
                        #         # print "\t\tNo.%d => X:%5.2f Y:%5.2f Z:%5.2f" %(OkSign,asdf0,asdf1,asdf2)
                        #         # print "\t\tTotal: %5.2f" %(asdf)
                        #     elif (ServiceT==220 and ProductT==36):
                        #         a=1
                        #         # print ("\t Stype: 0xDC, Ptype: 0x24, fullpacket: " + returnstringpacket(Fullpkt))
                        #         # print ("\t Stype: 0xDC, Ptype: 0x24, fullpacket: 0x" + returnstringpacket(Fullpkt[0:2]) + ' ' + returnstringpacket(Fullpkt[3:9]) + ' ' + returnstringpacket(Fullpkt[9:0-1]) + ' ')
                        #     else:
                        #         a=1
                        #         # print ("------------------------------------------------------------------------------")
                        #         # print ("\tfullpacket: " + returnstringpacket(Fullpkt))
                        #         # print ("\tServiceType: " + str(Fullpkt[report_pkt_offset +25]))
                        #         # print ("\tProductType: " + str(Fullpkt[report_pkt_offset +26]))
                        #         # print ("------------------------------------------------------------------------------")
                        #         # print "\tServiceType: ", printpacket(Fullpkt[report_pkt_offset +25])
                        #         # print "\tServiceType: ", printpacket(Fullpkt[report_pkt_offset +26])
                        #         # print "\t\tHum:%d.%d" %( struct.unpack("B",Fullpkt[30])[0] , struct.unpack("B",Fullpkt[31])[0]*100/256)
                        #     # print ("\tServiceType: "+ str(Fullpkt[report_pkt_offset +25]))
                        #     # print ("\tMAJOR: "+ returnstringpacket(pkt[report_pkt_offset -6: report_pkt_offset - 4]))
                        #     # print ("\tMINOR: "+ returnstringpacket(pkt[report_pkt_offset -4: report_pkt_offset - 2]))
                        #     # print ("\tMAC address: " + packed_bdaddr_to_string(pkt[report_pkt_offset + 3:report_pkt_offset + 9]))
                        # elif (iBeaconT=="e2c56db5dffb48d2b060d0f5a71096e0"):
                        #     asdf = 1
                        #     # print (MACaddr)
                        #     # fp=open('/home/pi/wrt/MACList','a+t')
                        #     # fp.write(MACaddr)
                        #     # fp.write('\n')
                        #     # fp.close()
                        # fpr.close()
                    # build the return string
                    # Adstring = packed_bdaddr_to_string(pkt[report_pkt_offset + 3:report_pkt_offset + 9])
                    # Adstring += ","
                    # Adstring += returnstringpacket(pkt[report_pkt_offset -22: report_pkt_offset - 6]) 
                    # Adstring += ","
                    # Adstring += "%i" % returnnumberpacket(pkt[report_pkt_offset -6: report_pkt_offset - 4]) 
                    # Adstring += ","
                    # Adstring += "%i" % returnnumberpacket(pkt[report_pkt_offset -4: report_pkt_offset - 2]) 
                    # Adstring += ","
                    # Adstring += "%i" % struct.unpack("b", pkt[report_pkt_offset -2])
                    # Adstring += ","
                    # Adstring += "%i" % struct.unpack("b", pkt[report_pkt_offset -1])

                    # print "\tAdstring=", Adstring
                    # myFullList.append(Adstring)
                done = True
    sock.setsockopt( bluez.SOL_HCI, bluez.HCI_FILTER, old_filter )
    return myFullList


