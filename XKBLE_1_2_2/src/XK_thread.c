#include <pthread.h>
#include <stdio.h>
#include "XK_thread.h"
#include "RadarCommand.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <sys/socket.h>
#include "XK_CommonDefineSet.h"
#include "mqtt.h"
#include "XK_atcmd.h"
#include "shm.h"
#include <curses.h>
#include <bluetooth/l2cap.h>

/*********************************************** 
 *  thread
************************************************/
pthread_mutex_t g_GetDataMutexLock;
pthread_mutex_t g_QueuemMutexLock;

pthread_t ScanBeacon_Thread_t;
pthread_t SystemMonitor_Thread_t;
pthread_t XK_Watch_Thread_t;
pthread_t PrintData_Thread_t;
pthread_t SendToServer_Thread_t;
pthread_t MqttSend_Thread_t;
pthread_t SendMythings_Thread_t;
pthread_t Production_Thread_t;
pthread_t Shm_Thread_t;
int threadNum=0;

extern stIBconData iBconDate[MAX_DEVICE_NUM];
xk_ble_data_t xk_Data[MAX_DEVICE_NUM];
extern char gDebugMode;
extern char gInfoMode;

void *ScanBeaconThread(int device);
void *SystemMonitorThread(void *data);
void *XK_WatchThread(void *data);
void *PrintDataThread(void *data);
void *SendToServerThread(void *data);
void *MqttSendThread(void *data);
void *SendMythingsThread(void *data);
void *ProductionThread(void *data);
void *ShareRadarDataThread(void *data);

int threadCreate[THREAD_MAX];

//buffer
int gPreAdCnt[FIND_ACC_MAX_N];

//for gateway manager
int dataIdx;
float shmDataBuffer_S2M[SHM_BUFFER_SIZE_S2M];
float shmDataBuffer_M2S[SHM_BUFFER_SIZE_M2S];

//for Behrtech Mythings
char gFlgMythsConnStatus = 0;


int XK_GetTime(struct timespec srcTime){
    float res;

    res = (float)(srcTime.tv_nsec/100000000);
    res += (srcTime.tv_sec%1000)*10;

    return res;
}

int XkWatchThreadInit(void)
{
	/* Monitoring system status */
    threadCreate[threadNum++] = pthread_create(&XK_Watch_Thread_t, NULL, (void *)XK_WatchThread, (void *)'a');
    if (pthread_detach(XK_Watch_Thread_t) != 0) 
    {
        LOG_E("pthread_detach(%d)\n", threadNum - 1);
    }
}

int XkBleThreadInit(int device)
{
    // pthread_mutex_init(&g_GetDataMutexLock, NULL);
    // pthread_mutex_init(&g_QueuemMutexLock, NULL);    

	/* Scannning beacon device */
    threadCreate[threadNum++] = pthread_create(&ScanBeacon_Thread_t, NULL, (void *)ScanBeaconThread, device);
    if (pthread_detach(ScanBeacon_Thread_t) != 0) 
    {
        LOG_E("pthread_detach(%d)\n", threadNum - 1);
    }

	/* Monitoring system status */ 
    threadCreate[threadNum++] = pthread_create(&SystemMonitor_Thread_t, NULL, (void *)SystemMonitorThread, (void *)'a');
    if (pthread_detach(SystemMonitor_Thread_t) != 0) 
    {
        LOG_E("pthread_detach(%d)\n", threadNum - 1);
    }
    
	/* Printing device data */    
    if(gDebugMode == ON){
        threadCreate[threadNum++] = pthread_create(&PrintData_Thread_t, NULL, (void *)PrintDataThread, (void *)'a');
        if (pthread_detach(PrintData_Thread_t) != 0) 
        {
            LOG_E("pthread_detach(%d)\n", threadNum - 1);
        }
    }

	/* Sending data to server */
    if(configInfo.http_onoff == 1){
        threadCreate[threadNum++] = pthread_create(&SendToServer_Thread_t, NULL, (void *)SendToServerThread, (void *)'a');
        if (pthread_detach(SendToServer_Thread_t) != 0) 
        {
            LOG_E("pthread_detach(%d)\n", threadNum - 1);
        }
    }

    /* Sending data using mqtt */
    if(configInfo.mqtt_onoff == 1){
        threadCreate[threadNum++] = pthread_create(&MqttSend_Thread_t, NULL, (void *)MqttSendThread, (void *)'a');
        if (pthread_detach(MqttSend_Thread_t) != 0) 
        {
            LOG_E("pthread_detach(%d)\n", threadNum - 1);
        }
    }

    /* Sending data using mqtt */
    if(configInfo.behr_myts_onoff == 1){
        threadCreate[threadNum++] = pthread_create(&SendMythings_Thread_t, NULL, (void *)SendMythingsThread, (void *)'a');
        if (pthread_detach(SendMythings_Thread_t) != 0) 
        {
            LOG_E("pthread_detach(%d)\n", threadNum - 1);
        }
    }
    
    /* Production */
    threadCreate[threadNum++] = pthread_create(&Production_Thread_t, NULL, (void *)ProductionThread, (void *)'a');
    if (pthread_detach(Production_Thread_t) != 0) 
    {
        LOG_E("pthread_detach(%d)\n", threadNum - 1);
    }
    
    /* shm */
    threadCreate[threadNum++] = pthread_create(&Shm_Thread_t, NULL, (void *)ShareRadarDataThread, (void *)'a');
    if (pthread_detach(Shm_Thread_t) != 0) 
    {
        LOG_E("pthread_detach(%d)\n", threadNum - 1);
    }
        
}



