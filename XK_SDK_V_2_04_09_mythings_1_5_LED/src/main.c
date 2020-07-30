#define _GNU_SOURCE
#include "UART.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdint.h>
#include <errno.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include <pthread.h>
#include <sched.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <linux/usbdevice_fs.h>

#include "XK_CommonDefineSet.h"
#include "MsgQueue.h"
#include "httpClient.h"
#include "RadarCommand.h"
#include "parson.h"
#include "otherDevice.h"
#include "XK_usb.h"
#include "XK_atcmd.h"
#include "BehrMythings.h"
#include "XK_LED_lighting.h"



#if INOUT_ENABLE
	#include "Plugin_inoutCounting.h"
#endif

#if FALL_ENABLE
    #include "Plugin_falldetection.h"
    // #include "otherDevice.h"
    #include "subHttpClient.h"
#endif


#define SEND_THREAD_ONOFF       1

typedef union {
	char c[10*4];
	int i[10];
	float f[10];
} rcvmsgUnion_t;

int g_nXKMsgQueId[MAX_NUM_CPU_CORE];

XK_UARTHandle_t XK_UARTHandle[MAX_NUM_USB_DEVICE];
XK_HTTPHandle_t XK_HTTPHandle_o;
XK_HTTPHandle_t *XK_HTTPHandle;
XK_InitInfo_t XK_InitInfo;


pthread_mutex_t g_GetDataMutexLock;
pthread_mutex_t g_QueuemMutexLock;

stMsgQueues g_stXKMsgQueData[MAX_NUM_CPU_CORE];
stMsgQueues p_stXKMsgQueData[MAX_NUM_CPU_CORE];
stGetRadar_Data_t g_Radar_Data[GETDATA_BUFFER_NUMBER + 5];
int g_nXKMsgQueId[MAX_NUM_CPU_CORE];

key_t XK_IpcKey[MAX_NUM_CPU_CORE];
int AlgorithmThreadDeinit(void);
int AlgorithmThreadInit(void);

char device[]="/dev/ttyUSB0";

const char checkChar[]="XAKA";

unsigned long baud = 115200;

int fd[MAX_NUM_USB_DEVICE];
short usbStatus[MAX_NUM_USB_DEVICE];
short usbNodata[MAX_NUM_USB_DEVICE];
short preUsbStatus[MAX_NUM_USB_DEVICE];
struct termios options;

unsigned char flushBuff[RECV_BUFF_SIZE];
unsigned int cntReadyToStore=0;
unsigned int cntReal=0;
unsigned char *pTest, flg1= 0;
unsigned char flgSwapID = 0;

unsigned char recvQueue[MAX_NUM_USB_DEVICE][RECV_BUFF_SIZE];
unsigned int loopi, loopj;
unsigned int preCntData = 0, curCntData = 0;
unsigned char flgNewData = 0;
char gFlgPause=0;
int nShearBufferIdx[MAX_NUM_USB_DEVICE] = {0,};
int nShearBufferIdx_pre[MAX_NUM_USB_DEVICE] = {0,};
double piTemperature;

ParamQ_Handle_t RadarCMD_Q;

static char *tag = "main";
char *help = 
"================ How to execute XK-SDK ================\n"
"   -default:   display default \n"
"   -h:         display help \n"
"   -f:         force run \n"
"   -s:         display raw send data \n"
"   -r:         display raw receive data \n"
"   -w:         display raw data \n"
"   -d:         display debug messege \n"
"   -R:         display free heap memory size (free memory) \n"
"   -u:         display CPU usage \n"
"   -t:         send a-type message \n"
// "   -l:         save radar data \n"
"   -v:         check SDK, API version \n"
// "   -a:         with auto rebooting mode \n"
"=======================================================\n";


char  pritnOpt = MAIN_OPT_DEFAULT;
char  pritnHeap = OFF;
char  pritnCpu = OFF;
char  send_A_type = ON;

unsigned char debugDispUSBnum=255;
int info_send_period;
int info_SIC_interval;
int info_send_if_changed;


const int CH1_Pin = 26; // Active-low button - Broadcom pin 17, P1 pin 11
int test_flg = LOW;
char gFlgLogging = OFF;
// char gFlgAutoReboot = OFF;
char gFlgSysLog = OFF;
char gFlgForce = OFF;
char gFlgFirst = 1;
char gFlgDataLog = 1;
char gFlgAutoReboot = 1;
char gTypeData;
char gTypeRID;

char gPrePid_s[10];
// char gFlgRadarRxStatus[MAX_NUM_USB_DEVICE] = {0,};
char gFlgMythsConnStatus = 0;
char gFlgWiFiConnStatus = 0;

pthread_t Get_XKRadar_Data_Thread_t[MAX_NUM_USB_DEVICE];
pthread_t SendToServer_Thread_t;
pthread_t GatewayServer_Thread_t;
pthread_t USB_Manage_Thread_t;
pthread_t Logging_Thread_t;
pthread_t Mythings_Thread_t;
pthread_t Queues_Thread_t;
pthread_t XK_Watch_Thread_t;
pthread_t XK_Loop_Thread_t;
pthread_t XK_LED_Thread_t;

int clients[CONNMAX];
char *ROOT;
extern listenfd;

unsigned int idxFront[MAX_NUM_USB_DEVICE] = {0,}, preIdxXAKA[MAX_NUM_USB_DEVICE] = {0,}, curIdxXAKA[MAX_NUM_USB_DEVICE] = {0,};
float compareBuffer[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];

#if MYTHINGS_ENABLE == 1
float rcvdData[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];
#endif

struct tm *gLocalTimeTmp;
struct tm gLocalTime;

XK_USBinfo_t infoUSB[USB_DEV_PORT_MAX_SIZE];
XK_USBinfoBySymlink_t infoUSB_bySymlink[USB_DEV_HUB_ADAPTER_SIZE+1];
XK_UsbCmdData_t gUSB_CMD_Data;

int numOfConnectedUSB=0;

void print_USBStaus(short *status);
void *SendToServerThread(void *data);
void *GatewayServerThread(void *data);
void *LoggingDataThread(void *data);
void *XK_WatchThread(void *data);
void *XK_LoopThread(void *data);
void *XK_LEDThread(void *data);
void *GetRadarDataProcess(void *data);

int exists(const char *fname);
void GetConnectionStatus(short * cur, int *USBthreadCreate, pthread_t *Get_XKRadar_Data_Thread_t);
void OptionParser(int argc, char *argv[]);
int XK_USB_Scan_All(void);
int XK_USB_Reset(int port);
int XK_USB_Power_Reset(int port);
void XK_Exit(void);
void IsThereDifference(XK_HTTPHandle_t *HTTPHandle, int port, int idxApp, int cur, int pre);
void CheckAppInfo(void);
void XK_LED_LIGHT_ONOFF(int color, char onOff);

int gRunning;
int IsRunning()
{
    int ret = FALSE;
    gRunning = sem_open(XK_PROCESS_ID, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 1);

    if(gRunning == SEM_FAILED)
    {
        if(errno == EEXIST)
        {            
            // printf("dddddd = 0x%x     %d\n", gRunning, SEM_FAILED);
            LOG_E("Process","'xksdk' Process is already running.");
            printf("  - Enter command to exit.\n  - $ sudo killall -2 xksdk\n  - $ bin/xksdk -f\n");
            exit(-1);
        }
    }

    return ret;
}

void  INThandler(int sig)
{
    char  c;
    LOG_I("Process","Interrupt signal is detected! 'xksdk' process terminated!!");
    sem_close(gRunning);
    sem_unlink(XK_PROCESS_ID);
    exit(-1);
}

