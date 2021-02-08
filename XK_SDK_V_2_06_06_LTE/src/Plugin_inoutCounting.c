
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
#include "Plugin_inoutCounting.h"
// #include "httpClient.h"
#include "Plugin_inoutCounting_type.h"


#define SEND2R_BUFF_SIZE RECV_BUFF_SIZE
#define CPU_N 1 // TODO: What number can it use.
#define MYTIME_PER_SEC (1000)


#if INOUT_DEBUG_ENABLE
#define VEIW_N 99 // 2: result data / 1: 2 + send2radar packet / 0: 1 + 2 + recieved relay data from radar //99: all parameter
#define INOUT_DEBUG_ENABLE_LV2 0
    static int justCheck(int nn);
#define INOUT_DEBUG_ENABLE_SIMPLE 1
#else
#define INOUT_DEBUG_ENABLE_SIMPLE 0
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ABS(x)                      ((x > 0) ? x : (-1)*x)
#define SIGN(x)                     ((x == 0) ? 0 : ((x > 0) ? 1 : -1))
#define GET_SIZE_FLOAT_DATA         RELAY_S_LENGTH_MAX
#define RESULT_DATA_LENGTH          50
#define START_CURSOR                50
#define TIME_N_INOUT                9//1
#define MAXDATALENGTH               6000
#define SPACE_N_INOUT               3//1
#define D_LENGTH                    7
#define DECENDING_N_TIME            8 //2
#define DECENDING_N_SPACE           6 //1
#define DIRECTION_SEARCH_MAX_LAG    10
#define DIRECTION_SEARCH_OFFSET     5
#define RRRR_SIZE_D                 3
#define SIGN_PERCENTAGE             0.6
#define LENGTH_SLOWTIME             10
#define SIGN_MASK_TIME              2000//1200
#define SIGN_MASK_DISTANCE          2
#define INOUT_RATIO                  2

static float RawData_Size_L[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA] = {0, };
static float RawData_Size_R[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA] = {0, };
static float RawData_Size_L_column[LENGTH_SLOWTIME] = {0};
static float RawData_Size_R_column[LENGTH_SLOWTIME] = {0};
static float A_corr_Matrix[GET_SIZE_FLOAT_DATA][LENGTH_SLOWTIME * 2 - 1] = {0, };
static float A_corr[LENGTH_SLOWTIME * 2 - 1] = {0};
static float XCorr_Max_Value_Matrix[GET_SIZE_FLOAT_DATA] = {0};
static int XCorr_Max_Index_Matrix[GET_SIZE_FLOAT_DATA] = {0};
static float Result[RESULT_DATA_LENGTH][GET_SIZE_FLOAT_DATA] = {0};
static int Result2[RESULT_DATA_LENGTH][GET_SIZE_FLOAT_DATA] = {0};
static float Post_Result[RESULT_DATA_LENGTH][GET_SIZE_FLOAT_DATA] = {0, };
static int Post_Result2[RESULT_DATA_LENGTH][GET_SIZE_FLOAT_DATA] = {0, };
static float Peak_Array[MAXDATALENGTH][2] = {0, };
static int RRRR[DIRECTION_SEARCH_MAX_LAG][RRRR_SIZE_D] = {0, };
static float R_Minus_Sign_Array[MAXDATALENGTH][GET_SIZE_FLOAT_DATA] = {0, };
static float R_Plus_Sign_Array[MAXDATALENGTH][GET_SIZE_FLOAT_DATA] = {0, };
static float R_Sign_Array[MAXDATALENGTH][GET_SIZE_FLOAT_DATA] = {0, };
static float CountingHist[MAXDATALENGTH][3] = {0, };
static float R_Plus_Sign_Array_Sum_300[GET_SIZE_FLOAT_DATA] = {0, };
static float R_Minus_Sign_Array_Sum_300[GET_SIZE_FLOAT_DATA] = {0, };
static float R_Sign_Array_Copy[MAXDATALENGTH][GET_SIZE_FLOAT_DATA] = {0, };

static int DataCursor = 1;
static int LoopFlag = 0;
static const float CorrThreshold = 0.00000000000005;
static int DDongFlag;
// static int HHH,HHH_R, XXX_L, XXX_H, YYY_L, YYY_H, TotalNNN;
// static int D_sum = 0;
static unsigned int NumberOfPoint = 0;
static int RRRR2_Sign, sign_sum, vvv_Difference;
static int TotalNumPeople = 0;
static int D[D_LENGTH] = {0};
static int HHH,HHH_R, XXX_L, XXX_H, YYY_L, YYY_H, TotalNNN;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static pthread_t SendToRadar_Thread_t;
static pthread_mutex_t SendRadar_inout_lock;

static void *SendToRadarThread(void *data);
static int CheckRadarFunc_inout(void);
static unsigned long mytime(struct timeval* inputT, struct timeval* refT);
static int mytimeAdd(struct timeval* inputT, const long period);
static unsigned long mytimecheck(struct timeval* inputT);
static int makeMsgToRadar(float* RelayData[],int nsi);
void XCorrelation(float *A, float *B, float *Result_corr, int Size_corr);

static initDataSet_t D_init;
static result_inout_t Res[INOUT_MAX_N];
static int fps_inout;
static int IsInout[MAX_NUM_USB_DEVICE] = {0};

static struct timeval ZTime;

static int updateFalg = 0;
static int sendCheckFalg = 0;