static int find_conn(int s, int dev_id, long arg)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;

	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		perror("Can't allocate memory");
		exit(1);
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(s, HCIGETCONNLIST, (void *) cl)) {
		perror("Can't get connection list");
		exit(1);
	}

	for (i = 0; i < cl->conn_num; i++, ci++)
		if (!bacmp((bdaddr_t *) arg, &ci->bdaddr)) {
			free(cl);
			return 1;
		}

	free(cl);
	return 0;
}


void *ScanBeaconThread(int device)
{
    int i;      
	uint8_t buf[HCI_MAX_EVENT_SIZE];

    int timerCnt, timeDiff;
    int preTimerCnt[HCI_MAX_EVENT_SIZE];
	struct tm preLocalTime[MAX_DEVICE_NUM];
    struct timespec timerCntSrc;

	evt_le_meta_event * meta_event;
	le_advertising_info * info;
	int len;
	int flgFound=0;
    int bytes_read, client;
   
    while(1){        
        clock_gettime(CLOCK_REALTIME, &timerCntSrc);
        timerCnt = XK_GetTime(timerCntSrc);

        len = read(device, buf, sizeof(buf));
		if ( len >= HCI_EVENT_HDR_SIZE ) {
            xk_findAccAll(buf);
			meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
			if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT ) {
				uint8_t reports_count = meta_event->data[0];
				void * offset = meta_event->data + 1;
				while ( reports_count-- ) {
					info = (le_advertising_info *)offset;
					char addr[18];
					ba2str(&(info->bdaddr), addr);
					// printf("%s - RSSI %d\n", addr, (char)info->data[info->length]);
                    
                    for(i=0; i<configInfo.radarDeviceNum; i++){
                        if( strcmp(configInfo.radarDeviceList[i], addr)==0 ){
                            iBconDate[i].RSSI=GetComplements((char)info->data[info->length]);

                            ParseBleData(info->data, &iBconDate[i], info->length);
                            xk_parseMsg(&iBconDate[i],&xk_Data[i]);

                            if(gPreAdCnt[i] == xk_Data[i].adCnt){
                                iBconDate[i].connectionStatus = CONN_STATUS_ERROR_2;
                            }
                            else{
                                iBconDate[i].connectionStatus = CONN_STATUS_CONNECTED;
                            }
                            iBconDate[i].cntData++;
                            gPreAdCnt[i] = xk_Data[i].adCnt;

                            preTimerCnt[i] = timerCnt;
                            LoggingBleData(addr, &iBconDate[i]);
                        }
                    }
					offset = info->data + info->length + 2;
				}
			}
		}

        for(i=0; i<configInfo.radarDeviceNum; i++)
        {
            timeDiff = (10000 + timerCnt - preTimerCnt[i]) % 10000;
            if (timeDiff > (IBCON_TIMEOUT_SEC*10)&&iBconDate[i].cntData) iBconDate[i].connectionStatus = CONN_STATUS_DISCONNECTED;
        }
        delay(10);
    }
}