int main (int argc, char *argv[])
{
/************************ Do delete ************************
*   This section is for testing to ttyACM0.
*   Using other USB device.
*
************************* Do delete ************************/
#if 0
int fAtcmd;
    XK_ATcmdInit(&fAtcmd);
    XK_ATcmdWriteSimple(fAtcmd, "AT\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "ATI\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "ATE0\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "AT+CSUB\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "AT+CSQ\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "AT+COPS?\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "AT+CGREG?\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "AT&D2\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "AT+CGDCONT=1,\"IP\",\"internet\",,0,0\r");
    XK_ATcmdRead(fAtcmd, 2);
    XK_ATcmdWriteSimple(fAtcmd, "ATD*99#");

delay(2000);    
XK_ATcmdRead(fAtcmd, 2);



    // XK_ATcmdWriteSimple(fAtcmd, AT_CMD_INFO);
    // delay(1000);
    // XK_ATcmdRead(fAtcmd, 2);
    // delay(1000);
    // XK_ATcmdSendUniMsg(fAtcmd, 222.0, 75.2, 25.3);
    // XK_ATcmdRead(fAtcmd, 8);
    XK_ATcmdClose(fAtcmd);


return 0;
#endif

/************************ Do delete ************************
*   This section is for raspberry pi relay board. singtel.
*
*
************************* Do delete ************************/
#if 0
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    pinMode(CH1_Pin, OUTPUT);     // Set regular LED as output
    digitalWrite(CH1_Pin, test_flg); // Turn LED ON
    while(1){
        delay(1000);
        if(test_flg==LOW) test_flg = HIGH;
        else test_flg = LOW;

        digitalWrite(CH1_Pin, test_flg); // Turn LED ON
    }
#endif

    if(argc > 1){
        if(strcmp(argv[1],"-v")==0){        
            printf("- SDK version: %s\n- API version: %s\n Copyright(c) 2019. XandarKardian. All rights reserved.\n", SDK_VERSION, API_VERSION);
            sem_close(gRunning);
            sem_unlink(XK_PROCESS_ID);
            return 0;
        }
        else if(strcmp(argv[1],"-h")==0){
            pritnOpt = MAIN_OPT_HELP;
            printf("%s\n", help);
            return 0;
        }
    } 

    delay(1000);
    int thread3Create;
    thread3Create = pthread_create(&XK_Watch_Thread_t, NULL, (void *)XK_WatchThread, (void *)'a');

    if (pthread_detach(XK_Watch_Thread_t) != 0) 
    {
        perror("pthread_detach(3)\n");
    }    
    
    int thread4Create;
    int thread5Create;
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    pinMode(LEDLIGHT_G0_RED_PIN, OUTPUT);     // Set regular LED as output
    pinMode(LEDLIGHT_G0_YELLOW_PIN, OUTPUT);     // Set regular LED as output
    pinMode(LEDLIGHT_G0_GREEN_PIN, OUTPUT);     // Set regular LED as output
    
    pinMode(LEDLIGHT_G1_RED_PIN, OUTPUT);    
    pinMode(LEDLIGHT_G1_YELLOW_PIN, OUTPUT);    
    pinMode(LEDLIGHT_G1_GREEN_PIN, OUTPUT);    
    pinMode(LEDLIGHT_N0_PIN, OUTPUT);    
    pinMode(LEDLIGHT_N1_PIN, OUTPUT);     
    

    thread4Create = pthread_create(&XK_Loop_Thread_t, NULL, (void *)XK_LoopThread, (void *)'a');
    if (pthread_detach(XK_Loop_Thread_t) != 0) 
    {
        perror("pthread_detach(4)\n");
    }    

    thread5Create = pthread_create(&XK_LED_Thread_t, NULL, (void *)XK_LEDThread, (void *)'a');

    if (pthread_detach(XK_LED_Thread_t) != 0) 
    {
        perror("pthread_detach(5)\n");
    }    

    delay(100);


    int i;
    int k;
    unsigned char tmp;
    
    time_t timer;
    struct tm *t;

    // timer = time(NULL);
    // t = localtime(&timer);
    XK_USB_Scan_All();

    XK_HTTPHandle = &XK_HTTPHandle_o;
    XK_HTTPHandle->optDisp = 0;
    sprintf(XK_HTTPHandle->bootedTime, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:00", gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday, gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec , (gLocalTime.tm_gmtoff/(60*60) > 0)? '+':'-', (gLocalTime.tm_gmtoff/(60*60) > 0)? gLocalTime.tm_gmtoff/(60*60):(gLocalTime.tm_gmtoff/(60*60))*(-1));
    LOG_I("system", "%s Started *****************", XK_HTTPHandle->bootedTime);

    InitInfo(XK_HTTPHandle, &XK_InitInfo);
    info_send_period = XK_InitInfo.info_send_period;
    info_SIC_interval = XK_InitInfo.info_SIC_interval;

    info_send_if_changed = XK_InitInfo.send_if_changed;
    
    gFlgSysLog = XK_InitInfo.flgSystemLog;
    gFlgDataLog = XK_InitInfo.flgDataLog;
    gFlgAutoReboot = XK_InitInfo.flgAutoReboot;
    gTypeData = XK_InitInfo.typeData;
    gTypeRID = XK_InitInfo.typeRID;

    XK_GetMAC(XK_HTTPHandle->MAC_e_Addr, MAC_E_PATH);
    XK_GetMAC(XK_HTTPHandle->MAC_w_Addr, MAC_W_PATH);

    strcpy(gUSB_CMD_Data.MAC_e_Addr, XK_HTTPHandle->MAC_e_Addr);
    strcpy(gUSB_CMD_Data.MAC_w_Addr, XK_HTTPHandle->MAC_w_Addr);
    
    InitParamSize();
    CheckAppInfo();
    
    LOG_D("parameter","XK_InitInfo.info_send_period = %f", XK_InitInfo.info_send_period);
    LOG_D("parameter","XK_InitInfo.flgSystemLog = %d", XK_InitInfo.flgSystemLog);
    LOG_D("parameter","XK_InitInfo.mode = %s", XK_InitInfo.mode);
    LOG_D("parameter","XK_InitInfo.flgSendData = %d", XK_InitInfo.flgSendData);

    LOG_I("Mode","%s \n", XK_InitInfo.mode);
    
    printf("********************** Information ************************\n");
    XK_PrintInfo_s("device", XK_HTTPHandle->info.device);
    XK_PrintInfo_s("function", XK_HTTPHandle->info.function);
    XK_PrintInfo_s("client", XK_HTTPHandle->info.client);
    XK_PrintInfo_s("mode", XK_InitInfo.mode);
    XK_PrintInfo_s("host", XK_HTTPHandle->info.host);
    XK_PrintInfo_s("page", XK_HTTPHandle->info.page);
    XK_PrintInfo_i("port", XK_HTTPHandle->info.port);
    XK_PrintInfo_i("server-port", XK_HTTPHandle->info.server_port);
    XK_PrintInfo_i("send-period(ms)", (int)info_send_period);
    XK_PrintInfo_s("system-logging", (gFlgSysLog==1)? "ON":"OFF");
    XK_PrintInfo_s("data-logging", (gFlgDataLog==1)? "ON":"OFF");
    XK_PrintInfo_s("auto-reboot", (gFlgAutoReboot==1)? "ON":"OFF");
    XK_PrintInfo_c("data-type", gTypeData);
    XK_PrintInfo_c("rID-type", gTypeRID);
    printf("***********************************************************\n\n");

    strcpy(gUSB_CMD_Data.host, XK_HTTPHandle->info.host);
    strcpy(gUSB_CMD_Data.page, XK_HTTPHandle->info.page);
    strcpy(gUSB_CMD_Data.xksdkVer, SDK_VERSION);
    gUSB_CMD_Data.port = XK_HTTPHandle->info.port;
    gUSB_CMD_Data.data_type = gTypeData;
    gUSB_CMD_Data.send_period = info_send_period;

    OptionParser(argc, argv);

    if(gFlgForce == ON){
        sem_close(gRunning);
        sem_unlink(XK_PROCESS_ID);
    }
    signal(SIGINT, INThandler);
    IsRunning();
    WritePID();

    LOG_D("Argument", "argc : %d\n", argc);
    for(k=0;k<argc;k++)
        LOG_D("Argument", "argv%d : %s\n", k, argv[k]);

    for(k=0;k<MAX_NUM_USB_DEVICE;k++)
    {
        XK_UARTHandle[k].fd = &fd[k];
        XK_UARTHandle[k].numUSRCMD = argc;
        XK_UARTHandle[k].devUSB = k;
        XK_UARTHandle[k].prntOptn = pritnOpt;
        XK_UARTHandle[k].recvQueue = recvQueue[k];   
        XK_UARTHandle[k].flgConnection = 0;   
        XK_UARTHandle[k].flgDataAble = 0;   
        XK_UARTHandle[k].cntNodata = 0;
        XK_UARTHandle[k].CheckOneTime = 0;
        XK_HTTPHandle->radarID[k] = 0;
        usbNodata[k] = 0;              
    }
    XK_HTTPHandle->flgSIC = 0;
    
    memset(usbStatus, -1, MAX_NUM_USB_DEVICE*sizeof(short));

    AlgorithmThreadInit();
    pause();
    
    int initialMem = GetRamInKB();

    while(1)
    {  
        delay(1000);
    }
    
    sem_close(gRunning);
    sem_unlink(XK_PROCESS_ID);
    return 0;
}

void *XK_WatchThread(void *data){
    time_t timer;
    char month[12][10] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    FILE *fd_log_file;   
    char aaa[100]; 
    char bbb[100]; 

    while(1){  
        // fd_log_file = fopen("~/testestestest.txt", "a+");  
        timer = time(NULL);         
        gLocalTimeTmp = localtime(&timer);
        memcpy(&gLocalTime,gLocalTimeTmp,sizeof(struct tm));
        delay(100);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
int IsInternetConnected( void){
    int     is_connected;
    int     rtn;

    rtn = system("wget -q --timeout=100 --spider http://google.com");
    if(rtn) is_connected = 0;
    else is_connected = 1;

    return  is_connected;
}


void *XK_LoopThread(void *data){
    int rtn;
    int i;

    int XKtimeCheck_Cur = 0;
    int XKtimeCheck_Pre = 0;
    int timeDiff = 0;

    int period1 = 60;
    int timeBucket = 10000 * period1;

    struct timespec sTimeOutNanoTime_Cur;
    struct timespec sTimeOutNanoTime_Pre;
    Sendmsg_t tmpCastBuff;

    delay(2000);

    while(1){
        if( (timeDiff/10) % period1 == 0)       //60 sec
        {
            gFlgWiFiConnStatus = IsInternetConnected();

            for (i = 0; i < MAX_NUM_USB_DEVICE; i++)
            {
                if(usbStatus[i]==0)
                {
                    // if(XK_UARTHandle[i].rcvDataSize > 0){
                        //serial communication
                        tmpCastBuff.f[0] = 6610.0;
                        write(*XK_UARTHandle[i].fd, &tmpCastBuff.c[0], 1*4);
                        // printf("[%d] triggered\n", i);
                    // }
                    delay(1000);
                    //WiFi or other communications
                    if(gFlgMythsConnStatus == 1 || gFlgWiFiConnStatus == 1){
                        tmpCastBuff.f[0] = 5242.0;    
                        write(*XK_UARTHandle[i].fd, &tmpCastBuff.c[0], 1*4);
                    }
                    // delay(200);
                }
            }
        }
        delay(100);
        
        clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTime_Cur);
        XKtimeCheck_Cur = XK_GetTime(sTimeOutNanoTime_Cur);
        timeDiff = (timeBucket + XKtimeCheck_Cur) % timeBucket;

    }
}


void SetLEDStatus(int groupN, int numOfPeople){
    if(numOfPeople<=gLEDInfo.group[groupN].maxCap){
        LED_Status[groupN][COLOR_IDX_RED]=LED_OFF;
        LED_Status[groupN][COLOR_IDX_YELLOW]=LED_OFF;
        LED_Status[groupN][COLOR_IDX_GREEN]=LED_ON;
    }
    // else if(numOfPeople>gLEDInfo.group[groupN].medianCap && numOfPeople<=gLEDInfo.group[groupN].maxCap){
    //     LED_Status[COLOR_IDX_RED]=LED_ON;
    //     LED_Status[COLOR_IDX_YELLOW]=LED_OFF;
    //     LED_Status[COLOR_IDX_GREEN]=LED_OFF;
    // }
    else if(numOfPeople>gLEDInfo.group[groupN].maxCap){
        LED_Status[groupN][COLOR_IDX_RED]=LED_ON;
        LED_Status[groupN][COLOR_IDX_YELLOW]=LED_OFF;
        LED_Status[groupN][COLOR_IDX_GREEN]=LED_OFF;
    }
}

int GetGroupNum(char *serial){
    int i, j;
    // arr[PARAM_IDX_ZONE_NUM_PEOPLE+4]
    for (i = 0; i < gLEDInfo.numOfGroup; i++){
        for (j = 0; j < gLEDInfo.group[i].numOfSensor; j++){
            if( strcmp(gLEDInfo.group[i].usingSerial[j], serial) == 0 ) {
//                printf("Group #%d  %s     %s \n",i,gLEDInfo.group[i].usingSerial[j], serial);
                return i;
            }
        }
    }

    return -1;
}

void *XK_LEDThread(void *data){
    int i, j;
    int res=0;
    int tmpNumOfPeople[MAX_GROUP_NUM]={0,};
    int aaa = 0;

    int ledPinNum[MAX_GROUP_NUM][3];

    sleep(3);
    /*
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G0_RED_PIN, LED_ON);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G1_RED_PIN, LED_ON);
    sleep(1);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G0_RED_PIN, LED_OFF);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G1_RED_PIN, LED_OFF);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G0_YELLOW_PIN, LED_ON);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G1_YELLOW_PIN, LED_ON);
    sleep(1);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G0_YELLOW_PIN, LED_OFF);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G1_YELLOW_PIN, LED_OFF);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G0_GREEN_PIN, LED_ON);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G1_GREEN_PIN, LED_ON);
    sleep(1);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G0_GREEN_PIN, LED_OFF);
    XK_LED_LIGHT_ONOFF(LEDLIGHT_G1_GREEN_PIN, LED_OFF);
    */

int a, b, c;
    while(1){  
        for (j = 0; j < MAX_GROUP_NUM; j++) tmpNumOfPeople[j] = 0;

        for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
            if(usbStatus[i]==0){
                // printf("#%d.%d\n  %d\n",i,j,(int)g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i][PARAM_IDX_ZONE_NUM_PEOPLE+4]);
                res = GetGroupNum(XK_HTTPHandle->serialNum[i]);
                if(res == -1 && XK_UARTHandle[i].cntNodata<5){
                    // LOG_E("LED", "Invalid Serial Number");
                }
                else{
                    tmpNumOfPeople[res] += (int)g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i][PARAM_IDX_ZONE_NUM_PEOPLE+4];
                }
            }
        }

        for (i = 0; i < MAX_GROUP_NUM; i++){
            SetLEDStatus(i, tmpNumOfPeople[i]);
        }

        // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nHow many people?     G0=%d     G1=%d\n", tmpNumOfPeople[0], tmpNumOfPeople[1]);

        // if(aaa==0){
        //     LED_Status[0][COLOR_IDX_RED]=LED_ON;
        //     LED_Status[0][COLOR_IDX_YELLOW]=LED_OFF;
        //     LED_Status[0][COLOR_IDX_GREEN]=LED_OFF;
        // }
        // else if(aaa==1){    
        //     LED_Status[0][COLOR_IDX_RED]=LED_OFF;
        //     LED_Status[0][COLOR_IDX_YELLOW]=LED_ON;
        //     LED_Status[0][COLOR_IDX_GREEN]=LED_OFF;
        // }
        // else if(aaa==2){
        //     LED_Status[0][COLOR_IDX_RED]=LED_OFF;
        //     LED_Status[0][COLOR_IDX_YELLOW]=LED_OFF;
        //     LED_Status[0][COLOR_IDX_GREEN]=LED_ON;
        // }
        // if (aaa==2)aaa=0;
        // else aaa++;
        
        
        
        for (i = 0; i < MAX_GROUP_NUM; i++){
        // printf("######  %d          %d    %d    %d\n", aaa, LED_Status[i][COLOR_IDX_RED], LED_Status[i][COLOR_IDX_YELLOW], LED_Status[i][COLOR_IDX_GREEN]);
            XK_LED_LIGHT_ONOFF(gLEDInfo.group[i].ledRedPin, LED_Status[i][COLOR_IDX_RED]);
            XK_LED_LIGHT_ONOFF(gLEDInfo.group[i].ledYellowPin, LED_Status[i][COLOR_IDX_YELLOW]);
            XK_LED_LIGHT_ONOFF(gLEDInfo.group[i].ledGreenPin, LED_Status[i][COLOR_IDX_GREEN]);
        }
        delay(500);
    }
}

