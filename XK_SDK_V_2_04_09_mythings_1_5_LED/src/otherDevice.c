
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include "XK_tools.h"
#include "radar.h"
#include <string.h>
#include <time.h> 
#include <stdarg.h> 
#include <sched.h>
#include "UART.h"

#include "XK_CommonDefineSet.h"
#include "otherDevice.h"
#include "Plugin_falldetection.h"
#include "Plugin_inoutCounting.h"

#define CPU_N 2 // TODO: What number can it use.
#define OTHERD_DEBUG_ENABLE 0
#define OTHERD_DEBUG_ENABLE_LV2 0
#define MIN_DEVICE 1
#define CHECK_RATE 1
// #define UI_SEND_RATE FPS_WMFALL
#define SYNC_FLOAT_LEGACY 11.031507f
#define SYNC_FLOAT 4.28001f

static pthread_t otherDeviceHandle_Thread_t;

static int otherDevFunc[MAX_NUM_USB_DEVICE] = {0};
static int otherDevIndex[MAX_NUM_USB_DEVICE] = {0};
static int otherDev[MAX_NUM_USB_DEVICE] = {0};
static initDataSetOD_t od_init;
static connectPC_t cpc;

static void *otherDeviceHandler(void *data);
static int checkFunc(int pi);
static int deviceCheck();
// static int delayPeriod(struct timeval* WTimeT);

void *otherDeviceHandler(void *data)
{
    int nStatus = 0;
    int i, temp;
    int nControlMsg = 0;
    
    int res=0;
    unsigned long waitGETTime=0;
    long double a[4], b[4], loadavg;

    struct timeval WTimeT;
    struct timeval appTimeArr[MAX_NUM_USB_DEVICE];
    
    gettimeofday(&WTimeT, NULL);
    for(int ti=0; ti<MAX_NUM_USB_DEVICE;ti++){
        gettimeofday(&appTimeArr[ti], NULL);
    }
    
    cpu_set_t cpu2;
    
    CPU_ZERO(&cpu2);
    CPU_SET(CPU_N, &cpu2);
    temp = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu2);
    for (i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpu2))
        {
            // printf("CPU2: CPU %d\n", i);
        }
    }

    delay(2500);
    while(1)
    {   
        if(CheckRadarFuunc()<0) {
#if OTHERD_DEBUG_ENABLE
            printf("Waiting for Connection with Check the other Divce\n");
#endif
            delay(1000);
            continue;
        }

        deviceCheck();

        // delayPeriod(&WTimeT);

        for(int pi=0;otherDevIndex[pi]>=0;pi++){
            if(otherDevFunc[pi]==0 && delayPeriodCheck(&appTimeArr[pi],1000/fps_forCheck[pi])){
                checkFunc(pi);
        
            } else if(otherDevFunc[pi] == RADAR_APP_WMFALL && delayPeriodCheck(&appTimeArr[pi],1000/fps_forCheck[pi])){
#if OTHERD_DEBUG_ENABLE
                printf("It's Fall UI Section\n");
#endif
                char UIdata[sizeof(sendPCFallmsg_t)];
                makeUIFallResult(UIdata, 0); // TODO: Now, only first set of radar is connected with PC.
                write(cpc.fd,UIdata,sizeof(sendPCFallmsg_t));

                int len;
                unsigned char recvData[4096];

                if((len = read(cpc.fd,recvData,4096))>0){
                    delay(10);
                    int temp =len;
                    while((len += read(cpc.fd,&recvData[len],512))!=temp) temp = len;

                    float dataf[1024] = {0};
                    memcpy(dataf,recvData,4096);

#if OTHERD_DEBUG_ENABLE && 0
                    printf("%d:::\t",len);
                    for(int li=0;li<len;li++) printf("%4.2X",recvData[li]);
                    printf("\nloop\n");
#endif
#if OTHERD_DEBUG_ENABLE
                    printf("%d:::\t",len/4);
                    for(int li=0;li<len/4;li++) printf("%10f",dataf[li]);
                    printf("\nloop\n");
#endif
                    if(dataf[0] == SYNC_FLOAT_LEGACY){
                        // printf("Im IN!!\n");
                        cmd_fall_system_legacy(&dataf[1],0);
                    }
                }
            }else if(otherDevFunc[pi] == RADAR_APP_INOUT && delayPeriodCheck(&appTimeArr[pi],1000/fps_forCheck[pi])){
#if OTHERD_DEBUG_ENABLE
                printf("It's INOUT UI Section\n");
#endif
                char UIdata[2048];
                int Ln = makeUIInoutResult(UIdata, 0); // TODO: Now, only first set of radar is connected with PC.
                write(cpc.fd,UIdata,Ln);

                int len;
                unsigned char recvData[4096];

                if((len = read(cpc.fd,recvData,4096))>0){
                    delay(10);
                    int temp =len;
                    while((len += read(cpc.fd,&recvData[len],512))!=temp) temp = len;

                    float dataf[1024] = {0};
                    memcpy(dataf,recvData,4096);
                    
                    cmd_Inout_system_legacy(&dataf[0],0,len/4);
                }
            }
            delay(100);
        }
        // delay(500);
    }
}