void *SystemMonitorThread(void *data)
{
    long double a[4], b[4], loadavg;
    double temperature = get_temp(0);
    int memHeap = GetRamInKB();
    int cpuUsage=0;;
    CPU_Tick(a);
    delay(100);
    loadavg = CPU_Tock(a, b);

    httpHandle.memHeap = memHeap;
    httpHandle.cpuUsage = cpuUsage;
    httpHandle.piTemperature = temperature;
    
    while(1){
		CPU_Tick(a);
        
        temperature = get_temp(0);
        memHeap = GetRamInKB();
        cpuUsage = (unsigned int)(loadavg*100);
        if(gInfoMode == ON){
            printf("----------------------------------------------------\n");
            printf("+ Temperature =\t\t%3.3f'C\n", temperature);
            printf("+ CPU usage =\t\t%d %\n", cpuUsage);
            printf("+ Memory usage =\t%ld KB\n", memHeap);
            printf("----------------------------------------------------\n");
        }
        httpHandle.memHeap = memHeap;
        httpHandle.cpuUsage = cpuUsage;
        httpHandle.piTemperature = temperature;
		delay(1000);
		loadavg = CPU_Tock(a, b);
    }
}

void *XK_WatchThread(void *data){
    time_t timer;
	struct tm *localTimeTmp;
	struct tm *utcTimeTmp;
    
    int i;
    FILE *fd_log_file;   
    char aaa[100]; 
    char bbb[100]; 
    char month[12][10] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    int preMin=0;

    while(1){  
        timer = time(NULL);         
        localTimeTmp = localtime(&timer);
        utcTimeTmp = gmtime( &timer);
    
        memcpy(&gLocalTime,localTimeTmp,sizeof(struct tm));
        memcpy(&gUtcTime,utcTimeTmp,sizeof(struct tm));

        if(preMin != gUtcTime.tm_min){
            if(gUtcTime.tm_min % STATIC_WIFI_CHANNEL_PREIOD == 0) {
                system("sudo iw reg set US");
            }
        }
        preMin = gUtcTime.tm_min;

        delay(100);
    }
}

void *PrintDataThread(void *data){
    int i, j;
    int preCntData[MAX_DEVICE_NUM] = {0,};
    int cntPopupTime[MAX_DEVICE_NUM] = {0,};
    char conditionPopup = 0;

    while(1){
        printf("\n\n\n\n\n\n\n\n\n");
        xk_printDetectMAClistAll();
        for(i=0; i<configInfo.radarDeviceNum; i++){
            if(iBconDate[i].cntData != preCntData[i]){
                cntPopupTime[i] = 10;                
            }

            if (cntPopupTime[i]) printf("%s",ANSI_COLOR_YELLOW);
            printf("[%02d]   Data count (%d)   Device MAC %s   ", i, iBconDate[i].cntData, configInfo.radarDeviceList[i]);
            printf("RSSI(%d)\n  data: ", iBconDate[i].RSSI);
            // xk_parseMsg(&iBconDate[i],&xk_Data[i]);
            xk_parsePrint(&xk_Data[i]);
            // // for(j=0;j<3;j++) printf("%02x ", iBconDate[i].iBconPrefix.advFlgs[j]);
            // // for(j=0;j<2;j++) printf("%02x ", iBconDate[i].iBconPrefix.advHeader[j]);
            // for(j=0;j<2;j++) printf("%02x ", iBconDate[i].iBconPrefix.companyID[j]);
            // // for(j=0;j<1;j++) printf("%02x ", iBconDate[i].iBconPrefix.iBconType);
            // // for(j=0;j<1;j++) printf("%02x ", iBconDate[i].iBconPrefix.iBconLen);
            // for(j=0;j<16;j++) printf("%02x ", iBconDate[i].UUID[j]);
            // for(j=0;j<2;j++) printf("%02x ", iBconDate[i].majorNum[j]);
            // for(j=0;j<2;j++) printf("%02x ", iBconDate[i].minorNum[j]);
            if (cntPopupTime[i]) printf("%s",ANSI_COLOR_RESET);
            printf("\n\n");
            preCntData[i] = iBconDate[i].cntData;

            if(cntPopupTime[i]>0)cntPopupTime[i]--;
        }
        delay(100);
    }
}

int AlgorithmThreadDeinit(void) {
    pthread_mutex_destroy(&g_GetDataMutexLock);
    pthread_mutex_destroy(&g_QueuemMutexLock);

    return 0;
}

void *SendToServerThread(void *data)
{
    int res = 0;    
    int timeDiff;
    int XKtimeCheckSend = 0;
    int XKtimeCheckSend_pre = 0;    
    struct timespec sTimeOutNanoTimeSendPeriod;

    delay(3000);
    XKtimeCheckSend_pre = XK_GetTime(sTimeOutNanoTimeSendPeriod);
    while(1)
    {
        res = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod);
        XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeSendPeriod);
        timeDiff = (10000 + XKtimeCheckSend - XKtimeCheckSend_pre) % 10000;
        
        if( timeDiff > configInfo.info_send_period / 100){            
            res = SendRadarData(&httpHandle);
            XKtimeCheckSend_pre = XKtimeCheckSend;
        } 
        delay(100);
    }
}