void *GetRadarDataProcess(void *data)
{
    int nStatus = 0;
    int i, j, temp;
    int nControlMsg = 0;
    float compVar=0;
    unsigned int cntFrame_th0=0;
    int portNum = *(int *)data;
    char buffCMD[30];
    char buffCMD_send[30];
    char buffCMD_info[1000]={0,};
    char flgWait=1;
    char usbDataType = USB_DATA_TYPE_NORMAL;

    long double a[4], b[4], loadavg;
    char dump[50];
    char flushBuff[3000];
    char cntState=0;
    stMsgHandler stQueueMsg;
    rcvmsgUnion_t getData;
    
    cpu_set_t cpu3;
    CPU_ZERO(&cpu3);
    CPU_SET(3, &cpu3);
    temp = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu3);

    for (i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpu3))
        {
            printf("CPU3: CPU %d\n", i);
        }
    }
    
    XK_UARTHandle[portNum].CheckOneTime = 0;

    
    read(fd, &flushBuff[0], MAX_STR_LEN);  

    while(1)
    {
        if(gFlgPause) while(gFlgPause);

        if(debugDispUSBnum!=255){
            XK_UARTHandle[debugDispUSBnum].rcvDataSize = Get_XKRadar_Data_Packet(&XK_UARTHandle[debugDispUSBnum], &g_Radar_Data[nShearBufferIdx[debugDispUSBnum]], idxFront, preIdxXAKA, curIdxXAKA, 255);
        }
        else{            
            XK_UARTHandle[portNum].rcvDataSize = Get_XKRadar_Data_Packet(&XK_UARTHandle[portNum], &g_Radar_Data[nShearBufferIdx[portNum]], idxFront, preIdxXAKA, curIdxXAKA, portNum);
            if(gFlgSysLog){
                XK_HTTPHandle->flgParamSizeErr = 0;
                if(XK_UARTHandle[portNum].rcvDataSize-3 != XK_UARTHandle[portNum].paramSize && XK_UARTHandle[debugDispUSBnum].rcvDataSize > 0){
                    XK_HTTPHandle->flgParamSizeErr = 1;
                }
                for(j=0;j<XK_UARTHandle[portNum].rcvDataSize;j++){
                    compareBuffer[portNum][j] = g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum][j];
                }
                gFlgFirst = 0;
            }

            XK_HTTPHandle->radarID[portNum] = XK_UARTHandle[portNum].radarID;   
            XK_HTTPHandle->cntNodata[portNum] = XK_UARTHandle[portNum].cntNodata;
            XK_HTTPHandle->idxApp[portNum] = GetMetaInfo(XK_HTTPHandle->serialNum[portNum], g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum], XK_UARTHandle[portNum].appNum);

            if(XK_HTTPHandle->idxApp[portNum]>=0) IsThereDifference(XK_HTTPHandle, portNum, XK_HTTPHandle->idxApp[portNum], nShearBufferIdx[portNum], nShearBufferIdx_pre[portNum]);
        }
        // pthread_mutex_lock(&g_QueuemMutexLock);

    
        getData.f[0] = g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum][0];
        getData.f[1] = g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum][1];

        if( getData.c[0]=='X' &&
            getData.c[1]=='A' &&
            getData.c[2]=='K' &&
            getData.c[3]=='A' &&
            getData.c[4]=='U' &&
            getData.c[5]=='C' &&
            getData.c[6]=='M' &&
            getData.c[7]=='D'
        ){
            usbDataType = USB_DATA_TYPE_COMMAND;
            getData.f[0] = g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum][2];   
            buffCMD[0] = getData.c[0];
            buffCMD[1] = getData.c[1];
            buffCMD[2] = getData.c[2];
            buffCMD[3] = getData.c[3];
            buffCMD[4] = '\0';

            if(strcmp(buffCMD, XK_USB_CMD_IS_CONNECTED)==0 && flgWait==1){
                // printf("is connected?\n");
                sprintf(buffCMD_send, "XAKAUCMD%s", XK_USB_CMD_STRAT);
                if (cntState==0) write(*XK_UARTHandle[portNum].fd, buffCMD_send, 12);
                memset(buffCMD, 0, 12);
                cntState = 1;
            }            
            else if(strcmp(buffCMD, XK_USB_CMD_GET_INFO)==0 && flgWait==1){
                // printf("info?\n");
                XK_GetTeamviewerID(gUSB_CMD_Data.teamviewerID, "/etc/teamviewer/global.conf");
                XK_GetTeamviewerVer(gUSB_CMD_Data.teamviewerVer, "/etc/teamviewer/global.conf");

                sprintf(buffCMD_info, "XAKAUCMD%s", XK_USB_CMD_DATA);
                // sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", XK_USB_CMD_DATA);  
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.teamviewerID);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.teamviewerVer);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.MAC_e_Addr);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.MAC_w_Addr);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.xksdkVer);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%d", gUSB_CMD_Data.send_period);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%c", gUSB_CMD_Data.data_type);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.host);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%s", gUSB_CMD_Data.page);
                sprintf(&buffCMD_info[strlen(buffCMD_info)], ",%d", gUSB_CMD_Data.port);

                if (cntState==1) write(*XK_UARTHandle[portNum].fd, buffCMD_info, strlen(buffCMD_info));
                memset(buffCMD, 0, 12);
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

        nShearBufferIdx_pre[portNum] = nShearBufferIdx[portNum];

        if(usbDataType == USB_DATA_TYPE_COMMAND){
            memset(g_Radar_Data[nShearBufferIdx[portNum]].m_nALLRadarData[portNum], 0, 4*4);
        } 
        else nShearBufferIdx[portNum]++;
        
        if(nShearBufferIdx[portNum] >= GETDATA_BUFFER_NUMBER)
        {
            nShearBufferIdx[portNum] = 0;
        }

        // pthread_mutex_unlock(&g_QueuemMutexLock);
        if(cntFrame_th0>3000000000) cntFrame_th0 = 1000;
        else cntFrame_th0++;
        delay(10);
    }
    return 0;
}