// int otherDevHinit(stGetRadar_Data_t g_Radar_Data[],XK_UARTHandle_t XK_UARTHandle[], int nShearBufferIdx[],short usbStatus[]){
int otherDevHinit(XK_UARTHandle_t XK_UARTHandle[],short usbStatus[], pthread_t Get_XKRadar_Data_Thread_t[], XK_HTTPHandle_t *XK_HTTPHandle){

    for(int pi=0;pi<MAX_NUM_USB_DEVICE;pi++){
        fps_forCheck[pi] = CHECK_RATE; // TODO: It will be Changalbe.
    }
    od_init.UARTHandle_Temp = XK_UARTHandle;
    od_init.Data_Thread_t = Get_XKRadar_Data_Thread_t;
    // od_init.g_Data = g_Radar_Data;
    // od_init.nShearBufferIdx = nShearBufferIdx;
    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
        od_init.appNum[i] = &(XK_UARTHandle[i].appNum);
    }
    od_init.usbStatus = usbStatus;
    od_init.usbStatus_http = XK_HTTPHandle->USBStatus;
    
    int thrdCreate = pthread_create(&otherDeviceHandle_Thread_t, NULL, (void *)otherDeviceHandler, (void *)'a');
    if (pthread_detach(otherDeviceHandle_Thread_t) != 0) 
    {
        perror("pthread_detach(SendToRadarThread)\n");
    }

    // LOG_I("system", "Start a thread for Window GUI (WMFALL / Inout).");

    return 0;
}

int otherDevHDeinit(){
    pthread_cancel(otherDeviceHandle_Thread_t);
    
    int status;
    pthread_join(otherDeviceHandle_Thread_t,(void **)&status);
    return 0;
}


int CheckRadarFuunc(){
    int Cnt_N_otherD = 0;
    // int otherDevIndex[MAX_NUM_USB_DEVICE] = {0};
    int StateFlag = 0;
    int ChangeFlag = 0;

    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
        if(od_init.usbStatus[i]>=0 && *od_init.appNum[i] == OTHER_DEVICE){
#if OTHERD_DEBUG_ENABLE
            printf("Other Dev port is %2d and AppNum is %d...\n",i,*od_init.appNum[i]);
#endif
            if(otherDev[i] != 1) ChangeFlag = 1;
            
            otherDev[i] = 1;
            otherDevIndex[Cnt_N_otherD] = i;
            Cnt_N_otherD++;
        } else{
            if(otherDev[i] != 0) ChangeFlag = 1;
            otherDev[i] = 0;
        }
    }
    for (int i = Cnt_N_otherD; i < MAX_NUM_USB_DEVICE; i++){
            otherDevIndex[Cnt_N_otherD] = -1;
    }
    
#if OTHERD_DEBUG_ENABLE_LV2 // Just check the AppNum of other Device.
    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
        if(od_init.usbStatus[i]==0 && *od_init.appNum[i] != OTHER_DEVICE){
            printf("other port is %2d and AppNum is %d...\n",i,*od_init.appNum[i]);
        }
    }
#endif

    if(Cnt_N_otherD<MIN_DEVICE){
        StateFlag = -1;
        return StateFlag;
    }

    if(ChangeFlag == 0) return StateFlag;
    #if OTHERD_DEBUG_ENABLE
        printf("Radar Port has been changed...\n");
    #endif

    for(int ti=0; ti<Cnt_N_otherD;ti++){
        int pi = otherDevIndex[ti];
    #if OTHERD_DEBUG_ENABLE
        printf("Try to close the USB%d Thread...\n",pi);
    #endif
        char tempStr2[100];
        sprintf(tempStr2,"/dev/ttyUSB%d",pi);
        LOG_I(tempStr2, "A non-radar serial connection has been detected.");
        pthread_cancel(od_init.Data_Thread_t[pi]);
        od_init.usbStatus[pi] = 1; // for Sync with http_usbstatus..
        od_init.usbStatus_http[pi] = 1; // remove this port from HTTP Msg
        close(*(od_init.UARTHandle_Temp[pi].fd));
    }

    // TODO: What will be happen if the device is changed.
    
    return StateFlag;
}