void *MqttSendThread(void *data)
{
    int i,j;
    struct timespec sTimeOutNanoTimeVal;
    struct timespec sTimeOutNanoTimeTmp;
    struct timespec sTimeOutNanoTimeSendPeriod;
    struct timespec sTimeOutNanoTimeSendPeriod_pre;
    
    char timeStamp[100]={0,};

    int XKtimeCheckSend = 0;
    int XKtimeCheckSend_pre = 0;
    int rtn;
    int timeDiff=0;

    // char stateCMD[MAX_NUM_USB_DEVICE]={0,};
    // int sendCnt[MAX_NUM_USB_DEVICE]={0,};
    int sendCntMain=0;
    int res=1;
    
    char tmpAtPacket[MQTT_MSG_SZ];
    // char tmpMacAddr[20]={0,};
    char mqttPacket[MQTT_MSG_SZ+100];
    char destMsg[MQTT_MSG_SZ+100];
    int fAtcmd;
    char isZone = 0;
    // float rcvdData[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];

    float tmpMovement;
    int tmpStabltBR;
    int tmpStabltHR;
    
    delay(3000);
    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeVal);
    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod_pre);
    
    XKtimeCheckSend_pre = XK_GetTime(sTimeOutNanoTimeSendPeriod_pre);
    
    while(1)
    {
        rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod);
        XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeSendPeriod);

        timeDiff = (10000 + XKtimeCheckSend - XKtimeCheckSend_pre) % 10000;
        // printf("%d \n", timeDiff);

        if( timeDiff > (long long)configInfo.MqttInfo.frequency / 100 )
        {
            memset(tmpAtPacket, 0, MQTT_MSG_SZ);
            memset(mqttPacket, 0, MQTT_MSG_SZ+100);

            for(i=0; i<configInfo.radarDeviceNum; i++)
            {
                MakeMqttMsg(tmpAtPacket, &xk_Data[i], i);
            }

            struct timeval val;
            gettimeofday(&val, NULL);

            sprintf(timeStamp, "%04d-%02d-%02dT%02d:%02d:%02d.%06ldZ", 
                    gUtcTime.tm_year + 1900,
                    gUtcTime.tm_mon + 1,
                    gUtcTime.tm_mday,
                    gUtcTime.tm_hour,
                    gUtcTime.tm_min,
                    gUtcTime.tm_sec ,
                    val.tv_usec
            );

            sprintf(mqttPacket, "{\"SensorData\": \"%s\",\"Time\": \"%s\",\"Type\": \"xkgw_ble\"}", tmpAtPacket, timeStamp);

            printf("%s\n\n", mqttPacket);
            // printf("%s\n", timeStamp);

            res = MqttPublish(configInfo.MqttInfo, mqttPacket);

            XKtimeCheckSend_pre = XKtimeCheckSend;
        }
        delay(100);
    }
}