void XK_LED_LIGHT_ONOFF(int color, char onOff){
    digitalWrite(color, onOff); 
}


int XK_GetTime(struct timespec srcTime){
    float res;

    res = (float)(srcTime.tv_nsec/100000000);
    res += (srcTime.tv_sec%1000)*10;

    return res;
}

void *SendMythings(void *data)
{
    int nStatus = 0;
    int i, temp;
    int nControlMsg = 0;
    unsigned long waitGETTime=0;
    struct timespec sTimeOutNanoTimeSendPeriod;
    struct timespec sTimeOutNanoTimeSendPeriod_pre;

    long long timeCheckSend = 0;
    long long timeCheckSend_pre = 0;

    int XKtimeCheckSend = 0;
    int XKtimeCheckSend_pre = 0;

    int isChanged=0;
    int idxApp=-1;
    int rtn;
    int timeDiff=0;

    char stateCMD[MAX_NUM_USB_DEVICE]={0,};
    int sendCnt[MAX_NUM_USB_DEVICE]={0,};
    int sendCntMain=0;
    int res=1;

    int errCnt=0;
    int _errCnt=0;

    char tmpAtPacket[MYTHINGS_MSG_SZ];
    char tmpAtPacket_asc[MYTHINGS_MSG_SZ*2];
    char destMsg[MYTHINGS_MSG_SZ+50];
    int fAtcmd;
    char isZone = 0;

    // wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    // pinMode(LEDLIGHT_RED_PIN, OUTPUT);     // Set regular LED as output
    // pinMode(LEDLIGHT_YELLOW_PIN, OUTPUT);     // Set regular LED as output
    // pinMode(LEDLIGHT_GREEN_PIN, OUTPUT);     // Set regular LED as output

    typedef struct
    {
        int appNum;
        char serial[13];
    } MythingsRcvInfo_t;

    MythingsRcvInfo_t myInfo[MAX_NUM_USB_DEVICE];
    
    stGetRadar_Data_t *pProcessInput_Data;
    cpu_set_t cpu1;

    CPU_ZERO(&cpu1);
    CPU_SET(1, &cpu1);
    temp = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu1);
    for (i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpu1))
        {
            printf("CPU1: CPU %d\n", i);
        }
    }

    delay(3000);
    XK_ATcmdInit(&fAtcmd);

    
    // rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod_pre);
    XKtimeCheckSend_pre = 0;

    while(1)
    {
        rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod);
        XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeSendPeriod);

        timeDiff = (10000 + XKtimeCheckSend - XKtimeCheckSend_pre) % 10000;
        // printf("%d \n", timeDiff);

        if( timeDiff  > MYTHINGS_SEND_PERIOD*10 )
        {
            for (i = 0; i < MAX_NUM_USB_DEVICE; i++)
            {
                if(usbStatus[i]==0){
                    memcpy(rcvdData[i], g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i], NUM_RCV_PARA*4);
                }
            }
            memset(tmpAtPacket, 0, MYTHINGS_MSG_SZ);
            memset(tmpAtPacket_asc, 0, MYTHINGS_MSG_SZ*2);

            for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
                if(usbStatus[i]==0){
                    if(strlen(tmpAtPacket)>0) tmpAtPacket[strlen(tmpAtPacket)] = SEND_SEPARATOR_COMMON;
                    switch(XK_UARTHandle[i].appNum){
                        case  21:       //INOUT single
                            if(sendCnt[i] == SEND_PERIOD_INOUT_VERSION){
                                stateCMD[i] = SENDING_STATE_INOUT_VER;
                                if(res) sendCnt[i] = 0;
                            }
                            else{
                                stateCMD[i] = SENDING_STATE_INOUT_NORMAL;
                                sendCnt[i]++;
                            }
                            
                            if(stateCMD[i] == SENDING_STATE_INOUT_VER){                            
                                ///////////////// inout version /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_INOUT_VERSION;

                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_VER+4])/100000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_VER+4])/10000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_VER+4])/1000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_VER+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_VER+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_VER+4])%10)+'0';
                            }
                            else {
                                ///////////////// inout normal /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_INOUT_NORMAL;
                                
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_CONN_STS+4])%10)+'0';
                                
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_IN_CNT+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_IN_CNT+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_IN_CNT+4])%10)+'0';

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_OUT_CNT+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_OUT_CNT+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_OUT_CNT+4])%10)+'0';

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_INOUT_UPDT_CNT+4])%10)+'0';

                            }
                        break;

                        case  30:       //Zone
                            if(sendCnt[i] == SEND_PERIOD_ZONE_VERSION){
                                stateCMD[i] = SENDING_STATE_ZONE_VER;
                                if(res) sendCnt[i] = 0;
                            }
                            else if(sendCnt[i] % SEND_PERIOD_ZONE_MAP_VAL == 0){
                                stateCMD[i] = SENDING_STATE_ZONE_MAP_VAL;
                                if(res) sendCnt[i]++;
                            }
                            else{
                                stateCMD[i] = SENDING_STATE_ZONE_NORMAL;
                                sendCnt[i]++;
                            }

                            if(stateCMD[i] == SENDING_STATE_ZONE_VER){
                                ///////////////// zone version /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_ZONE_VERSION;
                                
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);
                                
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_VER+4])/100000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_VER+4])/10000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_VER+4])/1000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_VER+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_VER+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_VER+4])%10)+'0';
                            }
                            else if(stateCMD[i] == SENDING_STATE_ZONE_MAP_VAL){
                                ///////////////// zone mapping val /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_ZONE_MAP_VAL;
                                
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_RDR_MAP_IDX+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_RDR_MAP_IDX+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_RDR_MAP_IDX+4])%10)+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (unsigned int)((rcvdData[i][PARAM_IDX_ZONE_RDR_MAP_IDX+4])*10)%10+'0';

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_ENV_MAP_IDX+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_ENV_MAP_IDX+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_ENV_MAP_IDX+4])%10)+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (unsigned int)((rcvdData[i][PARAM_IDX_ZONE_ENV_MAP_IDX+4])*10)%10+'0';
                            }
                            else{
                                ///////////////// zone normal /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_ZONE_NORMAL;

                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_PRE_STS+4])%10)+'0';

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_NUM_PEOPLE+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_NUM_PEOPLE+4])%10)+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (unsigned int)((rcvdData[i][PARAM_IDX_ZONE_NUM_PEOPLE+4])*10)%10+'0';

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_LONG_T_IDX+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_LONG_T_IDX+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_ZONE_LONG_T_IDX+4])%10)+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (unsigned int)((rcvdData[i][PARAM_IDX_ZONE_LONG_T_IDX+4])*10)%10+'0';
                            }
                        break;

                        case  40:       //Presence
                        
                            if(sendCnt[i] == SEND_PERIOD_PRES_VERSION){
                                stateCMD[i] = SENDING_STATE_PRES_VER;
                                if(res) sendCnt[i] = 0;
                            }
                            else{
                                stateCMD[i] = SENDING_STATE_PRES_NORMAL;
                                sendCnt[i]++;
                            }

                            if(stateCMD[i] == SENDING_STATE_PRES_VER){
                                ///////////////// presence normal /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_PRES_VERSION;
                                
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);
                                
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_VER+4])/100000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_VER+4])/10000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_VER+4])/1000)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_VER+4])/100)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_VER+4])/10)%10+'0';
                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_VER+4])%10)+'0';
                            }
                            else{
                                ///////////////// presence normal /////////////////
                                tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_PRES_NORMAL;
                                
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%d", XK_UARTHandle[i].appNum);
                                sprintf(&tmpAtPacket[strlen(tmpAtPacket)], "%s", XK_HTTPHandle->serialNum[i]);

                                tmpAtPacket[strlen(tmpAtPacket)] = (((unsigned int)rcvdData[i][PARAM_IDX_PRES_PRES_STS+4])%10)+'0';
                            }
                        break;

                        default :       //
                            tmpAtPacket[strlen(tmpAtPacket)] = SEND_CMD_COMMON_ERROR;
                            tmpAtPacket[strlen(tmpAtPacket)] = SEND_ERROR_INVALID_APPNUM + 48;
                            LOG_E("SendMythings", "Invalid App number");
                        break;
                    }
                }
            }

            if(strlen(tmpAtPacket) >= MYTHINGS_MSG_SEND_LIMIT_SZ) {    
                memset(tmpAtPacket, 0, MYTHINGS_MSG_SZ);        
                tmpAtPacket[0] = SEND_CMD_COMMON_ERROR;
                tmpAtPacket[1] = SEND_ERROR_OVERFLOW_DATA + 48;
            }
            else if(strlen(tmpAtPacket) == 0){
                memset(tmpAtPacket, 0, MYTHINGS_MSG_SZ);        
                tmpAtPacket[0] = SEND_CMD_COMMON_ERROR;
                tmpAtPacket[1] = SEND_ERROR_EMPTY + 48;
            }
            
            tmpAtPacket[strlen(tmpAtPacket)] = '\0';
            for(i=0 ; i<strlen(tmpAtPacket) ; i++) sprintf(&tmpAtPacket_asc[i*2], "%02x", tmpAtPacket[i]);

    // printf("%d:%s\n", sendCntMain, tmpAtPacket);
    // printf("%d:%s\n\n", sendCntMain, tmpAtPacket_asc);

            sprintf(destMsg, AT_CMD_UNI, strlen(tmpAtPacket_asc)/2, AT_TAB, tmpAtPacket_asc, AT_EOF);
            // delay(1000);

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
                    XK_ATcmdWriteSimple(fAtcmd, AT_CMD_RST);
                    delay(2000);
                }
            }

            // rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod);
            // XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeSendPeriod);
            // rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod_pre);
            XKtimeCheckSend_pre = XKtimeCheckSend;
        }

        delay(100);
    }
}


