
#define _GNU_SOURCE
#include <stdio.h>
#include "XK_tools.h"
#include "radar.h"
#include <string.h>
#include <time.h> 
#include <stdarg.h> 
#include <sched.h>
#include "UART.h"

#include "XK_CommonDefineSet.h"
#include "Plugin_falldetection.h"
// #include "httpClient.h"
#include "Plugin_falldetection_type.h"

#define SEND2R_BUFF_SIZE RECV_BUFF_SIZE
#define CPU_N 1 // TODO: What number can it use.
#define MYTIME_PER_SEC (1000000)
// #define FALL_DEBUG_ENABLE 0
#if FALL_DEBUG_ENABLE
#define VEIW_N 99 // 2: result data / 1: 2 + send2radar packet / 0: 1 + 2 + recieved relay data from radar //99: all parameter
#define FALL_DEBUG_ENABLE_LV2 0
static int justCheck(int nn);
#else
#define FALL_DEBUG_ENABLE_SIMPLE 0
#endif

#define FALL_LOG_ENABLE 0

static pthread_t SendToRadar_Thread_t;
static pthread_mutex_t SendRadar_lock;

static void *SendToRadarThread(void *data);
static int CheckRadarFunc(void);
static unsigned long mytime(struct timeval* inputT, struct timeval* refT);
static int mytimeAdd(struct timeval* inputT, const int fps);
static unsigned long mytimecheck(struct timeval* inputT);
static int SaveData(int Nset);
static int forLEDctrl(int Nset);
static float get_fstatus(int Nset);

static initDataSet_t D_init;
static result_t Res[WMFALL_MAX_N];
static int fps_wmfall;
static int IswmFall[MAX_NUM_USB_DEVICE] = {0};

static struct timeval ZTime;

void *SendToRadarThread(void *data)
{
    int nStatus = 0;
    int i, temp;
    int nControlMsg = 0;
    
    int res=0;
    unsigned long waitGETTime=0;
    unsigned long CheckwaitGETTime=0;
    long double a[4], b[4], loadavg;
    
    struct timeval RTime;
    struct timeval RTimeT;
    struct timeval WTime;
    struct timeval WTimeT;
    
    gettimeofday(&ZTime, NULL);
    gettimeofday(&RTime, NULL);
    gettimeofday(&RTimeT, NULL);
    gettimeofday(&WTime, NULL);
    gettimeofday(&WTimeT, NULL);
    
    cpu_set_t cpu2;
    
    CPU_ZERO(&cpu2);
    CPU_SET(CPU_N, &cpu2);
    temp = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu2);
    for (i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpu2))
        {
            // printf("CPU1: CPU %d\n", i);
        }
    }

    delay(3000);
    waitGETTime = mytime(&WTime,&RTime);
    CheckwaitGETTime = mytime(&WTimeT,&RTimeT);
    while(1)
    {   
        // if(CheckwaitGETTime>3*MYTIME_PER_SEC){
        //     mytimeAdd(&RTimeT,1);
            if(CheckRadarFunc()<0) {
#if FALL_DEBUG_ENABLE
                printf("Waiting for Connection with WMFall\n");
#endif
                delay(3000);
                // waitGETTime = clock();
                gettimeofday(&RTime, NULL);
                continue;
            }
        // }
        int TempTime = ((MYTIME_PER_SEC)/fps_wmfall - mytime(&WTime,&RTime));

#if FALL_DEBUG_ENABLE_SIMPLE
        printf("%d\t %d\t",TempTime,mytime(&WTime,&ZTime));
        int Bindx = (*Res[0].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
        printf("%10.2f\n",Res[0].r[0].data[Bindx][Para11]);
#endif
        (TempTime>0)? delay(TempTime/1000):1;
        while(mytime(&WTime,&RTime) < (MYTIME_PER_SEC)/fps_wmfall);

        mytimeAdd(&RTime,fps_wmfall);
#if FALL_DEBUG_ENABLE
        printf("\nThread Send2Radar at time: %ld ms // %ld ms\n",mytime(&WTimeT,&ZTime)/1000,mytimecheck(&RTime));
#endif
#if FALL_DEBUG_ENABLE
            justCheck(VEIW_N);
#endif
#if FALL_LOG_ENABLE
        SaveData(0);
#endif
        forLEDctrl(0);

        pthread_mutex_lock(&SendRadar_lock);
        for (int nsi = 0; nsi < D_init.NumSystem; nsi++){
            if(!getNoData_fall(nsi)){
                int TempI = 0;
                Res[nsi].Send2Rbuf[TempI++] = 404.3333;

                for(int rri=0;rri<NUM_RADAR_FALL;rri++){
                    int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    for(unsigned int spi=0;spi<sizeof(SendtoRadarP)/4;spi++){
                        Res[nsi].Send2Rbuf[TempI++] = Res[nsi].r[rri].data[Bindx][SendtoRadarP[spi]];
                    }
                }
                Res[nsi].Send2Rbuf[TempI++] = 4.28428;
                Res[nsi].Send2Rbuf[TempI++] = 255.255;

                Res[nsi].msglen = TempI*4;

                for(int rri=0;rri<NUM_RADAR_FALL;rri++){
                    write(*(Res[nsi].r[rri].fd),(char*)(Res[nsi].Send2Rbuf),Res[nsi].msglen);
                }
            }
        }
        pthread_mutex_unlock(&SendRadar_lock);
    } // while(1)
}