void *SendMythingsThread(void *data)
{
    int i,j;
    struct timespec sTimeOutNanoTimeVal;
    struct timespec sTimeOutNanoTimeTmp;
    struct timespec sTimeOutNanoTimeSendPeriod;
    struct timespec sTimeOutNanoTimeSendPeriod_pre;
    
    int XKtimeCheckSend = 0;
    int XKtimeCheckSend_pre = 0;
    int rtn;
    int timeDiff=0;

    // char stateCMD[MAX_NUM_USB_DEVICE]={0,};
    // int sendCnt[MAX_NUM_USB_DEVICE]={0,};
    int sendCntMain=0;
    int res=1;
    
    char tmpAtPacket[MYTHINGS_MSG_SZ];
    char tmpAtPacket_asc[MYTHINGS_MSG_SZ*2];
    char destMsg[MYTHINGS_MSG_SZ*2];

    int fAtcmd;
    char isZone = 0;
    // float rcvdData[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];

    float tmpMovement;
    int tmpStabltBR;
    int tmpStabltHR;

    int idxSep, numOfSep, numOfSep_;
    
    int errCnt=0;
    int _errCnt=0;

    delay(3000);
    XK_ATcmdInit(&fAtcmd);

    configInfo.myths_period = MYTHINGS_SEND_PERIOD;

    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeVal);
    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod_pre);
    
    XKtimeCheckSend_pre = configInfo.myths_period*(-1);
    
    while(1)
    {
        rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod);
        XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeSendPeriod);

        timeDiff = (10000 + XKtimeCheckSend - XKtimeCheckSend_pre) % 10000;
        // printf("!!%d     %d\n", timeDiff, configInfo.myths_period/100 );

        // if( timeDiff > (long long)configInfo.MqttInfo.frequency / 100 )
        if( timeDiff > (long long)configInfo.myths_period / 100 )
        {
            numOfSep = configInfo.radarDeviceNum / MYTHINGS_MSG_MAX_NUM_OF_DEV;
            numOfSep_ = configInfo.radarDeviceNum % MYTHINGS_MSG_MAX_NUM_OF_DEV;

            for(idxSep=0; idxSep<numOfSep + (numOfSep_>0); idxSep++){
                memset(tmpAtPacket, 0, MYTHINGS_MSG_SZ);
                memset(tmpAtPacket_asc, 0, MYTHINGS_MSG_SZ*2);
                memset(destMsg, 0, MYTHINGS_MSG_SZ*2);
                
                if(idxSep == numOfSep){
                    for(i=idxSep*MYTHINGS_MSG_MAX_NUM_OF_DEV; i<idxSep*MYTHINGS_MSG_MAX_NUM_OF_DEV+numOfSep_; i++)
                    {
                        MakeMqttMsg(tmpAtPacket, &xk_Data[i], i);
                    }
                }
                else{
                    for(i=idxSep*MYTHINGS_MSG_MAX_NUM_OF_DEV; i<idxSep*MYTHINGS_MSG_MAX_NUM_OF_DEV+MYTHINGS_MSG_MAX_NUM_OF_DEV; i++)
                    {
                        MakeMqttMsg(tmpAtPacket, &xk_Data[i], i);
                    }
                }

                for(i=0 ; i<strlen(tmpAtPacket) ; i++) sprintf(&tmpAtPacket_asc[i*2], "%02x", tmpAtPacket[i]);
                
                
                // printf("%s\n", tmpAtPacket);
                // printf("%d:%s\n\n", sendCntMain, tmpAtPacket_asc);

                sprintf(destMsg, AT_CMD_UNI, strlen(tmpAtPacket_asc)/2, AT_TAB, tmpAtPacket_asc, AT_EOF);

                read(fAtcmd, tmpAtPacket, MYTHINGS_MSG_SZ);
                XK_ATcmdWriteSimple(fAtcmd, destMsg);
                delay(100);
                res = XK_ATcmdRead(fAtcmd, 8);

                if(res){
                    sendCntMain++;
                    gFlgMythsConnStatus = 1;
                    errCnt = 0;
                }
                else {
                    gFlgMythsConnStatus = 0;
                    errCnt++;
                }

                if(errCnt >= 100) system("sudo reboot");
                else{
                    if(errCnt&&errCnt%5==0){
                        LOG_E("Behrtech","Module frozen");
                        digitalWrite(GPIO_UART_RST, LOW); // Turn LED ON
                        delay(1000);
                        digitalWrite(GPIO_UART_RST, HIGH); // Turn LED ON
                        LOG_I("Behrtech","Reset");
                    }
                }

            }

            XKtimeCheckSend_pre = XKtimeCheckSend;
        }
        delay(100);
    }
}