void *SendToServerThread(void *data)
{
    int nStatus = 0;
    int i, temp;
    int nControlMsg = 0;
    // int nShearBufferIdx = 0;
    int res=0;
    unsigned long waitGETTime=0;
    // long double a[4], b[4], loadavg;
    struct timespec sTimeOutNanoTimeVal;
    struct timespec sTimeOutNanoTimeTmp;
    struct timespec sTimeOutNanoTimeSendPeriod;
    struct timespec sTimeOutNanoTimeSendPeriod_pre;

    long long timeCheckSend = 0;
    long long timeCheckSend_pre = 0;

    int XKtimeCheckSend = 0;
    int XKtimeCheckSend_pre = 0;

    int isChanged=0;
    // int nShearBufferIdx_pre[MAX_NUM_USB_DEVICE];
    int idxApp=-1;

    int rtn;
    int timeDiff=0;
    // long long  timeDiff=0;

    stGetRadar_Data_t *pProcessInput_Data;
    cpu_set_t cpu1;
       
#if ONOFF_SUB_EPOINT
    char host_ori[100];
    char page_ori[100];
    int port_ori;
    strcpy(host_ori, XK_HTTPHandle->info.host);
    strcpy(page_ori, XK_HTTPHandle->info.page);
    port_ori = XK_HTTPHandle->info.port;
#endif

    CPU_ZERO(&cpu1);
    CPU_SET(1, &cpu1);
    temp = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu1);
    for (i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpu1))
        {
            printf("CPU1: CPU %d\n", i);
        }
    }

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

        if( timeDiff > (long long)info_send_period / 100 || (((XK_HTTPHandle->flgSIC==1 && timeDiff > (long long)info_SIC_interval / 100))&&info_send_if_changed))
        // if( (timeCheckSend - timeCheckSend_pre) > (long long)info_send_period * 1000000 || (XK_HTTPHandle->flgSIC==1 && (timeCheckSend - timeCheckSend_pre) > (long long)info_SIC_interval * 1000000))
        {
            // LOG_I("test","time stamp %d", timeDiff);
            if(XK_HTTPHandle->flgSIC == 1) XK_HTTPHandle->flgSIC = 0;

            for (i = 0; i < MAX_NUM_USB_DEVICE; i++)
            {
                if(usbStatus[i]==0){
                    // for (j = 0; j < NUM_RCV_PARA; j++) XK_HTTPHandle->rcvdRadarData[i][j] = g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i][j];
                    memcpy(XK_HTTPHandle->rcvdRadarData[i], g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i], NUM_RCV_PARA*4);
                    XK_HTTPHandle->rcvDataSize[i] = XK_UARTHandle[i].rcvDataSize;
                    XK_HTTPHandle->appNum[i] = XK_UARTHandle[i].appNum;
                    XK_HTTPHandle->fd[i] = *XK_UARTHandle[i].fd;

                    // XK_HTTPHandle->idxApp[i] = GetMetaInfo(XK_HTTPHandle->serialNum, XK_HTTPHandle->rcvdRadarData[i], XK_HTTPHandle->appNum[i]);
                    // IsThereDifference(XK_HTTPHandle, i, XK_HTTPHandle->idxApp[i], nShearBufferIdx[i], nShearBufferIdx_pre[i]);


                }
            }

#if ONOFF_SUB_EPOINT
            strcpy(XK_HTTPHandle->info.host, host_ori);
            strcpy(XK_HTTPHandle->info.page, page_ori);
            XK_HTTPHandle->info.port = port_ori;
#endif

            res = SendRadarData(XK_HTTPHandle);
            if(res == 1){
                rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeSendPeriod_pre);
                XKtimeCheckSend_pre = XK_GetTime(sTimeOutNanoTimeSendPeriod_pre);
            }
            else{
                if( (sTimeOutNanoTimeTmp.tv_sec - sTimeOutNanoTimeVal.tv_sec) > (5 * 60)){
                    LOG_E("SendToServerThread","No response from server.");
                    // LOG_E("SendToServerThread","Getting IP address...");
                    // LOG_E("SendToServerThread","Rebooting XK-SDK...");
                    // system("sudo xksdk -f &");
                    // exit(0);
                }
                rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeTmp);
                // timeCheckSend_pre += (long long)(1*SIZE_NSEC);
            }


            // if(gFlgAutoReboot){
            if(0){
                if(res == 1){
                    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeVal);
                }
                else{
                    if( (sTimeOutNanoTimeTmp.tv_sec - sTimeOutNanoTimeVal.tv_sec) > (5 * 60))  system("sudo reboot");
                    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeTmp);
                }
            }

#if ONOFF_SUB_EPOINT
            strcpy(XK_HTTPHandle->info.host, SUB_E_POINT);
            strcpy(XK_HTTPHandle->info.page, SUB_E_PAGE);
            XK_HTTPHandle->info.port = SUB_PORT;

            res = SendRadarData(XK_HTTPHandle);

            if(gFlgAutoReboot){
                if(res == 1){
                    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeVal);
                }
                else{
                    if( (sTimeOutNanoTimeTmp.tv_sec - sTimeOutNanoTimeVal.tv_sec) > (5 * 60))  system("sudo reboot");
                    rtn = clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeTmp);
                }
            }
#endif
        }
        delay(100);
        // delay((int)info_send_period);
    }
}

void *GatewayServerThread(void *data)
{

#if 0
    int nStatus = 0;
    int i, temp;
    int nControlMsg = 0;
    // int nShearBufferIdx = 0;
    int res=0;
    unsigned long waitGETTime=0;
    // long double a[4], b[4], loadavg;
    struct timespec sTimeOutNanoTimeVal;
    struct timespec sTimeOutNanoTimeTmp;
    int rtn;
    stGetRadar_Data_t *pProcessInput_Data;
    cpu_set_t cpu1;

    CPU_ZERO(&cpu1);
    CPU_SET(1, &cpu1);
    temp = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu1);
    for (i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpu1))
        {
            printf("CPU1: CPU %d\n", i);
        }
    }