int checkFunc(int pi){
    int StateFlag = 0;

    // for(int pi=0;otherDevIndex[pi]>=0;pi++){
        char tempStr[] = "/dev/ttyUSB%d";
        char tempStr2[100];
        sprintf(tempStr2,tempStr,otherDevIndex[pi]);

        short TempShort = -1;
        UART_init(&(cpc.fd), tempStr2, &TempShort);
        char strBuf[1024]={0};
        delay(750);
        read(cpc.fd,strBuf,1024);
        close(cpc.fd);
#if OTHERD_DEBUG_ENABLE
        printf("strbuf..\n%s\n",strBuf);
#endif

        if(strstr(strBuf,"XKUIFALL")){
#if OTHERD_DEBUG_ENABLE
            printf("Fall UI is detected..\n");
#endif
            // LOG_I(tempStr2, "Requested connection for WMFall GUI...");
            if(isEnable_fall()){
                otherDevFunc[pi] = RADAR_APP_WMFALL;
                TempShort = -1;
                UART_init(&(cpc.fd), tempStr2, &TempShort);
                write(cpc.fd,"XAKAGUI",strlen("XAKAGUI"));
                fps_forCheck[pi] = FPS_WMFALL;
                LOG_I(tempStr2, "Request for WMFall GUI... WMFall GUI connection complete.");
            }
            else{
#if OTHERD_DEBUG_ENABLE
                printf("But Fall radars are not exist..\n");
#endif
                write(cpc.fd,"NOFALL",strlen("NOFALL"));
                LOG_E("system", "Request for WMFall GUI... There is not the WMFall system.");
            }
        }

        if(strstr(strBuf,"XKUIINOUT")){
        // if(strstr(strBuf,"XGUI")){
#if OTHERD_DEBUG_ENABLE
            printf("INOUT UI is detected..\n");
#endif
            // LOG_I(tempStr2, "Requested connection for InOut GUI");
            if(isEnable_Inout()){
                otherDevFunc[pi] = RADAR_APP_INOUT;
                TempShort = -1;
                UART_init(&(cpc.fd), tempStr2, &TempShort);
                write(cpc.fd,"XGUI",strlen("XGUI"));
                fps_forCheck[pi] = CHECK_RATE;//FPS_INOUT;
                LOG_I(tempStr2, "Request for InOut GUI... InOut GUI connection complete.");
            }
            else{
#if OTHERD_DEBUG_ENABLE
                printf("But Inout radars are not exist..\n");
#endif
                write(cpc.fd,"NOINOUT",strlen("NOINOUT"));
                LOG_E("system", "Request for InOut GUI... There is not the InOut system.");
            }
        }
        
    // }

    return StateFlag;
}

// int delayPeriod(struct timeval* WTimeT){

//     long RefT = WTimeT->tv_sec*1000 + WTimeT->tv_usec/1000;
//     gettimeofday(WTimeT, NULL);
//     long CurT = WTimeT->tv_sec*1000 + WTimeT->tv_usec/1000 - RefT;
//     long periodT = 1000.0/fps_forCheck;
//     long delayT = periodT - (CurT);
//     // printf("%d    %d\n",(long)(1000.0/fps_forCheck),delayT);
//     if(delayT<0) delayT = 0;
//     if(delayT>periodT) delayT = periodT;
//     delay(delayT);
//     gettimeofday(WTimeT, NULL);

// #if OTHERD_DEBUG_ENABLE
//         printf("\nThread otherDevice at time: %ld ms\n",WTimeT->tv_sec*1000 + WTimeT->tv_usec/1000 - RefT);
// #endif

// }

int delayPeriodCheck(struct timeval* WTimeT,int RefTime){
    struct timeval TempTime;

    long RefT = WTimeT->tv_sec*1000 + WTimeT->tv_usec/1000;
    gettimeofday(&TempTime, NULL);
    long CurT = TempTime.tv_sec*1000 + TempTime.tv_usec/1000 - RefT;
    if(CurT<0) CurT = 0;
    if(RefTime<0) RefTime = 0;

    return (CurT>RefTime);
}

int deviceCheck(){
    for(int ppi=0;otherDevIndex[ppi]>=0;ppi++){
        int pi = otherDevIndex[ppi];
        char tempStr[] = "/dev/ttyUSB%d";
        char device[100];
        sprintf(device,tempStr,pi);
        if(!exists(device)){
            od_init.usbStatus[pi] = -1; // for Sync with http_usbstatus..
            od_init.usbStatus_http[pi] = -1; // remove this port from HTTP Msg
            *od_init.appNum[pi] = 0;
            otherDevFunc[ppi] = 0;
            close(cpc.fd);
            LOG_I(device, "The connection has been closed",device);
#if OTHERD_DEBUG_ENABLE
            printf("PC connection in USB%d is closed\n",pi);
#endif
        }
    }
}