void *ProductionThread(void *data)
{
    int i;
    int tmpParam;
    int USBthreadCreate[MAX_NUM_USB_DEVICE];
    long double a[4], b[4], loadavg;
    int initialMem = GetRamInKB();
    int preTimeDay;
    int preTimeHour;
    char usbStatus[MAX_NUM_USB_DEVICE];
    int fd[MAX_NUM_USB_DEVICE];
    
    char getData[100];
    char buffCMD[100];
    char buffCMD_send[30];
    char device[50];  
    char buffCMD_info[1000]={0,}; 
    int len;
    char cntState=0;
    char usbDataType = USB_DATA_TYPE_NORMAL;
    char flgWait=1;
    char flgOpen=0;
    char cntOpen=0;

    time_t timer;
    struct tm *t;

    char gFlgWrOneTime=1;
    
    preTimeHour = gLocalTime.tm_hour;
            
    XK_UsbCmdData_t gUSB_CMD_Data;

    while(1)
    {
        cntOpen = 0;
        for (i = 0; i < MAX_NUM_USB_DEVICE; i++) usbStatus[i] = -1;
        for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
            sprintf(device, "/dev/ttyUSB%d", i);
            if( access( device, R_OK | W_OK ) == 0 ){
                UART_init(&fd[i], device, &usbStatus[i]);
                flgOpen=1;
                cntOpen++;
                len = read(fd[i], &getData[0], MAX_STR_LEN);
                
                if( getData[0]=='X' &&
                    getData[1]=='A' &&
                    getData[2]=='K' &&
                    getData[3]=='A' &&
                    getData[4]=='U' &&
                    getData[5]=='C' &&
                    getData[6]=='M' &&
                    getData[7]=='D'
                ){
                    usbDataType = USB_DATA_TYPE_COMMAND;
                    // getData.f[0] = g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum][2];   
                    buffCMD[0] = getData[8];
                    buffCMD[1] = getData[9];
                    buffCMD[2] = getData[10];
                    buffCMD[3] = getData[11];
                    buffCMD[4] = '\0';

                    if(strcmp(buffCMD, XK_USB_CMD_IS_CONNECTED)==0 && flgWait==1){
                        // printf("is connected?\n");
                        sprintf(buffCMD_send, "XAKAUCMD%s", XK_USB_CMD_STRAT);
                        if (cntState==0) write(fd[i], buffCMD_send, 12);
                        memset(buffCMD, 0, 12);
                        cntState = 1;
                    }            
                    else if(strcmp(buffCMD, XK_USB_CMD_GET_INFO)==0 && flgWait==1){
                        
                        // printf("%s", device);
                        // printf("info?\n");
                        
                        JSON_Value *confValFromFile;
                        JSON_Object *confMainObj;
                        JSON_Object *confConfObj;
                        char path_local_tmp[50]={0,};
                        char path_local[50]={0,};
                        char gwName[50]={0,};
                        char gwVersion[50]={0,};

                        XK_GetTeamviewerID(gUSB_CMD_Data.teamviewerID, "/etc/teamviewer/global.conf");
                        XK_GetTeamviewerVer(gUSB_CMD_Data.teamviewerVer, "/etc/teamviewer/global.conf");
                        XK_GetImageVersion(gwVersion);
                        
                        char tmpGwNum[10]={0,};
                        sprintf(tmpGwNum, "%c%c%c%c%c", getData[12], getData[13], getData[14], getData[15], getData[16]);
                        tmpGwNum[5]=0;


                        confValFromFile = XK_ParseFile(CONFIG_PATH);
                        confMainObj = XK_GetObjectFromVal(confValFromFile);
                        confConfObj = XK_GetLowObjFromHighObj_Sel(confMainObj, "config");
                        sprintf(gwName, DEVIENAME, tmpGwNum);
                        json_object_set_string(confConfObj, "device", gwName);


                        strcpy(path_local_tmp, XKBLE_VERSION);
                        path_local_tmp[1]='_';
                        path_local_tmp[3]='_';
                        sprintf(path_local, "/home/xk/XKBLE_%s/config.json", path_local_tmp);
                        // printf("%s\n", path_local);
                        json_serialize_to_file_pretty(confValFromFile, path_local);
                        json_serialize_to_file_pretty(confValFromFile, CONFIG_PATH);

                        json_value_free(confValFromFile);

                        if(gFlgWrOneTime){
                            SetHostapd(configInfo.MAC_w_Addr, configInfo.APmode_SSID, configInfo.APmode_PASS);
                            sprintf(buffCMD_info, "XAKAUCMD%s", XK_USB_CMD_DATA);
                            // sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", XK_USB_CMD_DATA);  
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gwName);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.function);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gwVersion);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.teamviewerID);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.teamviewerVer);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.MAC_e_Addr);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.MAC_w_Addr);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", XKBLE_VERSION);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%d", configInfo.info_send_period);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%c", configInfo.typeData);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.host);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.page);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%d", configInfo.port);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.APmode_SSID);
                            sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", configInfo.APmode_PASS);
                            if (cntState==1) write(fd[i], buffCMD_info, strlen(buffCMD_info));
                            memset(buffCMD, 0, 12);
                            gFlgWrOneTime = 0;
                            close(fd[i]);
                        }
                        cntState = 2;
                    }
                    else if(strcmp(buffCMD, XK_USB_CMD_EXIT)==0 && flgWait==1){
                        // printf("finish\n");
                        // sprintf(buffCMD_info, "XAKAUCMD%s", XK_USB_CMD_DONE);
                        // if (cntState==2) write(*XK_UARTHandle[portNum].fd, buffCMD_info, strlen(buffCMD_info));
                        flgWait = 0;
                        memset(buffCMD, 0, 12);
                    }
                }
            }
        }

        if(cntOpen == 0){
            flgWait=1;
            flgOpen=0;
            gFlgWrOneTime=1;
            cntState=0;
        }
        delay(100);
    }

    return 0;
}