#define  BUFF_SIZE   1024

    delay(2000);
    int server_socket;
    int client_socket;
    int client_addr_size;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char buff_rcv[BUFF_SIZE+5];
    char buff_snd[BUFF_SIZE+5];

    struct ifreq ifr_e;
    char localIP_e[50];
    struct ifreq ifr_w;
    char localIP_w[50];

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == server_socket){
        LOG_E("Server", "Can't create socket");
        exit(1);
    }

    memcpy(ifr_e.ifr_name, "eth0", IFNAMSIZ-1);ioctl(server_socket, SIOCGIFADDR, &ifr_e);
    strcpy(localIP_e, inet_ntoa(((struct sockaddr_in *)&ifr_e.ifr_addr)->sin_addr));
    XK_HTTPHandle->IPAddr_e = localIP_e;

    memcpy(ifr_w.ifr_name, "wlan0", IFNAMSIZ-1);ioctl(server_socket, SIOCGIFADDR, &ifr_w);
    strcpy(localIP_w, inet_ntoa(((struct sockaddr_in *)&ifr_w.ifr_addr)->sin_addr));
    XK_HTTPHandle->IPAddr_w = localIP_w;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family     = AF_INET;
    server_addr.sin_port       = htons(XK_HTTPHandle->info.server_port);
    server_addr.sin_addr.s_addr= htonl(INADDR_ANY);

    if(-1 == bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        LOG_E("Server", "bind error!");
        // close(server_socket);
        exit(1);
    }

    if(-1 == listen(server_socket, 5)){
        LOG_E("Server", "wait mode fail");
        exit(1);
    }
    LOG_I("System", "XandarKardian gateway server start to listening...");
    while(1){

        client_addr_size  = sizeof( client_addr);
        client_socket     = accept( server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

        if (-1 == client_socket){
            LOG_E("Server", "client connect fail");
            close(client_socket);
            continue;
            // exit(1);
        }
                 
        for (i = 0; i < MAX_NUM_USB_DEVICE; i++)
        {
            if(usbStatus[i]==0){
                memcpy(XK_HTTPHandle->rcvdRadarData[i], g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i], NUM_RCV_PARA);
                XK_HTTPHandle->rcvDataSize[i] = XK_UARTHandle[i].rcvDataSize;
                XK_HTTPHandle->appNum[i] = XK_UARTHandle[i].appNum;
                XK_HTTPHandle->fd[i] = *XK_UARTHandle[i].fd;
            }
        }
        res = respond(XK_HTTPHandle, client_socket);
    }

#endif
}


void *USB_Manage_Thread(void *data)
{
    int i;
    int tmpParam;
    int USBthreadCreate[MAX_NUM_USB_DEVICE];
    long double a[4], b[4], loadavg;
    int initialMem = GetRamInKB();
    int preTimeDay;
    int preTimeHour;

    time_t timer;
    struct tm *t;
    
    for (i = 0; i < MAX_NUM_USB_DEVICE; i++) XK_HTTPHandle->USBStatus[i] = -1;
    
    for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
        sprintf(device, "/dev/ttyUSB%d", i);
        if( access( device, R_OK | W_OK ) == 0 ){
            // usbStatus[i] = 0;
            UART_init(XK_UARTHandle[i].fd, device, &usbStatus[i]);
            XK_HTTPHandle->USBStatus[i] = usbStatus[i];
        }
        else{

        }
    }
    for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
        if(usbStatus[i]==0) {
            tmpParam = i;
            USBthreadCreate[i] = pthread_create(&Get_XKRadar_Data_Thread_t[i], NULL, (void *)GetRadarDataProcess, (void *)&tmpParam);
            if (pthread_detach(Get_XKRadar_Data_Thread_t[i]) != 0) 
            {
                LOG_E("pthread_detach","Can't create Get_XKRadar_Data_Thread [%d]", i);
                // perror("pthread_detach(0)\n");
            }
            delay(100);
        }
    }

    preTimeHour = gLocalTime.tm_hour;
            
    while(1)
    {        
        if(gLocalTime.tm_hour != preTimeHour) {
            // system("sync");
            // system("sudo sysctl -w vm.drop_caches=3");
            preTimeHour = gLocalTime.tm_hour;
        }

        CPU_Tick(a); 
        XK_HTTPHandle->cpuUsage = (unsigned int)(loadavg*100);
        GetConnectionStatus(usbStatus, USBthreadCreate, Get_XKRadar_Data_Thread_t);      

        piTemperature = get_temp(0);
        XK_HTTPHandle->piTemperature = piTemperature;   
        XK_HTTPHandle->memHeap = GetRamInKB();   
        if(pritnCpu == ON) printf("The current CPU utilization is : %d%%  Temperature = %3.3f'C\n", (unsigned int)(loadavg*100), piTemperature);
        if(pritnHeap == ON) printf("Available DOS memory = %ld bytes      Initial Memory = %ld bytes       diff = %ld bytes\n", GetRamInKB(), initialMem, GetRamInKB() - initialMem);

        delay((int)1000);
        
        loadavg = CPU_Tock(a, b);
    }
}

void *LoggingDataThread(void *data)
{
    int file_access;  
    char logPrePath[100]={0,}; 
    char logFullPath[200]={0,}; 
    char logInfoBuff[200]={0,}; 
    int preTimeSec = -1;     
    int preTimeDay = -1;     
    int i, j;
    FILE *fd_log_file;    
    char month[12][10] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    char capChkPath[100]={0,};
    struct stat file_info;
    fpos_t gCurPos;

    time_t timer;
    struct tm *t;
    
    preTimeDay = gLocalTime.tm_mday;
    delay(4000);
    while(1)
    {
        // timer = time(NULL);         
        // t = localtime(&timer); 
        if(preTimeSec == -1) {
            for (i = 0; i < MAX_NUM_USB_DEVICE; i++) {
                if(usbStatus[i]==0) {   
                    sprintf(logPrePath, LOG_PATH, LOG_NAME);        
                    sprintf(logFullPath, logPrePath, i, gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday);

                    // fd_log_file = fopen(logFullPath, "w");

                    file_access = access(logFullPath, R_OK | W_OK);
                    if(file_access == -1)
                        fd_log_file = fopen(logFullPath, "w");
                    else if(file_access == 0)
                        fd_log_file = fopen(logFullPath, "a+");

                    if ( fd_log_file == NULL ) {
                        LOG_E("LOGGING","log file open error\n"); 
                        return -1;       
                    }
                    LOG_I("LOGGING", "Succeed to create log file port: %d", i);
                    
                    if(file_access == -1){
                        sprintf(logInfoBuff, LOG_INFO,  XK_HTTPHandle->info.device,
                                                        XK_HTTPHandle->info.page,
                                                        XK_HTTPHandle->info.client,
                                                        XK_HTTPHandle->info.function,
                                                        XK_HTTPHandle->info.client,
                                                        XK_HTTPHandle->info.serial,
                                                        XK_HTTPHandle->info.port,
                                                        month[gLocalTime.tm_mon], gLocalTime.tm_mday, gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec );
                        fwrite(logInfoBuff, 1, strlen(logInfoBuff), fd_log_file);
                    }
                    else if(file_access == 0){
                        fwrite("**rebooted\n", 1, 11, fd_log_file);       
                    }
                    
                    fclose(fd_log_file);
                    
                    LOG_I("LOGGING", "Succeed to write log info port: %d", i);
                }
            }
            preTimeSec = gLocalTime.tm_sec;
            // LOG_I("LOGGING", "Succeed to write log info");
        }
        else if(gLocalTime.tm_sec != preTimeSec) {
            for (i = 0; i < MAX_NUM_USB_DEVICE; i++) {
                if(usbStatus[i]==0) {   
                    sprintf(logPrePath, LOG_PATH, LOG_NAME);
                    sprintf(logFullPath, logPrePath, i, gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday);
                    fd_log_file = fopen(logFullPath, "a+");

                    if ( fd_log_file == NULL ) {
                        printf("log file open error\n"); 
                        return -1;
                    }

                    fprintf(fd_log_file, "%02d:%02d:%02d %s %02d,", gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec, month[gLocalTime.tm_mon], gLocalTime.tm_mday);

                    if(XK_UARTHandle[i].rcvDataSize>0){
                        for (j = 0; j < XK_UARTHandle[i].rcvDataSize; j++) {
                            fprintf(fd_log_file, "%f,", g_Radar_Data[(GETDATA_BUFFER_NUMBER + nShearBufferIdx[i] - 1)%GETDATA_BUFFER_NUMBER].m_nALLRadarData[i][j]);
                        }
                    }
                    
                    fprintf(fd_log_file, "\n");                    
                    fclose(fd_log_file);
                }
            }
            if(gLocalTime.tm_mday != preTimeDay) {
                int retval;
                retval = system("sudo find /var/log/xk/*.csv -maxdepth 1 -mtime +20 -delete");
                retval = system("sudo find /var/log/xk/sys/*.xkl -maxdepth 1 -mtime +20 -delete");
                LOG_W("Debug","Day 1+");
                preTimeDay = gLocalTime.tm_mday;
            }
                            
            preTimeSec = gLocalTime.tm_sec;
        }
        else {
        }
        delay(100);
    }
    fclose(fd_log_file);
}