static int Radar_Cal_Flag = 0;
static int SavedLen = 0;

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

    int BindexCheckArr[NUM_RADAR_INOUT]={0};

    delay(3000);
    waitGETTime = mytime(&WTime,&RTime);
    CheckwaitGETTime = mytime(&WTimeT,&RTimeT);

    int stateFalg = 0;
    
    while(1)
    {
        if(CheckRadarFunc_inout()<0) {
#if INOUT_DEBUG_ENABLE
            printf("Waiting for Connection with InOut\n");
#endif
            delay(3000);
            
            gettimeofday(&RTime, NULL);
            continue;
        }
        for (int nsi = 0; nsi < D_init.NumSystem; nsi++){
            if(!getNoData_Inout(nsi)){

                int TempI = 0;
                // int rNum,Bindx,sglen,prlen;
                int sglen;
                int bi1 = (*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                int bi2 = (*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

                int dc1 = Res[nsi].r[0].data[bi1][ParaCursor];
                int dc2 = Res[nsi].r[1].data[bi2][ParaCursor];

                int TimeoutCnt = 0;
                while(dc1 == BindexCheckArr[0] || dc2 == BindexCheckArr[1] && TimeoutCnt<200){
                    bi1 = (*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    bi2 = (*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    dc1 = Res[nsi].r[0].data[bi1][ParaCursor];
                    dc2 = Res[nsi].r[1].data[bi2][ParaCursor];
                    // printf(".");
                    TimeoutCnt++;
                    delay(10);
                }
                BindexCheckArr[0] = dc1;
                BindexCheckArr[1] = dc2;

                if(TimeoutCnt>=200) continue;

                sglen = Res[nsi].r[0].data[bi1][SendtoRadarP[1]];
                sglen += Res[nsi].r[1].data[bi1][SendtoRadarP[1]];
                sglen /= 2;
                SavedLen = sglen;
                char *inData[2];
                inData[0] = &Res[nsi].r[0].data[bi1][(int)Res[nsi].r[0].data[bi1][SendtoRadarP[2]] + R_INDEX_OFFSET];
                inData[1] = &Res[nsi].r[1].data[bi2][(int)Res[nsi].r[1].data[bi2][SendtoRadarP[2]] + R_INDEX_OFFSET];
                dataStore(sglen,inData);
                // printf("Bindx: %d / %d\t\t",bi1,bi2);
                // printf("DataCursor: %d\t\tCursorRcvd: %.0f / %.0f\t\t",DataCursor,Res[nsi].r[0].data[bi1][ParaCursor],Res[nsi].r[1].data[bi2][ParaCursor]);
                // printf("Thread Send2Radar at time: %ld ms // %ld ms\t\t",mytime(&WTime,&ZTime),mytimecheck(&RTime));
                {
                    int a = (*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    int b = (*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    // printf("Result: %.2f / %.2f\t\t",Res[nsi].r[0].data[a][RESULT_IN],Res[nsi].r[0].data[b][RESULT_OUT]);
                }
                long PeriodT = (Res[nsi].r[0].data[bi1][PARA_PERIOD_N] + Res[nsi].r[1].data[bi2][PARA_PERIOD_N])*MYTIME_PER_SEC/2;
#if INOUT_DEBUG_ENABLE
                if(mytime(&WTimeT,&RTimeT) > MYTIME_PER_SEC){
                    printf("PERIOD: %ld\n",PeriodT);
                    printf("RefTime for sendMsg: %ld ms\n",mytimecheck(&RTime));
                    printf("RunTime ref sendMsg: %ld ms\n",mytime(&WTime,&RTime));
                    printf("RunTime ref Zero   : %ld ms\n\n",mytime(&WTime,&ZTime));
                    mytimeAdd(&RTimeT,MYTIME_PER_SEC);
                }
#endif
                if(mytime(&WTime,&RTime) > PeriodT && stateFalg==0){
                    mytimeAdd(&RTime,PeriodT);
                    // LOG_I("INOUT", "Cycle is back");

                    pthread_mutex_lock(&SendRadar_inout_lock);
                    combineData(sglen);
                    pthread_mutex_unlock(&SendRadar_inout_lock);
#if INOUT_DEBUG_ENABLE
                    justCheck(VEIW_N);
#endif

                    float* forRelayData[NUM_RADAR_INOUT];
                    
                    // TODO: CHANGE THE DATA
                    // forRelayData[0] = &Res[nsi].r[0].data[bi1][(int)Res[nsi].r[0].data[bi1][SendtoRadarP[2]] + R_INDEX_OFFSET];
                    // forRelayData[1] = &Res[nsi].r[1].data[bi2][(int)Res[nsi].r[1].data[bi2][SendtoRadarP[2]] + R_INDEX_OFFSET];
                    forRelayData[0] = R_Minus_Sign_Array_Sum_300;
                    forRelayData[1] = R_Plus_Sign_Array_Sum_300;

                    makeMsgToRadar(forRelayData,nsi);
                    
                    while(dc1 == BindexCheckArr[0] || dc2 == BindexCheckArr[1]){
                        bi1 = (*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                        bi2 = (*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                        dc1 = Res[nsi].r[0].data[bi1][ParaCursor];
                        dc2 = Res[nsi].r[1].data[bi2][ParaCursor];
                        // printf(".");
                        // delay(10);
                    }
                    stateFalg = 1;
#if INOUT_DEBUG_ENABLE
                    printf("Check point for 30 seconds %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
#endif
                }
                else if(stateFalg==1){
                    updateFalg = 1;
                    stateFalg = 0;
                }
            }
        }
    } // while(1)
}


int inoutInit(stGetRadar_Data_t g_Radar_Data[],XK_UARTHandle_t XK_UARTHandle[], int nShearBufferIdx[],short usbStatus[]){

    fps_inout = FPS_INOUT; // TODO: It will be Changalbe.

    D_init.UARTHandle_Temp = XK_UARTHandle;
    D_init.g_Data = g_Radar_Data;
    D_init.nShearBufferIdx = nShearBufferIdx;
    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
        D_init.appNum[i] = &(XK_UARTHandle[i].appNum);
    }
    D_init.usbStatus = usbStatus;

    D_init.NumSystem = 0;

    pthread_mutex_init(&SendRadar_inout_lock, NULL);

    int thrdCreate = pthread_create(&SendToRadar_Thread_t, NULL, (void *)SendToRadarThread, (void *)'a');
    if (pthread_detach(SendToRadar_Thread_t) != 0) 
    {
        perror("pthread_detach(SendToRadarThread)\n");
    }

    return 0;
}

int inoutDeinit(){
    pthread_mutex_destroy(&SendRadar_inout_lock);
    pthread_cancel(SendToRadar_Thread_t);
    int status;
    pthread_join(SendToRadar_Thread_t,(void **)&status);

    return 0;
}


int CheckRadarFunc_inout(){
    int Cnt_N_inout = 0;
    int IsInoutIndex[MAX_NUM_USB_DEVICE] = {0};
    int StateFlag = 0;
    int ChangeFlag = 0;

    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
    #if USE_LEGACY_APPNUM
        if(D_init.usbStatus[i]==0 && (*D_init.appNum[i] == IN_OUT || *D_init.appNum[i] == 11))
    #else
        if(D_init.usbStatus[i]==0 && *D_init.appNum[i] == IN_OUT)
    #endif
        {
#if INOUT_DEBUG_ENABLE_LV2
            printf("Inout radar port is %2d and AppNum is %d...\n",i,*D_init.appNum[i]);
#endif
            if(IsInout[i] != 1) ChangeFlag = 1;
            
            IsInout[i] = 1;
            IsInoutIndex[Cnt_N_inout] = i;
            Cnt_N_inout++;
        } else{
            if(IsInout[i] != 0) ChangeFlag = 1;
            IsInout[i] = 0;
        }
    }
#if INOUT_DEBUG_ENABLE_LV2
    for (int i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
    #if USE_LEGACY_APPNUM
        if(D_init.usbStatus[i]==0 && (*D_init.appNum[i] == IN_OUT || *D_init.appNum[i] == 11))
    #else
        if(D_init.usbStatus[i]==0 && *D_init.appNum[i] == IN_OUT)
    #endif
        {
            printf("other port is %2d and AppNum is %d...\n",i,*D_init.appNum[i]);
        }
    }
#endif
    for (int i = Cnt_N_inout; i < MAX_NUM_USB_DEVICE; i++){
            IsInoutIndex[Cnt_N_inout] = -1;
    }

    if(Cnt_N_inout<NUM_RADAR_INOUT){
        StateFlag = -1;
    }

    if(ChangeFlag == 0) return StateFlag;
    #if INOUT_DEBUG_ENABLE
        printf("Radar Port has been changed...\n");
    #endif
    
    // for(int pi=0;pi<Cnt_N_inout;pi++){
    //     int Bindx = (D_init.nShearBufferIdx[pi]-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    //     int chckid = D_init.g_Data[Bindx].m_nALLRadarData[pi][Para0]; // TODO: Check how to pair for each radar.
    //     char tempStr2[100];
    //     sprintf(tempStr2,"/dev/ttyACM%d",pi);
    //     LOG_I(tempStr2, "A InOut radar (ID:%d) has been detected.",chckid);
    // }

    if(Cnt_N_inout<NUM_RADAR_INOUT){
        StateFlag = -1;
        LOG_E("system", "The InOut radar has been detected, but Not enough the radars.");
        return StateFlag;
    }

    int N_inout = 0;
    int ArrRID[MAX_NUM_USB_DEVICE] = {0};
    int ArrRID_port[MAX_NUM_USB_DEVICE] = {0};

    // Sort the Radar ID.
    for (int ci = 0; ci < Cnt_N_inout; ci++)
    {
        int i = IsInoutIndex[ci];
        // int chckid = D_init.UARTHandle_Temp[i].radarID;
        int Bindx = (D_init.nShearBufferIdx[i]-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
        int chckid = D_init.g_Data[Bindx].m_nALLRadarData[i][Para0]; // TODO: Check how to pair for each radar.
        int CK_port = i;
        for (int cci = 0; cci < Cnt_N_inout; cci++)
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
    
#if INOUT_DEBUG_ENABLE_LV2
    for (int ci = 0; ci < Cnt_N_inout; ci++) {
        printf("%6d/%d",ArrRID[ci],ArrRID_port[ci]);
    }
    printf("\n");
#endif
    if(Cnt_N_inout > NUM_RADAR_INOUT){
        // Check the No of pair.
        int ccic;
        for (ccic = 0; ccic < Cnt_N_inout-1; ccic++){
            if(ArrRID[ccic] != ArrRID[ccic+1]){
                int TEMP_arr[MAX_NUM_USB_DEVICE] = {0};
                int TEMP_arr2[MAX_NUM_USB_DEVICE] = {0};
                memcpy(TEMP_arr, &ArrRID_port[ccic+1],(Cnt_N_inout-1)*4);
                memcpy(&ArrRID_port[ccic], TEMP_arr,(Cnt_N_inout-1)*4);
                memcpy(TEMP_arr2, &ArrRID[ccic+1],(Cnt_N_inout-1)*4);
                memcpy(&ArrRID[ccic], TEMP_arr2,(Cnt_N_inout-1)*4);
                Cnt_N_inout--;
                ccic--;
            }
            else{
                ccic++;
            }
        }
        if(ccic != Cnt_N_inout) Cnt_N_inout--;

#if INOUT_DEBUG_ENABLE_LV2
        for (int ci = 0; ci < Cnt_N_inout; ci++) {
            printf("%6d/%d",ArrRID[ci],ArrRID_port[ci]);
        }
        printf("\n");
#endif
    }
    
    D_init.NumSystem = Cnt_N_inout/2;
    if(D_init.NumSystem>INOUT_MAX_N){
        D_init.NumSystem = INOUT_MAX_N;
    }
    #if INOUT_DEBUG_ENABLE
        printf("There are %d set of inout radar...\n", D_init.NumSystem);
    #endif
    
    // TODO: It has a pair as order of ID.
    int totalCnt = 0;
    for (int nsi = 0; nsi < D_init.NumSystem; nsi++)
    {
        for (int ci = 0; ci < NUM_RADAR_INOUT; ci++)
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
        LOG_I("system", "The InOut system #%d (radar ID %d & %d) is started.",nsi,Res[nsi].r[0].RadarID,Res[nsi].r[1].RadarID);
    }
    return StateFlag;
}




#if INOUT_DEBUG_ENABLE

int justCheck(int nn){
    #if INOUT_DEBUG_ENABLE_LV2
        printf("In the justCheck\n");
    #endif
    for (int nsi = 0; nsi < D_init.NumSystem; nsi++){
        printf("-------------------------------------------------------------\n");
        switch (nn){
            case 0:
                // for (int rri = 0; rri < NUM_RADAR_INOUT; rri++)
                // {
                //     int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                //     for(int spi = 0;spi<sizeof(SendtoRadarP)/4;spi++){
                //             printf("%10.2f",Res[nsi].r[rri].data[Bindx][SendtoRadarP[spi]]);
                //     }
                //     printf("\n-------------------------------------------------------------\n");
                // }

            case 1:{
                // int spi = 0;
                // printf("%10.2f",Res[nsi].Send2Rbuf[spi++]);
                // printf("\n");
                // for(;spi<sizeof(SendtoRadarP)/4 + 1;spi++){
                //     printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                // }
                // printf("\n");
                // for(;spi<(sizeof(SendtoRadarP)/4)*2 + 1;spi++){
                //     printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                // }
                // printf("\n");
                // printf("%10.2f",Res[nsi].Send2Rbuf[spi++]);
                // printf("%10.2f",Res[nsi].Send2Rbuf[spi++]);
                // printf("\n-------------------------------------------------------------\n");
                
                // printf("\n");
                // for(spi = 0;spi<RELAY_MSG_LEN;spi++){
                //     printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                // }
                // printf("\n-------------------------------------------------------------\n");
            }

            case 2:
                // for (int rri = 0; rri < NUM_RADAR_INOUT; rri++)
                // {
                //     printf("Np%d:P%d:I%d:f%d::",Res[nsi].r[rri].NumPair,Res[nsi].r[rri].port,Res[nsi].r[rri].RadarID,*(Res[nsi].r[rri].fd));
                //     int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                //     for(int spi = 0;spi<sizeof(ResRadarP)/4;spi++){
                //             printf("%10.2f",Res[nsi].r[rri].data[Bindx][ResRadarP[spi]]);
                //     }
                //     printf("\n-------------------------------------------------------------\n");
                // }
            default:
                break;
            case 98: 
                for(int spi = 0;spi<RELAY_MSG_LEN;spi++){
                    printf("%10.2f",Res[nsi].Send2Rbuf[spi]);
                }
                printf("\n-------------------------------------------------------------\n");
                break;
            case 99:
                for (int rri = 0; rri < NUM_RADAR_INOUT; rri++)
                {
                    int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    for(int spi = 0;spi<NUM_PARA_ADD;spi++){
                            printf("%10.2f",Res[nsi].r[rri].data[Bindx][spi]);
                            if(spi%5==5-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n-------------------------------------------------------------\n");
                }
                break;
            case 101:
                for (int rri = 0; rri < NUM_RADAR_INOUT; rri++)
                {
                    int Bindx = (*Res[nsi].r[rri].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
                    for(int spi = 0;spi<NUM_PARA_ADD;spi++){
                            printf("%10.2e",Res[nsi].r[rri].data[Bindx][spi]);
                            if(spi%5==5-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n-------------------------------------------------------------\n");
                }
                break;
            case 102:
                for (int rri = 0; rri < LENGTH_SLOWTIME; rri++)
                {
                    for(int spi = 0;spi<26;spi++){ // RawData_Size_L[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA]
                            printf("%10.2e",RawData_Size_L[rri][spi]);//RawData_Size_L // RawData_Size_R
                            if(spi%13==13-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n%d-------------------------------------------------------------\n",rri);
                }
                for (int rri = 0; rri < LENGTH_SLOWTIME; rri++)
                {
                    for(int spi = 0;spi<26;spi++){ // RawData_Size_L[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA]
                            printf("%10.2e",RawData_Size_R[rri][spi]);//RawData_Size_L // RawData_Size_R
                            if(spi%13==13-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n%d-------------------------------------------------------------\n",rri);
                }
                break;
            case 103:
                for (int rri = 0; rri < 1; rri++)
                {
                    for(int spi = 0;spi<26;spi++){ // RawData_Size_L[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA]
                            printf("%10.2e",Result[rri][spi]);//RawData_Size_L // RawData_Size_R
                            if(spi%13==13-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n-------------------------------------------------------------\n",rri);
                }
                for (int rri = 0; rri < 1; rri++)
                {
                    for(int spi = 0;spi<26;spi++){ // RawData_Size_L[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA]
                            printf("%10d",Result2[rri][spi]);//RawData_Size_L // RawData_Size_R
                            if(spi%13==13-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n-------------------------------------------------------------\n",rri);
                }
                for (int rri = 0; rri < 1; rri++)
                {
                    for(int spi = 0;spi<26;spi++){ // RawData_Size_L[LENGTH_SLOWTIME][GET_SIZE_FLOAT_DATA]
                            printf("%10d",Post_Result2[rri][spi]);//RawData_Size_L // RawData_Size_R
                            if(spi%13==13-1) printf("\t=>\t%d\n",spi);
                    }
                    printf("\n-------------------------------------------------------------\n",rri);
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

    return (Sec - SecR)*MYTIME_PER_SEC + (uSec - uSecR)/(1000000/MYTIME_PER_SEC);
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

    return (Sec - SecR)*MYTIME_PER_SEC + (uSec - uSecR)/(1000000/MYTIME_PER_SEC);
}

int mytimeAdd(struct timeval* inputT, const long period){

    unsigned long* Sec = &(inputT->tv_sec);
    unsigned long* uSec = &(inputT->tv_usec);

    unsigned long factor = (1000000/MYTIME_PER_SEC);
    *uSec += (period*factor)%1000000;
    *Sec += (period*factor)/1000000;
    // printf("asdfasdfasdfasd:%ld\n",period*factor);

    if(1000000 < *uSec) {
        *uSec -= 1000000;
        *Sec += 1;
    }

    return 0;
}

void makeJsonInoutResult(char * data, int Nset){
        
    int a = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int b = (*Res[Nset].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    float InRes = Res[Nset].r[0].data[a][RESULT_IN];
    float OutRes = Res[Nset].r[0].data[a][RESULT_OUT];
    float UpdataCnt = Res[Nset].r[0].data[a][RESULT_UPDATE_COUNTER];

    char templeteStr[]=
        ",\r\n\t\t\"in\":\"%0.2f\""
        ",\r\n\t\t\"out\":\"%0.2f\""
        ",\r\n\t\t\"inout\":\"%0.2f\""
        ",\r\n\t\t\"count\":\"%0.2f\""
        ;
    sprintf(data,templeteStr,
                        InRes,
                        OutRes,
                        InRes+OutRes,
                        UpdataCnt
                        );
                        
    updateFalg = 0;
}

int getNumofSet_Inout(){
    // return (D_init.NumSystem & updateFalg);
    return (D_init.NumSystem);
}

int isEnable_Inout(){
    return (D_init.NumSystem>0);
}

int getNoData_Inout(int Nset){

    int ResInt = 0;

    for(int ri=0;ri<NUM_RADAR_INOUT;ri++){
        if(*Res[Nset].r[ri].cntNodata>1){
            ResInt = 1;
        }
    }
    return 0;
}


float getRadarInfo_Inout(int infoN, int numR, int Nset){

    float res = -99;
    int Bindx = (*Res[Nset].r[numR].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    switch(infoN){
        case GET_INFO_IO_ID: // Radar ID
            res = (float)Res[Nset].r[numR].RadarID;
            res = Res[Nset].r[numR].data[Bindx][SendtoRadarP[0]];
            break;
        case GET_INFO_IO_SERIAL1:
            res = Res[Nset].r[numR].data[Bindx][SERIAL_1];
            break;
        case GET_INFO_IO_SERIAL2:
            res = Res[Nset].r[numR].data[Bindx][SERIAL_2];
            break;
        case GET_INFO_IO_PORT:
            res = (float)Res[Nset].r[numR].port;
            break;
        case GET_INFO_IO_NODATA:
            res = (float)(*Res[Nset].r[numR].cntNodata>1);
            break;
        default:
            break;
    }

    return res;
}



int makeUIInoutResult(char * data, int Nset){
    rserial_inout_t* r = Res[Nset].r;

    int Bindx = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int BindxA[2];
    BindxA[0] = (*Res[Nset].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    BindxA[1] = (*Res[Nset].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    float* DataArr[NUM_RADAR_INOUT];

    DataArr[0] = r[0].data[BindxA[0]];
    DataArr[1] = r[1].data[BindxA[1]];

    int DataByteN = (DataArr[0][PARA_IDX_NUM_N]+1)*4;
    float Data4Send[4096] = {0};
    memcpy(Data4Send,"XGUI",sizeof(char)*4);

    for (int pi = 0; pi < DataArr[0][PARA_IDX_NUM_N]; pi++){
        Data4Send[pi+1] = DataArr[0][pi+4];
        // printf("%.2f\t",Data4Send[pi+1]);
        // if(pi%5==5-1) printf("\n");
    }

    memcpy(data,Data4Send,DataByteN);

    return DataByteN;
}


int cmd_Inout_system_legacy(float RRxdata[],int Nset, int len){
    char SyncByte[4] = "XGUI";
    float *SyncFp = (float*)SyncByte;

    float fDataArr[1024] = {0};

    int fd_arr[NUM_RADAR_INOUT];
    fd_arr[0] = *(Res[Nset].r[0].fd);
    fd_arr[1] = *(Res[Nset].r[1].fd);

    if(*SyncFp != RRxdata[0]) return 0;
    // printf("%s\n",RRxdata);

    // pthread_mutex_lock(&SendRadar_inout_lock);
    // printf("After mutex\n");


    // for (int pi = 1;pi < len;pi++) printf("RRxdata[%d] %f\n", pi, RRxdata[pi]);

    if (RRxdata[1] == 5005){
        fDataArr[0] = 5005.0;
        pthread_mutex_lock(&SendRadar_inout_lock);
        write(fd_arr[0], fDataArr, 4);
        write(fd_arr[1], fDataArr, 4);
        pthread_mutex_unlock(&SendRadar_inout_lock);
        return 0;
    }
    else if (RRxdata[1] == 6006){
        LOG_I("InOut","Start Radar Mapping Command received.");
        Radar_Cal_Flag = 1;
        fDataArr[0] = 6006.0;
        
        TotalNumPeople = 0;
        NumberOfPoint = 0;
        DataCursor = 1;

        for (int i = 0;i <= MAXDATALENGTH;i++){
            for (int j = 0;j < GET_SIZE_FLOAT_DATA;j++){
                R_Plus_Sign_Array[i][j] = 0;
                R_Minus_Sign_Array[i][j] = 0;
            }
        }
        
        for (int i = 0;i <= MAXDATALENGTH;i++){
            for (int j = 0;j < 2;j++){
                Peak_Array[i][j] = 0;
            }
        }
        for (int i = 0;i <= MAXDATALENGTH;i++){
            for (int j = 0;j < 3;j++){
                CountingHist[i][j] = 0;
            }
        }
        
        pthread_mutex_lock(&SendRadar_inout_lock);
        write(fd_arr[0], fDataArr, 4);
        write(fd_arr[1], fDataArr, 4);
        pthread_mutex_unlock(&SendRadar_inout_lock);
        return 0;
    }
    else if (RRxdata[1] == -1){
        fDataArr[0] = -1;
        
        pthread_mutex_lock(&SendRadar_inout_lock);
        write(fd_arr[0], fDataArr, 4);
        write(fd_arr[1], fDataArr, 4);
        pthread_mutex_unlock(&SendRadar_inout_lock);
        return 0;
        
    }
    else if (RRxdata[1] == 100100){
        fDataArr[0] = 100100;
        
        pthread_mutex_lock(&SendRadar_inout_lock);
        write(fd_arr[0], fDataArr, 4);
        write(fd_arr[1], fDataArr, 4);
        pthread_mutex_unlock(&SendRadar_inout_lock);
        return 0;
        
    }
    else if (RRxdata[1] == 7980){
        LOG_I("InOut","Finish Radar Mapping Command received.");
        fDataArr[0] = 1010.1010;
        fDataArr[1] = RRxdata[2];
        fDataArr[2] = RRxdata[3];
        fDataArr[1] = RRxdata[4];
        fDataArr[2] = RRxdata[5];
        fDataArr[3] = 255.255;
        fDataArr[4] = 255.255;
        
        Radar_Cal_Flag = 2;
        combineData(SavedLen);
        
        pthread_mutex_lock(&SendRadar_inout_lock);
        write(fd_arr[0], fDataArr, 7*4);
        write(fd_arr[1], fDataArr, 7*4);
        pthread_mutex_unlock(&SendRadar_inout_lock);
        
    }
    else{
        if (RRxdata[1] == 4){
            fDataArr[0] = 1010.1010;
            fDataArr[1] = RRxdata[1];
            fDataArr[2] = RRxdata[2];
            fDataArr[3] = 255.255;
            fDataArr[4] = 255.255;

            pthread_mutex_unlock(&SendRadar_inout_lock);
            write(fd_arr[0], fDataArr, 5*4);
            write(fd_arr[1], fDataArr, 5*4);
            pthread_mutex_unlock(&SendRadar_inout_lock);
        }
        else{
            int selPort = (int)RRxdata[1];
            fDataArr[0] = 1010.1010;
            fDataArr[1] = RRxdata[1];
            fDataArr[2] = RRxdata[2];
            fDataArr[3] = 255.255;
            fDataArr[4] = 255.255;
            
            pthread_mutex_unlock(&SendRadar_inout_lock);
            write(fd_arr[selPort], fDataArr, 5*4);
            pthread_mutex_unlock(&SendRadar_inout_lock);
        }
        // pthread_mutex_unlock(&SendRadar_inout_lock);
        return 0;
    }
    // pthread_mutex_unlock(&SendRadar_inout_lock);
    return 0;
}

int makeMsgToRadar(float* RelayData[],int nsi){
    int TempI = 0;
    int rNum,Bindx,sglen,prlen;
    int bi1 = (*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int bi2 = (*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;

    memcpy(&(Res[nsi].Send2Rbuf[TempI++]),"IOCS",4);

    rNum = 0;
    Bindx = bi1;
    sglen = Res[nsi].r[rNum].data[Bindx][SendtoRadarP[1]];
    prlen = Res[nsi].r[rNum].data[Bindx][SendtoRadarP[2]];
    Res[nsi].Send2Rbuf[TempI++] = Res[nsi].r[rNum].data[Bindx][SendtoRadarP[0]];
    Res[nsi].Send2Rbuf[TempI++] = sglen;
    memcpy(&(Res[nsi].Send2Rbuf[TempI]),RelayData[0],sglen*4);
    TempI += sglen;

    memcpy(&(Res[nsi].Send2Rbuf[TempI++]),"////",4);

    rNum = 1;
    Bindx = bi2;
    sglen = Res[nsi].r[rNum].data[Bindx][SendtoRadarP[1]];
    prlen = Res[nsi].r[rNum].data[Bindx][SendtoRadarP[2]];
    Res[nsi].Send2Rbuf[TempI++] = Res[nsi].r[rNum].data[Bindx][SendtoRadarP[0]];
    Res[nsi].Send2Rbuf[TempI++] = sglen;
    memcpy(&(Res[nsi].Send2Rbuf[TempI]),RelayData[1],sglen*4);
    TempI += sglen;

    memcpy(&(Res[nsi].Send2Rbuf[TempI++]),"IOCE",4);

    Res[nsi].msglen = TempI*4;

#if INOUT_DEBUG_ENABLE_SIMPLE
    int a = (*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    int b = (*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER;
    printf("\n-------------------------------------------------------------\n");
    printf("Check the relay data\n");
    printf("Bindx: %d / %d\n",(*Res[nsi].r[0].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER
                            ,(*Res[nsi].r[1].BufferIdx-1 + GETDATA_BUFFER_NUMBER)%GETDATA_BUFFER_NUMBER);
    printf("sglen: %.0f / %.0f\n",Res[nsi].r[0].data[a][SendtoRadarP[1]]
                            ,Res[nsi].r[1].data[b][SendtoRadarP[1]]);
    printf("ID: %.0f / %.0f\n",getRadarInfo_Inout(GET_INFO_IO_ID,0,nsi)
                            ,getRadarInfo_Inout(GET_INFO_IO_ID,1,nsi));
    printf("msgLen: %d / %d\n",Res[nsi].msglen,TempI);
    printf("Result: %.2f / %.2f\n",Res[nsi].r[0].data[a][RESULT_IN]
                                ,Res[nsi].r[0].data[b][RESULT_OUT]);
    
    printf("\n-------------------------------------------------------------\n");
    for (int rri = 0; rri < NUM_RADAR_INOUT; rri++)
    {
        for(int spi = 0;spi<sglen;spi++){
                printf("%10.2e",RelayData[rri][spi]);
                if(spi%5==5-1) printf("\t=>\t%d\n",spi);
        }
        printf("\n-------------------------------------------------------------\n");
    }

    for(int spi = 0;spi<Res[nsi].msglen/4;spi++){
            printf("%10.2e",Res[nsi].Send2Rbuf[spi]);
            if(spi%5==5-1) printf("\t=>\t%d\n",spi);
    }
    printf("\n-------------------------------------------------------------\n");
#endif
    for(int rri=0;rri<NUM_RADAR_INOUT;rri++){
        write(*(Res[nsi].r[rri].fd),(char*)(Res[nsi].Send2Rbuf),Res[nsi].msglen);
    }

    return 0;
}

void XCorr_MaxValueIndex(float *A, int Size, float *MaxValue, int *MaxIndex)
{
   int i;
   float tmp = 0;
   int tmp_index = 0;

   for (i=0; i< Size * 2 -1; i++)   {
      if(tmp < A[i])   {
         tmp = A[i];
         tmp_index = i;
      }
   }
   *MaxValue = tmp;
   *MaxIndex = tmp_index - Size + 1;
}


int dataStore(int j_length,float *inData[]){
    int i,j,vvv;
    float Max_Value;
    int Max_Index;
    int HHH,HHH_R, XXX_L, XXX_H, YYY_L, YYY_H, TotalNNN;
    int MT_SearchStart_T, MT_SearchEnd_T, MT_SearchStart_S, MT_SearchEnd_S;
    int MD_SearchStart_T, MD_SearchEnd_T, MD_SearchStart_S, MD_SearchEnd_S;
    float MaxValue_Time, MaxValue_Distance;
    int DDong3,DDong2;
    
    // printf("\n-------------------------------------------------------------\n");
    // printf("j_length = %d\n", j_length);
    
    for(i = 0; i < LENGTH_SLOWTIME-1 ; i++){
        for(j = 0; j < j_length; j++){
            RawData_Size_L[i][j] = RawData_Size_L[i+1][j]; //1
            RawData_Size_R[i][j] = RawData_Size_R[i+1][j];//2
        }
    }
    
    for (i = 0;  i < j_length; i ++)	{
        RawData_Size_L[LENGTH_SLOWTIME-1][i] = inData[0][i];
        RawData_Size_R[LENGTH_SLOWTIME-1][i] = inData[1][i];
    }
    
    for(i=0;i< j_length ;i++){
        for(j=0;j< LENGTH_SLOWTIME ;j++){
            RawData_Size_L_column[j] = RawData_Size_L[j][i];
            RawData_Size_R_column[j] = RawData_Size_R[j][i];
        }

        XCorrelation(RawData_Size_L_column, RawData_Size_R_column, A_corr, LENGTH_SLOWTIME);
        XCorr_MaxValueIndex(A_corr, LENGTH_SLOWTIME, &Max_Value, &Max_Index);
        XCorr_Max_Value_Matrix[i] = Max_Value;
        XCorr_Max_Index_Matrix[i] = Max_Index;
    }

    //            #endif
    ///////////////////////////////////////////////////////////////////////////////////////error
    for(i = 0; i < RESULT_DATA_LENGTH-1 ; i++){
        for(j = 0; j < j_length; j++){
            Result[i][j] = Result[i+1][j];
            Result2[i][j] = Result2[i+1][j];
            Post_Result2[i][j] = Post_Result2[i+1][j];
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////error


    for(j = 0; j < j_length; j++){
        Result[RESULT_DATA_LENGTH-1][j] = XCorr_Max_Value_Matrix[j];
        Result2[RESULT_DATA_LENGTH-1][j] = XCorr_Max_Index_Matrix[j];
        Post_Result2[RESULT_DATA_LENGTH-1][j] = Result2[RESULT_DATA_LENGTH-1][j];
    }

    if((DataCursor > START_CURSOR-1) || (LoopFlag == 1)){
        LoopFlag = 1;
        HHH = (RESULT_DATA_LENGTH-1) - TIME_N_INOUT;//DataCursor - TIME_N_INOUT;      //isnt 8???????????????????????
        //HHH_R = DataCursor%MAXDATALENGTH-TIME_N_INOUT-1 > 0?DataCursor%MAXDATALENGTH-TIME_N_INOUT-1:MAXDATALENGTH+DataCursor%MAXDATALENGTH-TIME_N_INOUT-1;
        HHH_R = DataCursor%MAXDATALENGTH-TIME_N_INOUT-1 > 0?DataCursor%MAXDATALENGTH-TIME_N_INOUT-1:0;
        //      YYY_L = ((1>(HHH-TIME_N_INOUT))?1:(HHH-TIME_N_INOUT));
        YYY_L = (RESULT_DATA_LENGTH-1) - TIME_N_INOUT * 2 ;//0;
        YYY_H = (RESULT_DATA_LENGTH-1);//TIME_N_INOUT * 2 + 1;

        for(i = 0; i < RESULT_DATA_LENGTH-1; i++){
            for(j = 0; j < j_length; j++){
                Post_Result[i][j] = Post_Result[i+1][j];
            }
        }

        for(j = 0; j < j_length; j++){
            Post_Result[RESULT_DATA_LENGTH-1][j] = Result[HHH][j];
        }

        // D_sum = 0;
        for(vvv = 1; vvv < j_length; vvv++){
            XXX_L = ((0>(vvv-SPACE_N_INOUT))?0:(vvv-SPACE_N_INOUT));
            XXX_H = ((vvv+SPACE_N_INOUT)>j_length)?j_length:(vvv+SPACE_N_INOUT);

            for(i = 0; i < D_LENGTH; i++) D[i] = 1;
            D[1] = (Post_Result[HHH][vvv] > CorrThreshold)? 1 : 0;
        
            if(D[1] == 1){
                NumberOfPoint += 1;
                Peak_Array[NumberOfPoint][0] = HHH_R;
                Peak_Array[NumberOfPoint][1] = vvv;

                DDongFlag = 0;
                if(NumberOfPoint > 0){
                    MT_SearchStart_T = ((HHH - DECENDING_N_TIME) > 0)?(HHH - DECENDING_N_TIME):0;
                    MT_SearchEnd_T = (HHH + DECENDING_N_TIME > RESULT_DATA_LENGTH)?RESULT_DATA_LENGTH:HHH + DECENDING_N_TIME;
                    MT_SearchStart_S = ((vvv - 1) > 0)?(vvv - 1):0;
                    MT_SearchEnd_S = ((vvv + 1) < j_length)?(vvv + 1):j_length;

                    MD_SearchStart_T = ((HHH - 2) > 0)?(HHH - 2):0;
                    MD_SearchEnd_T = (HHH + 2 > RESULT_DATA_LENGTH)?RESULT_DATA_LENGTH:HHH + 2;
                    MD_SearchStart_S = ((vvv - DECENDING_N_SPACE) > 0)?(vvv - DECENDING_N_SPACE):0;
                    MD_SearchEnd_S = ((vvv + DECENDING_N_SPACE) < j_length)?(vvv + DECENDING_N_SPACE):j_length;

                    MaxValue_Time = 0;
                    MaxValue_Distance = 0;

                    for(i=MT_SearchStart_T; i<=MT_SearchEnd_T;i++){
                        for(j=MT_SearchStart_S;j<=MT_SearchEnd_S;j++){
                            MaxValue_Time = (Post_Result[i][j]>MaxValue_Time)?Post_Result[i][j]:MaxValue_Time;
                        }
                    }

                    for(i=MD_SearchStart_T; i<=MD_SearchEnd_T;i++){
                        for(j=MD_SearchStart_S;j<=MD_SearchEnd_S;j++){
                            MaxValue_Distance = (Post_Result[i][j]>MaxValue_Distance)?Post_Result[i][j]:MaxValue_Distance;
                        }
                    }

                    if(Post_Result[HHH][vvv] != MaxValue_Time || Post_Result[HHH][vvv] != MaxValue_Distance){
                        DDongFlag = 1;
                    }
                }
            
            
                for(i = 0; i < DIRECTION_SEARCH_MAX_LAG; i++){
                    for(j = 0; j < RRRR_SIZE_D; j++){
                        RRRR[i][j] = Post_Result2[HHH - DIRECTION_SEARCH_OFFSET - DIRECTION_SEARCH_MAX_LAG + i + 1][vvv-1+j];
                    }
                }                             

                int Sign_Plus = 0;
                int Sign_Minus = 0;
            
                for (i = 0; i < DIRECTION_SEARCH_MAX_LAG ; i++) { 
                    // [DIRECTION_SEARCH_MAX_LAG][RRRR_SIZE_D];
                    for (j = 0; j< RRRR_SIZE_D ; j++)  {
                        if (RRRR[i][j] > 0)	{
                            Sign_Plus += 1;
                        }
                        else if (RRRR[i][j] < 0) {
                            Sign_Minus += 1;
                        }
                    }
                }
            
                DDong3 = 0;
                if ((Sign_Plus + Sign_Minus) > DIRECTION_SEARCH_MAX_LAG*RRRR_SIZE_D )	{
                    DDong3 = 1;
                }
            
                DDong2 = 0;
                if ((Sign_Plus > 0) && (Sign_Minus > 0))  {
                    if (Sign_Plus >= SIGN_PERCENTAGE*(Sign_Minus+Sign_Plus))  {
                        sign_sum = 1;
                    }
                    else if (Sign_Minus >= SIGN_PERCENTAGE*(Sign_Minus+Sign_Plus))  {
                        sign_sum = -1;
                    }
                    else {
                        DDong2 = 1;
                    }
                }
                else if ((Sign_Plus > 0) && (Sign_Minus == 0))  {
                    sign_sum = 1;
                }
                else if ((Sign_Minus > 0) && (Sign_Plus == 0)) {
                    sign_sum = -1;
                }
                else {
                    DDong2 = 1;
                }
            
                if(DDongFlag == 1 || DDong2 == 1 || DDong3 == 1){
                    sign_sum = 0;

                    Peak_Array[NumberOfPoint][0] = 0;
                    Peak_Array[NumberOfPoint][1] = 0;
                    NumberOfPoint = NumberOfPoint - 1;
                }

                if(SIGN(sign_sum) == -1){
                    R_Minus_Sign_Array[HHH_R][vvv] = 1;
                    R_Sign_Array[HHH_R][vvv] = -1;
                    TotalNumPeople += 1;
                    CountingHist[TotalNumPeople][0] = Peak_Array[NumberOfPoint][0];
                    CountingHist[TotalNumPeople][1] = Peak_Array[NumberOfPoint][1];
                    CountingHist[TotalNumPeople][2] = -1;
                    //TotalIn += 1;
                    //EntranceTrace[HHH][vvv] = 1;
                }
                else if(SIGN(sign_sum) == 1){
                    R_Plus_Sign_Array[HHH_R][vvv] = 1;
                    R_Sign_Array[HHH_R][vvv] = 1;
                    TotalNumPeople += 1;
                    CountingHist[TotalNumPeople][0] = Peak_Array[NumberOfPoint][0];
                    CountingHist[TotalNumPeople][1] = Peak_Array[NumberOfPoint][1];
                    CountingHist[TotalNumPeople][2] = 1;
                    //TotalOut += 1;
                    //EntranceTrace[HHH][vvv] = -1;
                }
            }
        }

        // printf("TotalNumPeople: %d\t\t",TotalNumPeople);
        // printf("DataCursor: %d\n",DataCursor);
        // justCheck(103);
    
    }
            
        DataCursor  = (DataCursor+1)%MAXDATALENGTH;
}





void XCorrelation(float *A, float *B, float *Result_corr, int Size_corr){

   int i, j;
   float tmp=0;

   for (i = 0; i < Size_corr; i++){
      for(j = 0; j <= i ; j++){
         tmp += A[j] * B[Size_corr - i + j - 1];
      }
      Result_corr[i] = tmp/(i+1);
      tmp=0;
   }
   for (i = Size_corr-2; i >= 0; i--){
      for(j = 0; j <= i ; j++){
         tmp += A[Size_corr - i + j -1] * B[j];
      }
      Result_corr[(Size_corr-1) * 2 - i ] = tmp/(i+1);
      tmp=0;
   }

}

int combineData(int j_length){
    int i,ii,j,jj,kk;
        #if 1 // DoubleWindowFlag

        // if((tmp_past_time_300 != DataCursor/Cursor_of300s) || (Radar_Cal_Flag == 2)){

            int Time_300_Cursor = DataCursor%MAXDATALENGTH;
            int Time_Start_Cursor = 0;

            for(i = Time_Start_Cursor;i <= Time_300_Cursor;i++){
                for(j = 0;j < j_length;j++){
                    R_Sign_Array_Copy[i][j] = R_Sign_Array[i][j];
                }
            }
            // MaxTimeLength = Time_300_Cursor-Time_Start_Cursor+1;
        
            for(ii = 0;ii < TotalNumPeople;ii++){
                int TimeAxisLow = (CountingHist[ii][0] - SIGN_MASK_TIME) > 0?(CountingHist[ii][0] - SIGN_MASK_TIME):0;
                int TimeAxisHigh = (CountingHist[ii][0] + SIGN_MASK_TIME) > Time_300_Cursor?Time_300_Cursor:(CountingHist[ii][0] + SIGN_MASK_TIME);
                int DistanceAxisLow = (CountingHist[ii][1] - SIGN_MASK_DISTANCE) > 0?(CountingHist[ii][1] - SIGN_MASK_DISTANCE):0;
                int DistanceAxisHigh = (CountingHist[ii][1] + SIGN_MASK_DISTANCE) > j_length?j_length:(CountingHist[ii][1] + SIGN_MASK_DISTANCE);
                int MinusTemp = 0;
                int PlusTemp = 0;

                for(jj = TimeAxisLow;jj < TimeAxisHigh;jj++){
                    for(kk = DistanceAxisLow;kk < DistanceAxisHigh;kk++){
                        if(R_Sign_Array_Copy[jj][kk] == -1){
                            MinusTemp += 1;
                        }
                        else if(R_Sign_Array_Copy[jj][kk] == 1){
                            PlusTemp += 1;
                        }
                    }
                }
            
                if(MinusTemp != 0 && PlusTemp != 0){
                    if(PlusTemp/MinusTemp >= INOUT_RATIO){
                        if(CountingHist[ii][2] == -1){
                            R_Sign_Array[HHH_R][j_length] = 1;
                            R_Minus_Sign_Array[HHH_R][j_length] = 0;
                            R_Plus_Sign_Array[HHH_R][j_length] = 1;
                        }
                    }
                    else if(MinusTemp/PlusTemp >= INOUT_RATIO){
                        if(CountingHist[ii][2] == 1){
                            R_Sign_Array[HHH_R][j_length] = -1;
                            R_Plus_Sign_Array[HHH_R][j_length] = 0;
                            R_Minus_Sign_Array[HHH_R][j_length] = 1;
                        }
                    }
                }
            }

            for(ii = 0;ii < GET_SIZE_FLOAT_DATA;ii++){
                R_Plus_Sign_Array_Sum_300[ii] = 0;
                R_Minus_Sign_Array_Sum_300[ii] = 0;
            }
        
            for(ii = 0;ii < j_length;ii++){//GET_SIZE_FLOAT_DATA
                for(jj = Time_Start_Cursor;jj <= Time_300_Cursor;jj++){//for(jj = 0;jj <= Time_300_Cursor;jj++){
                    R_Plus_Sign_Array_Sum_300[ii] += R_Plus_Sign_Array[jj][ii];
                    R_Minus_Sign_Array_Sum_300[ii] += R_Minus_Sign_Array[jj][ii];
                }
            }

            if (Radar_Cal_Flag == 2)	{
                float fDataArr[100]={0};
                Radar_Cal_Flag = 0;
                fDataArr[0] = 1010.1010;
                fDataArr[1] = 11;
                fDataArr[2] = 1;
                fDataArr[3] = 255.255;
                fDataArr[4] = 255.255;
                delay(100);
                write(*(Res[0].r[0].fd), fDataArr, 5*4);
                write(*(Res[0].r[1].fd), fDataArr, 5*4);
                
                // printf("FACTOR RESULT : F = %f,         K = %f\n", g_Radar_Data[nShearBufferIdx].m_nALLRadarData[0][25+4], g_Radar_Data[nShearBufferIdx].m_nALLRadarData[0][26+4]);
            }

            // SEND : END
            // printf("RESULT : \t\t%.3f\t\t%.3f\t\t\t\t  tick :%.3f\tDataCursor :%.3f!!!!\n",(float)TotalIn,(float)TotalOut, (float)tmp_past_time_300, (float)DataCursor);
            TotalNumPeople = 0;
            NumberOfPoint = 0;
            DataCursor = 0;
            for (i = 0;i <= MAXDATALENGTH;i++)	{
                // [MAXDATALENGTH][GET_SIZE_FLOAT_DATA] = {0, };
                for (j = 0;j < GET_SIZE_FLOAT_DATA;j++)	{
                    //R_Sign_Array_Copy[i][j] = 0;
                    //R_Sign_Array[i][j] = 0;
                    R_Plus_Sign_Array[i][j] = 0;
                    R_Minus_Sign_Array[i][j] = 0;
                }
            }
        
            for (i = 0;i <= MAXDATALENGTH;i++) {
                for (j = 0;j < 2;j++)	  {
                    Peak_Array[i][j] = 0;
                }
            }
            for (i = 0;i <= MAXDATALENGTH;i++)	{
                for (j = 0;j < 3;j++)	  {
                    CountingHist[i][j] = 0;
                }
            }
            
            // if(cntNew30Data>=0xfffffff0) cntNew30Data = 0;
            // else cntNew30Data++;
        // }
    #endif
}