void *ShareRadarDataThread(void *data)
{
    int shmID_S2M;
    void *shmAddr_S2M;

    int shmID_M2S;
    void *shmAddr_M2S;

    int count;
    
    Sendmsg_t rcvdCmdMsg;

    // Sendmsg_t ttttt;
    // Sendmsg_t cmdPortNum;
    float tmpCmdPortNum;
    int cmdPortNum;
    
    count = 0;
    int i, j;
    int numOfRadar=0;;

    int finish_cnt;

    char flgEndOfCmd= 0;
    int cntCmdSZ= 0;


    shmID_S2M = XK_ShmGet(SHM_KEY_S2M, SHM_MEM_SIZE_S2M);
    shmAddr_S2M = XK_ShmAttach(shmID_S2M);

    // printf("%d\n", shmID_S2M);
    // printf("%x\n", shmAddr_S2M);

    shmID_M2S = XK_ShmGet(SHM_KEY_M2S, SHM_MEM_SIZE_M2S);
    shmAddr_M2S = XK_ShmAttach(shmID_M2S);

    // printf("%d\n", shmID_M2S);
    // printf("%x\n", shmAddr_M2S);


    delay(100);
    memcpy(shmDataBuffer_M2S, (float *)shmAddr_M2S, SHM_MEM_SIZE_M2S);

    int threadFrameCnt=0;
    float rcvdDataCnt=shmDataBuffer_M2S[0], rcvdDataCnt_pre=shmDataBuffer_M2S[0];
    float latestCmdCnt=0;

    char test_arr[4];
    int tmpDataSize=0;

    while(1)
    {
        numOfRadar = configInfo.radarDeviceNum;
        dataIdx=0;

        shmDataBuffer_S2M[XK_SHM_IDX_S2M_NUM_OF_DEV] = numOfRadar;
        shmDataBuffer_S2M[XK_SHM_IDX_S2M_LATEST_CMD_CNT] = latestCmdCnt;

        dataIdx=XK_SHM_IDX_S2M_RESERVED_2;

        for (i = XK_SHM_IDX_S2M_RESERVED_2; i < XK_SHM_IDX_S2M_SEPARATOR; i++){
            shmDataBuffer_S2M[i] = (float)count++;
            // dataIdx++;
        }

        dataIdx=XK_SHM_IDX_S2M_SEPARATOR;

        for (i = 0; i < configInfo.radarDeviceNum; i++)
        {
            shmDataBuffer_S2M[dataIdx++] = (float)SHM_SEPARATOR_BGW;
            shmDataBuffer_S2M[dataIdx++] = (float)i;    //To clearly distinguish in the web tool

            // if(iBconDate[i].connectionStatus == CONN_STATUS_DISCONNECTED || iBconDate[i].connectionStatus == CONN_STATUS_NONE || iBconDate[i].connectionStatus == CONN_STATUS_ERROR_2 ){
            if(iBconDate[i].connectionStatus <= CONN_STATUS_NONE){
                shmDataBuffer_S2M[dataIdx++] = (float)iBconDate[i].connectionStatus;      //it has to be upgrade
                shmDataBuffer_S2M[dataIdx++] = 0;
                MacConverterS2F(&shmDataBuffer_S2M[dataIdx], configInfo.radarDeviceList[i]);
                dataIdx+=6;
                
                shmDataBuffer_S2M[dataIdx++] = 0;
                shmDataBuffer_S2M[dataIdx++] = 0;
                shmDataBuffer_S2M[dataIdx++] = 0;
                shmDataBuffer_S2M[dataIdx++] = 0;
                shmDataBuffer_S2M[dataIdx++] = 0;
                shmDataBuffer_S2M[dataIdx++] = 0;
            }
            else{
                shmDataBuffer_S2M[dataIdx++] = 49;      //it has to be upgrade
                shmDataBuffer_S2M[dataIdx++] = (float)iBconDate[i].RSSI;
                MacConverterS2F(&shmDataBuffer_S2M[dataIdx], configInfo.radarDeviceList[i]);
                dataIdx+=6;

                shmDataBuffer_S2M[dataIdx++] = xk_Data[i].pres_cm;
                shmDataBuffer_S2M[dataIdx++] = xk_Data[i].adCnt;
                shmDataBuffer_S2M[dataIdx++] = xk_Data[i].cursor;
                shmDataBuffer_S2M[dataIdx++] = (xk_Data[i].dwellTime)/3600;
                shmDataBuffer_S2M[dataIdx++] = ((xk_Data[i].dwellTime)%3600)/60;
                shmDataBuffer_S2M[dataIdx++] = (xk_Data[i].dwellTime)%60;
            }
        }
        
        // for (i = 0; i < configInfo.radarDeviceNum; i++)
        //     printf("%f ", iBconDate[i].connectionStatus);
        //     printf("\n");

        shmDataBuffer_S2M[dataIdx++] = SHM_FINISH;
        shmDataBuffer_S2M[dataIdx++] = SHM_FINISH;

        //Write Radar data to Shm
        memcpy(shmAddr_S2M, shmDataBuffer_S2M, SHM_MEM_SIZE_S2M);
        //Read Command data from Shm
        memcpy(shmDataBuffer_M2S, (float *)shmAddr_M2S, SHM_MEM_SIZE_M2S);

#if(0)
        printf("SDK -> NODE\n");
        for (i = 0; i < dataIdx; i++){
            printf( "%f, ", shmDataBuffer_S2M[i]);
        }
        printf("\n\n");

        printf("NODE -> SDK\n");
        for (i = 0; i < 50; i++){
            printf( "%f, ", shmDataBuffer_M2S[i]);
        }
        printf("\n\n\n\n\n\n");
#endif

        for (i = 0; i < 50; i++){
            rcvdCmdMsg.f[i] = shmDataBuffer_M2S[i+2];
        }

        rcvdDataCnt = shmDataBuffer_M2S[XK_SHM_IDX_M2S_CHK_CNT];
        cmdPortNum = shmDataBuffer_M2S[XK_SHM_IDX_M2S_PORT];

        // if(rcvdDataCnt!=rcvdDataCnt_pre && threadFrameCnt > 1 && (usbStatus[cmdPortNum]==0)){
        //     flgEndOfCmd = 0;

        //     for (i = 0; i < 50; i++){
        //         // printf("[%d]\n", i);

        //         rcvdCmdMsg.f[i] = shmDataBuffer_M2S[XK_SHM_IDX_M2S_CMD_START+i];

        //         // if(shmDataBuffer_M2S[XK_SHM_IDX_M2S_CMD_START+i-1] == SHM_CMD_FINISH_0) printf("yes\n");
        //         // if(shmDataBuffer_M2S[XK_SHM_IDX_M2S_CMD_START+i] == SHM_CMD_FINISH_1) printf("yes\n");

        //         if(i>=1 && shmDataBuffer_M2S[XK_SHM_IDX_M2S_CMD_START+i-1] == SHM_CMD_FINISH_0 && shmDataBuffer_M2S[XK_SHM_IDX_M2S_CMD_START+i] == SHM_CMD_FINISH_1){
        //             flgEndOfCmd=1;
        //         }
        //         if(flgEndOfCmd == 1){
        //             cntCmdSZ = i-1;
        //             break;
        //         }
        //     }

        //     // printf( "port number is   %f   %d   %d \n", cmdPortNum, rcvdDataCnt, cntCmdSZ);

	    //     write(*XK_UARTHandle[cmdPortNum].fd, &rcvdCmdMsg.c[0], cntCmdSZ * 4);
        //     printf("************** wrote **************\n");

                
        //     // for (i = 0; i < cntCmdSZ; i++){
        //     //     printf( "%f, ", rcvdCmdMsg.f[i]);
        //     // }
                
        //     // printf("\nNODE -> SDK\n");
        //     // for (i = 0; i < 50; i++){
        //     //     printf( "%f, ", shmDataBuffer_M2S[i]);
        //     // }
        //     // printf("\n\n");

        //     // printf( "\n\n");

        //     // test_arr[0] = 0;
        //     // test_arr[1] = 0;
        //     // test_arr[2] = 0;
        //     // test_arr[3] = 0;
	    //     // write(*XK_UARTHandle[3].fd, test_arr, 4);

        //     latestCmdCnt = rcvdDataCnt;

        //     // if(latestCmdCnt>=255) latestCmdCnt=0;
        //     // else latestCmdCnt++;

        //     printf("CMD!!");
        // }
        rcvdDataCnt_pre = rcvdDataCnt;

        delay(100);
        if(threadFrameCnt > 1000000000) threadFrameCnt = 100;
        else threadFrameCnt++;
    }
}