int AlgorithmThreadInit(void)
{
    int nAlgoBufferi;


    int USBthreadCreate[MAX_NUM_USB_DEVICE], thread0Create, thread1Create, thread2Create, thread3Create, threadMythingsCreate, i, temp;
    int nQueueThreadCreate;
    
//     QueueInit();
//     XK_IpcKey[0] = ftok("Get_XKRadar_Data_Packet",1);
// #if SEND_THREAD_ONOFF
//     XK_IpcKey[1] = ftok("SendToServerThread",1);
// #endif
//     if((g_nXKMsgQueId[0] = msgget(XK_IpcKey[0], IPC_CREAT|0660)) == -1)
//     {
//         perror("Get Radar Data Msgget");
//         exit(1);
//     }
//     if((g_nXKMsgQueId[1] = msgget(XK_IpcKey[1], IPC_CREAT|0660)) == -1)
//     {
//         perror("Send To Server Msgget");
//         exit(1);
//     }
    // pthread_mutex_init(&g_GetDataMutexLock, NULL);
    // pthread_mutex_init(&g_QueuemMutexLock, NULL);
    
    // thread3Create = pthread_create(&XK_Watch_Thread_t, NULL, (void *)XK_WatchThread, (void *)'a');

    // if (pthread_detach(XK_Watch_Thread_t) != 0) 
    // {
    //     perror("pthread_detach(3)\n");
    // }    
    


    if(debugDispUSBnum!=255){
         
        sprintf(device, "/dev/ttyUSB%d", debugDispUSBnum);
        if( access( device, R_OK | W_OK ) == 0 ){
            UART_init(XK_UARTHandle[debugDispUSBnum].fd, device, &usbStatus[debugDispUSBnum]);
        }
        else{
            LOG_E("USB", "Invalid USB Port");
            exit(0);
        }
        int tmpParam = debugDispUSBnum;
        USBthreadCreate[debugDispUSBnum] = pthread_create(&Get_XKRadar_Data_Thread_t[debugDispUSBnum], NULL, (void *)GetRadarDataProcess, (void *)&tmpParam);
        if (pthread_detach(Get_XKRadar_Data_Thread_t[debugDispUSBnum]) != 0) 
        {
            perror("pthread_detach(0)\n");
        }
        delay(100);
    }
    else{

# if 1
#if INOUT_ENABLE
        inoutInit(g_Radar_Data, XK_UARTHandle, nShearBufferIdx, usbStatus);
#endif

#if FALL_ENABLE
        wmFallinit(g_Radar_Data, XK_UARTHandle, nShearBufferIdx, usbStatus);
#endif
#if (FALL_ENABLE || INOUT_ENABLE)
        otherDevHinit(XK_UARTHandle, usbStatus, Get_XKRadar_Data_Thread_t,XK_HTTPHandle);
#endif

    #if SEND_THREAD_ONOFF
        if(strcmp(XK_InitInfo.mode,"polling") == 0 && XK_InitInfo.flgSendData == 1){
            thread1Create = pthread_create(&SendToServer_Thread_t, NULL, (void *)SendToServerThread, (void *)'a');
        }
        else if(strcmp(XK_InitInfo.mode,"event") == 0){
            thread1Create = pthread_create(&GatewayServer_Thread_t, NULL, (void *)GatewayServerThread, (void *)'a');
        }
        else{
        }
    #endif
        thread0Create = pthread_create(&USB_Manage_Thread_t, NULL, (void *)USB_Manage_Thread, (void *)'a');

        if(gFlgDataLog){
            thread2Create = pthread_create(&Logging_Thread_t, NULL, (void *)LoggingDataThread, (void *)'a');
        }

        if (pthread_detach(USB_Manage_Thread_t) != 0) 
        {
            perror("pthread_detach(0)\n");
        }    
    #if SEND_THREAD_ONOFF
        if(strcmp(XK_InitInfo.mode,"polling") == 0 && XK_InitInfo.flgSendData == 1){
            if (pthread_detach(SendToServer_Thread_t) != 0) 
            {
                perror("pthread_detach(1)\n");
            }
        }
        else if(strcmp(XK_InitInfo.mode,"event") == 0){
            if (pthread_detach(GatewayServer_Thread_t) != 0) 
            {
                perror("pthread_detach(1)\n");
            }
        }
        else{
        }
    #endif
        if(gFlgDataLog){
            if (pthread_detach(Logging_Thread_t) != 0) 
            {
                perror("pthread_detach(2)\n");
            }
        }



#if (MYTHINGS_ENABLE == 1)
        threadMythingsCreate = pthread_create(&Mythings_Thread_t, NULL, (void *)SendMythings, (void *)'a');
        if (pthread_detach(Mythings_Thread_t) != 0) 
        {
            perror("pthread_detach(Mythings_Thread_t)\n");
        }
#endif 




#endif
    }
    // pthread_mutex_lock(&g_QueuemMutexLock);
    // pthread_mutex_unlock(&g_QueuemMutexLock);

    return 0;
}

int AlgorithmThreadDeinit(void)
{
    pthread_mutex_destroy(&g_GetDataMutexLock);
    pthread_mutex_destroy(&g_QueuemMutexLock);

    return 0;
}

void print_USBStaus(short *status){
    int i;
    printf("usbStatus\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n\t\t");
    for (i = 0; i < 10; i++)
    {
        printf("%d\t", status[i]);
    }
    printf("\n");
}

int exists(const char *fname)
{
	int mode = R_OK | W_OK;
    if( access( fname, mode ) == 0 ){
        return 1;
    }
    else{
        return 0;
    }
}

void GetConnectionStatus(short * cur, int *USBthreadCreate, pthread_t *Get_XKRadar_Data_Thread_t){
    int i, j;    
    char flgCheckUSB=0;
    FILE *tmpFD;
    int tmpParam;
    int flgFirst=0;
    int flgUpdate=0;

    for (i = 0; i < MAX_NUM_USB_DEVICE; i++){        
        sprintf(device, "/dev/ttyUSB%d", i);    
        flgCheckUSB = exists(device);
        if(cur[i]==-1 && flgCheckUSB==1){
            XK_USB_Scan_All();
            sprintf(device, "/dev/ttyUSB%d", i);
            LOG_I("USB", "USB%d Connected!!", i);

            UART_init(XK_UARTHandle[i].fd, device, &cur[i]);
            
            tmpParam = i;
            USBthreadCreate[i] = pthread_create(&Get_XKRadar_Data_Thread_t[i], NULL, (void *)GetRadarDataProcess, (void *)&tmpParam);
            if (pthread_detach(Get_XKRadar_Data_Thread_t[i]) != 0) 
            {
                perror("pthread_detach(0)\n");
            }
       
            delay(100);

            XK_HTTPHandle->USBStatus[i] = cur[i];
            XK_UARTHandle[i].cntNodata = 0;                        
        }
        else if(cur[i] == 0 && flgCheckUSB == 0) {
            pthread_cancel(Get_XKRadar_Data_Thread_t[i]);
            cur[i] = -1;
            XK_HTTPHandle->USBStatus[i] = cur[i];
            XK_UARTHandle[i].radarID = 0;        
            close(*XK_UARTHandle[i].fd);
            // sleep(1);
            XK_USB_Scan_All();
            LOG_E("USB", "USB%d cable disconnected", i);
        }
        else if(cur[i] == 0 && flgCheckUSB == 1 && XK_UARTHandle[i].cntNodata>=5) {
            // if(XK_UARTHandle[i].cntNodata % 10 == 0){
            //     pthread_cancel(Get_XKRadar_Data_Thread_t[i]);
            //     cur[i] = -1;
            //     XK_HTTPHandle->USBStatus[i] = cur[i];
            //     XK_UARTHandle[i].radarID = 0;
            //     close(*XK_UARTHandle[i].fd);
            //     XK_UARTHandle[i].cntNodata = 0;
            //     // flgUpdate = 1;                
            //     XK_USB_Reset(i);
            //     XK_USB_Scan_All();
            //     // LOG_E("USB", "USB%d Disconnected!!", i);
            //     flgFirst=1;
            // }
        }
    }
}