int wmFallinit(stGetRadar_Data_t g_Radar_Data[],XK_UARTHandle_t XK_UARTHandle[], int nShearBufferIdx[],short usbStatus[]){

    fps_wmfall = FPS_WMFALL; // TODO: It will be Changalbe.

    D_init.UARTHandle_Temp = XK_UARTHandle;
    D_init.g_Data = g_Radar_Data;
    D_init.nShearBufferIdx = nShearBufferIdx;
    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
        D_init.appNum[i] = &(XK_UARTHandle[i].appNum);
    }
    D_init.usbStatus = usbStatus;

    D_init.NumSystem = 0;

    pthread_mutex_init(&SendRadar_lock, NULL);

    int thrdCreate = pthread_create(&SendToRadar_Thread_t, NULL, (void *)SendToRadarThread, (void *)'a');
    if (pthread_detach(SendToRadar_Thread_t) != 0) 
    {
        perror("pthread_detach(SendToRadarThread)\n");
    }

    return 0;
}


int wmFallDeinit(){
    pthread_mutex_destroy(&SendRadar_lock);
    pthread_cancel(SendToRadar_Thread_t);
    int status;
    pthread_join(SendToRadar_Thread_t,(void **)&status);

    return 0;
}




int CheckRadarFunc(){
    int Cnt_N_wmfall = 0;
    int IswmFallIndex[MAX_NUM_USB_DEVICE] = {0};
    int StateFlag = 0;
    int ChangeFlag = 0;

    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
    #if USE_LEGACY_APPNUM
        if(D_init.usbStatus[i]==0 && (*D_init.appNum[i] == FallDetection_WallMount || *D_init.appNum[i] == 8) && D_init.UARTHandle_Temp[i].cntNodata<5)
    #else
        if(D_init.usbStatus[i]==0 && *D_init.appNum[i] == FallDetection_WallMount && D_init.UARTHandle_Temp[i].cntNodata<5)
    #endif
        {
#if FALL_DEBUG_ENABLE_LV2
            printf("Fall radar port is %2d and AppNum is %d...\n",i,*D_init.appNum[i]);
#endif
            if(IswmFall[i] != 1) ChangeFlag = 1;
            
            IswmFall[i] = 1;
            IswmFallIndex[Cnt_N_wmfall] = i;
            Cnt_N_wmfall++;
        } else{
            if(IswmFall[i] != 0) ChangeFlag = 1;
            IswmFall[i] = 0;
        }
    }
#if FALL_DEBUG_ENABLE_LV2
    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
    #if USE_LEGACY_APPNUM
        if(D_init.usbStatus[i]==0 && (*D_init.appNum[i] == FallDetection_WallMount || *D_init.appNum[i] == 8))
    #else
        if(D_init.usbStatus[i]==0 && *D_init.appNum[i] != FallDetection_WallMount)
    #endif
        {
            printf("other port is %2d and AppNum is %d...\n",i,*D_init.appNum[i]);
        }
    }