/*********************************************** 
 *  Application option parsing section
 *  Add app option 
************************************************/
void OptionParser(int argc, char *argv[]){
    
    int param_opt; 
    FILE *fMyPid = fopen(MYPID_PATH, "r");
    if(fMyPid == NULL) 
    {
        fMyPid = fopen(MYPID_PATH, "r");
        LOG_E("OptionParser","Can't read pid file, please command 'make install'");        
        XK_Exit();
    }

    char tmpCmdLine[100];
    int prePid_i=0;
    char preProcessName[MAX_PROCESS_NAME];
    char* tmpPtr;

    while( -1 !=( param_opt = getopt( argc, argv, "dwsrRtulafv")))
    {
        switch( param_opt)
        {
            case  'd'   :   
                pritnOpt = MAIN_OPT_DEBUG_V;
                printf("* What number of USB do you want see?? : ");
                scanf("%d", &debugDispUSBnum);
                LOG_I("option","Debug mode - /dev/ttyUSB%d is selected!!", debugDispUSBnum);
            break;

            case  'f'   : 
                gFlgForce = ON;
                LOG_I("option","Force Run");
                
                fgets(gPrePid_s, sizeof(gPrePid_s), fMyPid);
                fclose(fMyPid);

                sprintf(tmpCmdLine, "sudo kill -2 %s", gPrePid_s);
                prePid_i = atoi(gPrePid_s);
                GetProcessNameByPid(prePid_i, preProcessName);
                tmpPtr = strstr(preProcessName, "xksdk");
                // LOG_E("", "%s       %d", preProcessName,tmpPtr);
                if(tmpPtr != 0){
                    system(tmpCmdLine);
                    LOG_I("Force Run", "PID(%d) is killed\n", prePid_i);
                }
                else{
                    LOG_I("Force Run", "No existing process\n");
                }
            break;
            
            case  'w'   :  
                pritnOpt = MAIN_OPT_RAW;
                LOG_I("option","Debug mode - raw data is selected");
            break;
            
            // case  'h'   :  
            //     pritnOpt = MAIN_OPT_HELP;
            //     printf("%s", help);
            //     exit(1);
            // break;
            
            case  's'   :  
                XK_HTTPHandle->optDisp |= (0x1<<HTTP_DISP_OPT_SEND);
                LOG_I("option","Sending data will be displayed!!");
            break;

            case  'r'   :  
                XK_HTTPHandle->optDisp |= (0x1<<HTTP_DISP_OPT_RECV);
                LOG_I("option","Received data will be displayed!!");
            break;
            
            case  'R'   :  
                pritnHeap = ON;
                LOG_I("option","free-heap size will be displayed!!");
            break;

            // case  't'   :  
            //     send_A_type = ON;
            //     LOG_I("option","A-type data will be displayed!!");
            // break;

            case  'u'   :  
                pritnCpu = ON;
                LOG_I("option","CPU-usage will be displayed!!");
            break;

            // case  'l'   :  
            //     gFlgLogging = ON;
            //     LOG_I("option","Radar data will be saved!!");
            // break;
            
            // case  'a'   :  
            //     gFlgAutoReboot = ON;
            //     LOG_I("option","If It can't send massege to server, It will be rebooted!");
            // break;

            case  'v'   :  
                printf("\tSDK version: %s\n\tAPI version: %s\n", SDK_VERSION, API_VERSION);
            break;
            
            case  '?'   :  
                LOG_E("option","Invalid option: %c \n", optopt);
            break;

            default :   
            break;
        }
    }
}


char usbName[200][USB_DEV_PORT_MAX_SIZE];

int readDir(char * path)
{
    DIR *dir;
    int count=0;
    struct dirent *ent;
    dir = opendir (path);
    if (dir != NULL) {  
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            // printf ("%s\n", ent->d_name);
            strcpy(&usbName[count], ent->d_name);
            // printf ("%s\n", &name[count]);
            count++;
        }
        closedir (dir);
    } else {
         /* could not open directory */
         perror ("");
        return -1;
    }

    return count;
    
}

int parseDir(char * path, XK_USBinfo_t * usbInfo)
{
    int i;
    int hubCount=0;
    int cmmCount=0;
    char tmpChar=0;
    int charN = strlen(path);

    for(i=0;i<10;i++) usbInfo->hubNum[i] = 0;

    for(i=0;i<charN;i++){
        if (path[i] == '.'){
            cmmCount++;
            usbInfo->hubNum[hubCount++] = tmpChar-48;
        }
        else if(path[i] == ':'){
            break;
        }
        tmpChar = path[i];
    }
    usbInfo->hubNum[hubCount] = tmpChar-48;
    return hubCount;
}



int XK_USB_Scan_All(void){
        int i, j;
        int cntConnected=0;
        int fileN;
        int portN = 200;

        LOG_I("XK_USB_Scan_All","XK-USB Device scanning....");
        
        fileN = readDir("/sys/bus/usb/devices/");

        for(i=0;i<fileN;i++){
            // printf ("[%d]%s\n", i, &usbName[i]);
            portN = XK_USB_Get_Portnum(&usbName[i]);
            infoUSB[portN].portnum = portN;
            infoUSB[portN].hubN = parseDir(&usbName[i], &infoUSB[portN]);
            if(portN != 200){
                infoUSB[portN].busnum = XK_USB_Get_Busnum(infoUSB[portN].hubNum, infoUSB[portN].hubN);
                infoUSB[portN].devnum = XK_USB_Get_Devnum(infoUSB[portN].hubNum, infoUSB[portN].hubN);
            }
        }
        return cntConnected;
}


int XK_USB_Reset(int port){
        char path[200];
        int fd;
        int rc;
        LOG_E("XK_USB_Reset","USB cable problem found!");

        sprintf(path, USB_DEV_BUS_PATH);
        sprintf(&path[strlen(path)], "/%03d", infoUSB[port].busnum);
        sprintf(&path[strlen(path)], "/%03d", infoUSB[port].devnum);
        // sprintf(&path[strlen(path)], "/%03d", infoUSB_bySymlink[port].USBinfo.busnum);
        // sprintf(&path[strlen(path)], "/%03d", infoUSB_bySymlink[port].USBinfo.devnum);
        LOG_W("","%s",path);

        fd = open(path, O_WRONLY);
        if (fd < 0) {
                LOG_E("XK_USB_Reset","Error opening output file");
                return -1;
        }        
        usleep(1000);

        rc = ioctl(fd, USBDEVFS_RESET, 0);
        if (rc < 0) {
                LOG_E("XK_USB_Reset","Error in ioctl");
                close(fd);
                return -1;
        }
        usleep(1000);
        LOG_I("XK_USB_Reset","/dev/ttyUSB%d reset successful", port);

        close(fd);
        return 1;  
}

int XK_USB_Power_Reset(int port){
        char path[200];
        int fd;
        int rc;        
	    struct usbdevfs_ioctl usb_ioctl;

        sprintf(path, USB_DEV_BUS_PATH);
        sprintf(&path[strlen(path)], "/%03d", infoUSB[port].busnum);
        sprintf(&path[strlen(path)], "/%03d", infoUSB[port].devnum);
        // sprintf(&path[strlen(path)], "/%03d", infoUSB_bySymlink[port].USBinfo.busnum);
        // sprintf(&path[strlen(path)], "/%03d", infoUSB_bySymlink[port].USBinfo.devnum);
        LOG_W("","%s",path);

        fd = open(path, O_RDWR);
        if (fd < 0) {
                LOG_E("XK_USB_Power_Reset","Error opening output file");
                return -1;
        }        
        sleep(1);
        
        LOG_W("","Disconnect");
        
		usb_ioctl.ifno = 0;
		usb_ioctl.ioctl_code = USBDEVFS_DISCONNECT;
		usb_ioctl.data = NULL;
		rc = ioctl(fd, USBDEVFS_IOCTL, &usb_ioctl);
		if (rc == -1 && errno != ENODATA) {
			perror("Error in ioctl (USBDEVFS_DISCONNECT)");
			return 1;
		}

        // rc = ioctl(fd, USBDEVFS_DISCONNECT, 0);
        sleep(1);
        LOG_W("","Connect");

        
		usb_ioctl.ifno = 0;
		usb_ioctl.ioctl_code = USBDEVFS_CONNECT;
		usb_ioctl.data = NULL;
		rc = ioctl(fd, USBDEVFS_IOCTL, &usb_ioctl);
		if (rc == -1) {
			perror("Error in ioctl (USBDEVFS_CONNECT)");
			return 1;
		}

		printf("Bind-driver request sent to the kernel\n");
		// return 0;

        // rc = ioctl(fd, USBDEVFS_CONNECT, 0);

        if (rc < 0) {
                LOG_E("XK_USB_Power_Reset","Error in ioctl");
                close(fd);
                return -1;
        }
        sleep(1);

        LOG_I("XK_USB_Power_Reset","/dev/ttyUSB%d reset successful", port);

        close(fd);
        return 1;
}

void XK_Exit(void){
    char  c;
    LOG_I("SDK","Interrupt signal is detected! 'xksdk' process terminated!!");
    sem_close(gRunning);
    sem_unlink(XK_PROCESS_ID);
    exit(-1);
}


void IsThereDifference(XK_HTTPHandle_t *HTTPHandle, int port, int idxApp, int cur, int pre){
    int i;
    
    for (i = 0; i < gAppInfo[idxApp].appParamSZ; i++){       
        if( g_Radar_Data[cur].m_nALLRadarData[port][gAppInfo[idxApp].param[i].paramObjNum_V + PROTOCOL_INFO_MAX_SZ] != 
            g_Radar_Data[pre].m_nALLRadarData[port][gAppInfo[idxApp].param[i].paramObjNum_V + PROTOCOL_INFO_MAX_SZ]
        ) 
        {
            HTTPHandle->flgSIC = 1;
        }
    }
    
}

void CheckAppInfo(void){
    int i, j, k;
    
    for(i=0;i<APP_SIZE;i++){
        for(j=0;j<SEND_PARAM_SIZE_MAX;j++){
            if(gAppInfo[i].param[j].paramName[0] == 0){
                if(j != gAppInfo[i].appParamSZ){
                    LOG_E("CheckAppInfo", "Error App(%d): %s,  Parameter: %d in 'XK_RadarApplicationInfo.c'", i, gAppInfo[i].appName, j);
                }
                break;
            }
        }
    }
}



////////wifi reset
// #include <stdio.h>

// void main(void){

//         system("sudo ifconfig wlan0 down");
//         system("sudo ifconfig wlan0 up");

// }

// sudo teamviewer --daemon stop
// sudo teamviewer --daemon start

// sudo teamviewer --daemon restart

// sudo systemctl stop teamviewerd.service
// sudo systemctl start teamviewerd.service

// sudo systemctl restart teamviewerd.service