#endif
    for (int i = Cnt_N_wmfall; i < MAX_NUM_USB_DEVICE; i++){
            IswmFallIndex[Cnt_N_wmfall] = -1;
    }

    if(Cnt_N_wmfall<NUM_RADAR_FALL){
        StateFlag = -1;
    }

    if(ChangeFlag == 0) return StateFlag;
    #if FALL_DEBUG_ENABLE
        printf("Radar Port has been changed...\n");
    #endif
    
    // for(int pi=0;pi<Cnt_N_wmfall;pi++){
    //     int Bindx = (D_init.nShearBufferIdx[pi]-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    //     int chckid = D_init.g_Data[Bindx].m_nALLRadarData[pi][Para0]; // TODO: Check how to pair for each radar.
    //     char tempStr2[100];
    //     sprintf(tempStr2,"/dev/ttyUSB%d",pi);
    //     LOG_I(tempStr2, "A WMFall radar (ID:%d) has been detected.",chckid);
    // }

    if(Cnt_N_wmfall<NUM_RADAR_FALL){
        StateFlag = -1;
        LOG_E("system", "The WMFall radar has been detected, but Not enough the radars.");
        return StateFlag;
    }
    
    int N_wmfall = 0;
    int ArrRID[MAX_NUM_USB_DEVICE] = {0};
    int ArrRID_port[MAX_NUM_USB_DEVICE] = {0};

    // Sort the Radar ID.
    for (int ci = 0; ci < Cnt_N_wmfall; ci++)
    {
        int i = IswmFallIndex[ci];
        // int chckid = D_init.UARTHandle_Temp[i].radarID;
        int Bindx = (D_init.nShearBufferIdx[i]-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
        int chckid = D_init.g_Data[Bindx].m_nALLRadarData[i][PAIR_N];
        int CK_port = i;
        for (int cci = 0; cci < Cnt_N_wmfall; cci++)
        {
            if(ArrRID[cci] <= chckid){
                int temp = ArrRID[cci];
                ArrRID[cci] = chckid;
                chckid = temp; 

                temp = ArrRID_port[cci];
                ArrRID_port[cci] = CK_port;
                CK_port = temp;
            }
        }
    }
    
#if FALL_DEBUG_ENABLE_LV2
    for (int ci = 0; ci < Cnt_N_wmfall; ci++) {
        printf("%6d/%d",ArrRID[ci],ArrRID_port[ci]);
    }
    printf("\n");
#endif
    if(Cnt_N_wmfall > NUM_RADAR_FALL){
        // Check the No of pair.
        int ccic;
        for (ccic = 0; ccic < Cnt_N_wmfall-1; ccic++){
            if(ArrRID[ccic] != ArrRID[ccic+1]){
                int TEMP_arr[MAX_NUM_USB_DEVICE] = {0};
                int TEMP_arr2[MAX_NUM_USB_DEVICE] = {0};
                memcpy(TEMP_arr, &ArrRID_port[ccic+1],(Cnt_N_wmfall-1)*4);
                memcpy(&ArrRID_port[ccic], TEMP_arr,(Cnt_N_wmfall-1)*4);
                memcpy(TEMP_arr2, &ArrRID[ccic+1],(Cnt_N_wmfall-1)*4);
                memcpy(&ArrRID[ccic], TEMP_arr2,(Cnt_N_wmfall-1)*4);
                Cnt_N_wmfall--;
                ccic--;
            }
            else{
                ccic++;
            }
        }
        if(ccic != Cnt_N_wmfall) Cnt_N_wmfall--;

#if FALL_DEBUG_ENABLE_LV2
        for (int ci = 0; ci < Cnt_N_wmfall; ci++) {
            printf("%6d/%d",ArrRID[ci],ArrRID_port[ci]);
        }
        printf("\n");
#endif
    }
    
    D_init.NumSystem = Cnt_N_wmfall/2;
    if(D_init.NumSystem>WMFALL_MAX_N){
        D_init.NumSystem = WMFALL_MAX_N;
    }
    #if FALL_DEBUG_ENABLE_LV2
        printf("There are %d set of wmfall radar...\n", D_init.NumSystem);
    #endif
    
    // TODO: It has a pair as order of ID.
    int totalCnt = 0;
    for (int nsi = 0; nsi < D_init.NumSystem; nsi++)
    {
        for (int ci = 0; ci < NUM_RADAR_FALL; ci++)
        {
            int i = ArrRID_port[totalCnt++];
            for (int ij=0;ij<GETDATA_BUFFER_NUMBER;ij++){
                Res[nsi].r[ci].data[ij] = D_init.g_Data[ij].m_nALLRadarData[i];
            }
            Res[nsi].r[ci].BufferIdx = &D_init.nShearBufferIdx[i];
            Res[nsi].r[ci].fd = D_init.UARTHandle_Temp[i].fd;
            Res[nsi].r[ci].NumData = D_init.UARTHandle_Temp[i].rcvDataSize;
            Res[nsi].r[ci].port = i;
            Res[nsi].r[ci].RadarID = D_init.UARTHandle_Temp[i].radarID;
            int Bindx = (D_init.nShearBufferIdx[i]-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
            Res[nsi].r[ci].NumPair = D_init.g_Data[Bindx].m_nALLRadarData[i][PAIR_N];
            Res[nsi].r[ci].cntNodata = &D_init.UARTHandle_Temp[i].cntNodata;
        }
        LOG_I("system", "The WMFall system #%d (radar ID %d & %d) is started.",nsi,Res[nsi].r[0].RadarID,Res[nsi].r[1].RadarID);
    }

    return StateFlag;
}




#if FALL_DEBUG_ENABLE

int justCheck(int nn){
    #if FALL_DEBUG_ENABLE_LV2
        printf("In the justCheck\n");
    #endif
    for (int nsi = 0; nsi < D_init.NumSystem; nsi++){
        printf("-------------------------------------------------------------\n");
        switch (nn){
            case 0:
                for (int rri = 0; rri < NUM_RADAR_FALL; rri++)
                {
                    int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    for(int spi = 0;spi<sizeof(SendtoRadarP)/4;spi++){
                            printf("%10.2f",Res[nsi].r[rri].data[Bindx][SendtoRadarP[spi]]);
                    }
                    printf("\n-------------------------------------------------------------\n");
                }

            case 1:{
                int spi = 0;
                printf("%10.2f",Res[nsi].Send2Rbuf[spi++]);
                printf("\n");
                for(;spi<sizeof(SendtoRadarP)/4 + 1;spi++){
                    printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                }
                printf("\n");
                for(;spi<(sizeof(SendtoRadarP)/4)*2 + 1;spi++){
                    printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                }
                printf("\n");
                printf("%10.2f",Res[nsi].Send2Rbuf[spi++]);
                printf("%10.2f",Res[nsi].Send2Rbuf[spi++]);
                printf("\n-------------------------------------------------------------\n");
                
                printf("\n");
                for(spi = 0;spi<RELAY_MSG_LEN;spi++){
                    printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                }
                printf("\n-------------------------------------------------------------\n");
            }

            case 2:
                for (int rri = 0; rri < NUM_RADAR_FALL; rri++)
                {
                    printf("Np%d:P%d:I%d:f%d::",Res[nsi].r[rri].NumPair,Res[nsi].r[rri].port,Res[nsi].r[rri].RadarID,*(Res[nsi].r[rri].fd));
                    int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    for(int spi = 0;spi<sizeof(ResRadarP)/4;spi++){
                            printf("%10.2f",Res[nsi].r[rri].data[Bindx][ResRadarP[spi]]);
                    }
                    printf("\n-------------------------------------------------------------\n");
                }
            default:
                break;
            case 98: 
                for(int spi = 0;spi<RELAY_MSG_LEN;spi++){
                    printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                }
                printf("\n-------------------------------------------------------------\n");
                break;
            case 99:
                for (int rri = 0; rri < NUM_RADAR_FALL; rri++)
                {
                    int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    for(int spi = 0;spi<NUM_PARA_ADD;spi++){
                            printf("%10.2f",Res[nsi].r[rri].data[Bindx][spi]);
                            if(spi%5==5-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n-------------------------------------------------------------\n");
                }
                break;
        }
    }

    return 0;
}
#endif


unsigned long mytime(struct timeval* inputT, struct timeval* refT){

    gettimeofday(inputT, NULL);
    unsigned long Sec = inputT->tv_sec;
    unsigned long uSec = inputT->tv_usec;
    
    unsigned long SecR = refT->tv_sec;
    unsigned long uSecR = refT->tv_usec;

    if(uSecR > uSec) {
        uSec += 1000000;
        Sec -= 1;
    }

    return (Sec - SecR)*1000000 + (uSec - uSecR);
}

unsigned long mytimecheck(struct timeval* inputT){

    unsigned long Sec = inputT->tv_sec;
    unsigned long uSec = inputT->tv_usec;
    
    unsigned long SecR = ZTime.tv_sec;
    unsigned long uSecR = ZTime.tv_usec;

    if(uSecR > uSec) {
        uSec += 1000000;
        Sec -= 1;
    }

    return (Sec - SecR)*1000000 + (uSec - uSecR);
}

int mytimeAdd(struct timeval* inputT, const int fps){

    unsigned long* Sec = &(inputT->tv_sec);
    unsigned long* uSec = &(inputT->tv_usec);

    *uSec += MYTIME_PER_SEC/fps;

    if(MYTIME_PER_SEC < *uSec) {
        *uSec -= MYTIME_PER_SEC;
        *Sec += 1;
    }

    return 0;
}

void makeJsonFallResult(char * data, int Nset){

    msg_Result_data_t r2h_d;

    float para[100] = {0};

    int Bindx = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int SelR = Res[Nset].r[0].data[Bindx][Para6];

    
    Bindx = (*Res[Nset].r[SelR].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    for(unsigned int spi=0;spi<sizeof(forServerMsg)/4;spi++){
        para[spi] = Res[Nset].r[SelR].data[Bindx][forServerMsg[spi]];
    }

    r2h_d.a36_Fall_Flag = 100;
    r2h_d.F_res_Final = para[0];//
    r2h_d.F_res_Final = get_fstatus(0);//
    r2h_d.br_res = para[1];//
    r2h_d.bst_res = para[2];//
    r2h_d.hr_res = para[3];//
    r2h_d.hst_res = para[4];//
    r2h_d.mvi = para[5];//
    r2h_d.Pres_res = para[6];//
    r2h_d.testdata = 1973;
    r2h_d.position = para[7];
    r2h_d.NoMVPeriod = para[8];//
    r2h_d.f_cursor = para[9];//
    r2h_d.vitalScore = para[10];//

            
////////////////////////////////////////////////////////////////////////////
    { //msg start
        char TEMPSTR[] =",\r\n\t\t\"Num100\":\"%0.2f\""
                        ",\r\n\t\t\"f_status\":\"%0.2f\""
                        ",\r\n\t\t\"breathing_rate\":\"%0.2f\""
                        ",\r\n\t\t\"breathing_score\":\"%0.2f\""
                        ",\r\n\t\t\"heart_rate\":\"%0.2f\""
                        ",\r\n\t\t\"heart_score\":\"%0.2f\""
                        ",\r\n\t\t\"movement\":\"%0.2f\""
                        ",\r\n\t\t\"presence\":\"%0.2f\""
                        ",\r\n\t\t\"testdata\":\"%0.2f\""
                        ",\r\n\t\t\"position\":\"%0.2f\""
                        ",\r\n\t\t\"NoMVPeriod\":\"%0.2f\""
                        ",\r\n\t\t\"movement_score\":\"%0.2f\""
                        ",\r\n\t\t\"dataSeq\":\"%0.2f\""
                        ;
        sprintf(data,TEMPSTR,
                            r2h_d.a36_Fall_Flag,
                            r2h_d.F_res_Final,
                            r2h_d.br_res,
                            r2h_d.bst_res,
                            r2h_d.hr_res,
                            r2h_d.hst_res,
                            r2h_d.mvi,
                            r2h_d.Pres_res,
                            r2h_d.testdata,
                            r2h_d.position,
                            r2h_d.NoMVPeriod,
                            r2h_d.vitalScore,
                            r2h_d.f_cursor
                            );
    }
////////////////////////////////////////////////////////////////////////////
}

int getNumofSet_fall(){
    return D_init.NumSystem;
}

int isEnable_fall(){
    return (D_init.NumSystem>0);
}

int getNoData_fall(int Nset){

    int ResInt = 0;

    for(int ri=0;ri<NUM_RADAR_FALL;ri++){
        if(*Res[Nset].r[ri].cntNodata>1){
            ResInt = 1;
        }
    }
    return 0;
}


float getRadarInfo_fall(int infoN, int numR, int Nset){

    float res = -99;
    int Bindx = (*Res[Nset].r[numR].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    switch(infoN){
        case GET_INFO_F_ID: // Radar ID
            res = (float)Res[Nset].r[numR].RadarID;
            break;
        case GET_INFO_F_SERIAL1:
            res = Res[Nset].r[numR].data[Bindx][SERIAL_1];
            break;
        case GET_INFO_F_SERIAL2:
            res = Res[Nset].r[numR].data[Bindx][SERIAL_2];
            break;
        case GET_INFO_F_SERIAL_USER1:
            res = Res[Nset].r[numR].data[Bindx][SERIAL_USER1];
            break;
        case GET_INFO_F_SERIAL_USER2:
            res = Res[Nset].r[numR].data[Bindx][SERIAL_USER2];
            break;
        case GET_INFO_F_PORT:
            res = (float)Res[Nset].r[numR].port;
            break;
        case GET_INFO_F_NODATA:
            res = (float)(*Res[Nset].r[numR].cntNodata>1);
            break;
        case GET_INFO_F_SELR:
            res = Res[Nset].r[numR].data[Bindx][Para6];
            break;
        default:
            break;
    }

    return res;
}



void makeUIFallResult(char * data, int Nset){

    // msg_Result_data_t r2h_d;
    rserial_t* r = Res[Nset].r;
    sendPCFallmsg_t sendmsg_temp;
    sendPCFallmsg_t* sendmsg = &sendmsg_temp;

    float para[NUM_RADAR_FALL][100] = {0};

    int Bindx = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int BindxA[2];
    BindxA[0] = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    BindxA[1] = (*Res[Nset].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int SelR = Res[Nset].r[0].data[Bindx][Para6];

    Res[Nset].RRadius = 99.9;
    for(int rii=0; rii<2;rii++){
        Bindx = BindxA[rii];
        float TEMPFFF = r[rii].data[Bindx][ResUI16]*r[rii].data[Bindx][ResUI16];
        TEMPFFF -= r[rii].data[Bindx][R_HEIGHT]*r[rii].data[Bindx][R_HEIGHT];
        r[rii].EachR = sqrtf(TEMPFFF);

        if(Res[Nset].RRadius>r[rii].EachR) Res[Nset].RRadius = r[rii].EachR;
    }
    
    float RemainingTime = -1;
    
    if(r[0].data[BindxA[0]][ResUI15] == 4 || r[1].data[BindxA[1]][ResUI15] == 4){
        if(r[0].data[BindxA[0]][ResUI14] < r[1].data[BindxA[1]][ResUI14])
        {
            RemainingTime = r[0].data[BindxA[0]][ResUI14];
        }else{
            RemainingTime = r[1].data[BindxA[1]][ResUI14];
            
        }
    }

    Bindx = BindxA[SelR];
    memcpy(sendmsg->header, "XAKA", 4);
    // sendmsg->f_status = r[SelR].data[Bindx][ResUI0];
    sendmsg->f_status = get_fstatus(0);
    sendmsg->breathingRate = r[SelR].data[Bindx][ResUI1];
    sendmsg->breathingRateFlag = r[SelR].data[Bindx][ResUI2];
    sendmsg->heartRate = r[SelR].data[Bindx][ResUI3];//
    sendmsg->heartRateFlag = r[SelR].data[Bindx][ResUI4];//
    sendmsg->movement = r[SelR].data[Bindx][ResUI5];//
    sendmsg->humanPresence = r[SelR].data[Bindx][ResUI6];//
    sendmsg->noMovementTime = r[SelR].data[Bindx][ResUI7];//
    sendmsg->f_cursor = r[SelR].data[Bindx][ResUI8];//
    sendmsg->radius = Res[Nset].RRadius;//
    sendmsg->led_on_off = r[SelR].data[Bindx][ResUI9];//
    sendmsg->sensitivityAnalog = r[SelR].data[Bindx][ResUI10];//
    sendmsg->autoSensitivity_on_off = r[SelR].data[Bindx][ResUI11];//
    sendmsg->autoSensitivityIndex1 = r[0].data[BindxA[0]][ResUI12];
    sendmsg->autoSensitivityIndex2 = r[1].data[BindxA[1]][ResUI12];//
    sendmsg->autoSensitivityRemainingTime = RemainingTime;//
    sendmsg->Rheight[0] = r[0].data[BindxA[0]][R_HEIGHT];//
    sendmsg->Rheight[1] = r[1].data[BindxA[1]][R_HEIGHT];//
    // sendmsg->ID[0] = r[0].data[BindxA[0]][ResUI13];//
    // sendmsg->ID[1] = r[1].data[BindxA[1]][ResUI13];//
    sendmsg->ID[0] = r[0].data[BindxA[0]][Para12];// Change the parameter ID -> Vital Score
    sendmsg->ID[1] = r[1].data[BindxA[1]][Para12];//
    memcpy(sendmsg->ender, "ENDX", 4);

    memcpy(data,sendmsg,sizeof(sendPCFallmsg_t));

}


int cmd_fall_system_legacy(float RRxdata[],int Nset){
    
    pthread_mutex_lock(&SendRadar_lock);
    delay(1000);
    rserial_t* r = Res[Nset].r;
    int BindxA[2];
    BindxA[0] = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    BindxA[1] = (*Res[Nset].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    // printf("\nReceive: %.2f %.2f\n",RRxdata[0],RRxdata[1]);

    const float head_f = 1010.1010;
    const float end_f = 255.255;
    
    float cmdArr_after[1024] = {0};
    float cmdArr[1024] = {0};

    char CMD_name[50] = {0};

    if(RRxdata[0] - ((int)RRxdata[0])!=0){
        char eeech[100];
        sprintf(eeech,"Error data Received.. CMD: %.2f // %.2f\n",RRxdata[0],RRxdata[1]);
        LOG_E("PC_UI_FALL",eeech);
        return -1;
    }

    float TempEachEndR[2] = {0};

    if(RRxdata[0] == CMD_F_UI_END_RANGE){
        for(int rii=0; rii<NUM_RADAR_FALL;rii++){
            int Bindx = BindxA[rii];
            float TEMPFFF = r[rii].data[Bindx][R_HEIGHT]*r[rii].data[Bindx][R_HEIGHT];
            TEMPFFF += RRxdata[1]*RRxdata[1];
            TempEachEndR[rii] = sqrtf(TEMPFFF);
        }
    }
    
    int cursor = 0,cursor_a = 0;
    cmdArr[cursor++] = head_f;
    cmdArr_after[cursor_a++] = head_f;

    for(int rri=0;rri<NUM_RADAR_FALL;rri++){
        int fd_t = *(Res[Nset].r[rri].fd);
        // printf("In for loop %d %d\n",rri, fd_t);
        cursor = 1;
        cursor_a = 1;

        if(RRxdata[0] == (float)CMD_F_UI_DEFAULT_SET){
            cursor = 0;
            cursor_a = 0;

            cmdArr[cursor++] = 100100;
            cmdArr_after[cursor_a++] = -1;

            write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
            sprintf(CMD_name,"CMD DEFAULT");
        }
        else if(RRxdata[0] == (float)CMD_F_UI_END_RANGE){
            cursor_a = 0;

            cmdArr[cursor++] = 2;
            cmdArr[cursor++] = TempEachEndR[rri];
            cmdArr[cursor++] = end_f;
            cmdArr[cursor++] = end_f;

            cmdArr_after[cursor_a++] = -1;

            write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
            sprintf(CMD_name,"CMD END RANGE, val: %.2f",RRxdata[1]);
        }
        else if(RRxdata[0] == (float)CMD_F_UI_LED){
            cmdArr[cursor++] = 4;
            cmdArr[cursor++] = RRxdata[1];
            cmdArr[cursor++] = end_f;
            cmdArr[cursor++] = end_f;

            write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
            sprintf(CMD_name,"CMD LED, val: %.2f",RRxdata[1]);
        }
        else if(RRxdata[0] == (float)CMD_F_UI_CALIBRATION){
            cursor = 0;
            cmdArr[cursor++] = 5005;
            cmdArr[cursor++] = RRxdata[1];
            
            write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
            sprintf(CMD_name,"CMD CALIBRATION, val: %.2f",RRxdata[1]);
        }
        else if(RRxdata[0] == (float)CMD_F_UI_AUTO_S_ONOFF){
            cmdArr[cursor++] = 6;
            cmdArr[cursor++] = RRxdata[1];
            cmdArr[cursor++] = end_f;
            cmdArr[cursor++] = end_f;

            write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
            sprintf(CMD_name,"CMD AUTO SENS ONOFF, val: %.2f",RRxdata[1]);
        }
        else if(RRxdata[0] == (float)CMD_F_UI_RADAR_H_1 || RRxdata[0] == (float)CMD_F_UI_RADAR_H_2){
            if(rri == RRxdata[0] - CMD_F_UI_RADAR_H_1){
                cursor = 0;
                cmdArr[cursor++] = 408.3333;
                cmdArr[cursor++] = 3;
                cmdArr[cursor++] = RRxdata[1];
                cmdArr[cursor++] = end_f;

                write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
                sprintf(CMD_name,"CMD CHANGE RADAR%d H, val: %.2f",rri,RRxdata[1]);
            }
        }
        else if(RRxdata[0] == (float)CMD_F_UI_CHG_ID_BOTH1 || RRxdata[0] == (float)CMD_F_UI_CHG_ID_BOTH2){
            if(rri == RRxdata[0] - CMD_F_UI_CHG_ID_BOTH1){
                cmdArr[cursor++] = 0;
                cmdArr[cursor++] = RRxdata[1];
                cmdArr[cursor++] = end_f;
                cmdArr[cursor++] = end_f;

                write(fd_t,(char*)(&cmdArr),cursor*sizeof(float));
                sprintf(CMD_name,"CMD CHANGE RADAR%d ID, val: %.2f",rri,RRxdata[1]);
            }
        }else {
            char atat[200];
            sprintf(atat,"Unexpected CMD... CMD:%.2f\t%.2f\n",RRxdata[0],RRxdata[1]);
            LOG_E("PC_UI_FALL",atat);
        }
    }

    
    if(RRxdata[0] == (float)CMD_F_UI_DEFAULT_SET || RRxdata[0] == (float)CMD_F_UI_END_RANGE){
        delay(3000);
        for(int rri=0;rri<NUM_RADAR_FALL;rri++){
            int fd_t = *(Res[Nset].r[rri].fd);

            write(fd_t,(char*)(&cmdArr_after),cursor_a*sizeof(float));
        }
    }

    // printf("%s\n",CMD_name);
    LOG_I("PC_UI_FALL",CMD_name);

    delay(500);
    pthread_mutex_unlock(&SendRadar_lock);
    return 0;
}

#define P_VERSION 19
extern struct tm gLocalTime;

int SaveData(int Nset){
    int ri;
    
    rSaveData_t rr[2];
    fSaveData_t rres;
    rserial_t* r = Res[Nset].r;
    int BindxA[2];
    BindxA[0] = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    BindxA[1] = (*Res[Nset].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    for(ri=0;ri<NUM_RADAR_FALL;ri++){
        rr[ri].fd = r[ri].fd;
        rr[ri].port = r[ri].port;
        rr[ri].cursor = 1;
        rr[ri].status = 1;
        int version = (int) r[ri].data[BindxA[ri]][P_VERSION];
        sprintf(rr[ri].func_ver,"FALL_%d_%d_%d",(version/10000)%100,(version/100)%100,version%100);
        rr[ri].justSpace = 1;
        rr[ri].RadarID = r[ri].RadarID;
        rr[ri].NumData = r[ri].NumData;
        rr[ri].DataRefreshF = 1;
        rr[ri].timeout = 1;
        rr[ri].timeout_cnt = 1;
        for(int pi=0;pi<62;pi++){
            rr[ri].para[pi] = r[ri].data[BindxA[ri]][pi+3];
        }
        rr[ri].EachR = r[ri].EachR;
    }

    rres.Radar1 = r[0].data[BindxA[0]][Para6];
    rres.Radar2 = r[0].data[BindxA[0]][Para6+1];
    rres.SelR = rres.Radar1;
    rres.justSpace = 1;
    rres.msglen = Res[Nset].msglen;
    rres.RRadius =Res[Nset].RRadius;

    char path[100]={0};
    sprintf(path,PATH_SAVE);

    time_t curr;
    struct tm *d;
    curr=time(NULL);
    d = &gLocalTime;

    for(ri=0;ri<NUM_RADAR_FALL;ri++){
        if(r[ri].RadarID>=0){
            char FilePath[200];
            rr[ri].currTime = curr;
            sprintf(FilePath,"%sdata/Rdata_%d_p%d_%d.xkd",path,r[ri].RadarID,r[ri].port,
                                                        (d->tm_year+1900)*10000 + (d->tm_mon+1)*100 + (d->tm_mday));

            char DataConvert[sizeof(rSaveData_t)]={0};
            memcpy(DataConvert,&rr[ri],sizeof(rSaveData_t));

            FILE * fpd = fopen(FilePath,"a");
            for(unsigned int rdi=0;rdi<sizeof(rSaveData_t);rdi++){
                fprintf(fpd,"%c",DataConvert[rdi]);
            }
            fprintf(fpd,"\r\n\r\n");
            fclose(fpd);
        }
        else{
            // printf("Not enough data..\n");
            // LOG_E("Not enough data..");
            return -1;
        }
    }
    {
        char FilePath[200];
        rres.currTime = curr;
        sprintf(FilePath,"%sdata/Result_%d.xkd",path,(d->tm_year+1900)*10000 + (d->tm_mon+1)*100 + (d->tm_mday));

        char DataConvert[sizeof(fSaveData_t)]={0};

        memcpy(DataConvert,&rres,sizeof(fSaveData_t));

        FILE * fpd = fopen(FilePath,"a");
        for(unsigned int rdi=0;rdi<sizeof(fSaveData_t);rdi++){
        fprintf(fpd,"%c",DataConvert[rdi]);
        }
        fprintf(fpd,"\r\n\r\n");

        fclose(fpd);
    }
    // printf("Metadata is saved.\n");

    return 0;
}

int forLEDctrl(int Nset){
    // rserial_t* r = Res[Nset].r;
    
    // int Bindx = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    // int SelR = Res[Nset].r[0].data[Bindx][Para6];
    // Bindx = (*Res[Nset].r[SelR].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    // float fRes = r[SelR].data[Bindx][ResUI0];
    float fRes = get_fstatus(0);
    
    char path[100]={0};
    sprintf(path,PATH_SAVE);
    {
        char FilePath[200];
        sprintf(FilePath,"%sresLED.csv",path);
        FILE * fpd = fopen(FilePath,"w");

        fprintf(fpd,"%.0f\n",fRes);

        // if(fRes == 2)
        //     fprintf(fpd,"%d,%d,%d,%.2f\n",255,0,0,0.5);
        // else if(fRes == 1)
        //     fprintf(fpd,"%d,%d,%d,%.2f\n",0,0,255,0.5);
        // else if(fRes == 6)
        //     fprintf(fpd,"%d,%d,%d,%.2f\n",0,0,255,1.0);
        // else if(fRes == FALLOFF_STATUS)
        //     fprintf(fpd,"%d,%d,%d,%.2f\n",0,255,0,0.0);
        // else if(fRes == EMERGENCY_STATUS)
        //     fprintf(fpd,"%d,%d,%d,%.2f\n",255,0,255,0.5);
        // else /*if(fRes == 0)*/
        //     fprintf(fpd,"%d,%d,%d,%.2f\n",0,255,0,1.0);

        fclose(fpd);
    }

    return 0;
}

float get_fstatus(int Nset){
    rserial_t* r = Res[Nset].r;
    
    int Bindx = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int SelR = Res[Nset].r[0].data[Bindx][Para6];
    Bindx = (*Res[Nset].r[SelR].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    float fRes = r[SelR].data[Bindx][ResUI0];
    
    unsigned int HoldinFlag,EmergencyFlag;
    char path[100]={0};
    sprintf(path,PATH_SAVE);
    {
        char FilePath[200];
        sprintf(FilePath,"%sbeaconScan.csv",path);
        FILE * fpd = fopen(FilePath,"a+");
        fscanf(fpd,"%d,%d",&HoldinFlag,&EmergencyFlag);

        fclose(fpd);
    }
    // printf("%d,%d\n",HoldinFlag,EmergencyFlag);

    if(HoldinFlag > 1 ) HoldinFlag = 0;
    if(EmergencyFlag > 1) EmergencyFlag = 0;

    float fReturn = EmergencyFlag?EMERGENCY_STATUS:(HoldinFlag?FALLOFF_STATUS:fRes);
    // printf("%.0f,%.0f\n",fReturn,fRes);

    return fReturn;
}
