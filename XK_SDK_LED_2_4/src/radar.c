
#include <stdio.h>
#include "XK_tools.h"
#include "radar.h"
#include <string.h>
#include <time.h> 
#include <stdarg.h> 
#include "UART.h"

typedef union {
	char c[4*RECV_BUFF_SIZE];
	int i[RECV_BUFF_SIZE];
	float f[RECV_BUFF_SIZE];
} rcvmsg_t;

float tmp_err=0;
int cntErr=0, cntGenError = 0, cntErr1=0;

int Get_XKRadar_Data_Packet(XK_UARTHandle_t *XK_UARTHandle, stGetRadar_Data_t * g_Radar_Data, unsigned int * idxFront, unsigned int *preIdxXAKA, unsigned int *curIdxXAKA, unsigned char numOfUSB)
{
    rcvmsg_t rcvmsg;
    unsigned int frame_cnt=0;
    unsigned char tmpBuff[3000]={0,};
    int len;
    int i, j;
	int loopi, loopj;
    int fd = *XK_UARTHandle->fd;
    int devUSB = XK_UARTHandle->devUSB;
    int numUSRCMD = XK_UARTHandle->numUSRCMD;
    char prntOptn = XK_UARTHandle->prntOptn;
    // char *flgConn = &XK_UARTHandle->st;
    char *flgConn = &XK_UARTHandle->flgConnection;
    char *flgRcvable = &XK_UARTHandle->flgDataAble;
    unsigned int *appNumber = &XK_UARTHandle->appNum;
    unsigned char *recvQueue = XK_UARTHandle->recvQueue;
    unsigned int *cntNoData = &XK_UARTHandle->cntNodata;
    int *rID = &XK_UARTHandle->radarID;
    int *paramSize = &XK_UARTHandle->paramSize;
    stGetRadar_Data_t * dest_Data = g_Radar_Data;

    unsigned int *CheckOneTime = &XK_UARTHandle->CheckOneTime;

    unsigned long waitGETTime=clock();
    struct timeval newGETTimeT;
    struct timeval waitGETTimeT;
    gettimeofday(&waitGETTimeT, NULL);
       
    unsigned char absCmpCdtion = 0;
    unsigned char flgCompGet = 0;
    unsigned char cntTimeout = 0;
    unsigned char cntTimeout2 = 0;
    
    unsigned char preSeqData=0;
    int prntCol_N = PRINT_COL, prntCol, prntRow, prntRow_Tail;

    int temp_4byte = 0, numArg=0;

    dest_Data->m_nGetFlag[devUSB] = 0;
    flgCompGet = 0;
    
    waitGETTime=clock();
    while(flgCompGet == 0){
        len = read(fd, &tmpBuff[0], MAX_STR_LEN);  
        // printf("len = %d\n",len);       
        // for (i = 0; i < len; i++){
        //     printf("recev = %x\n",tmpBuff[i]);
        // }

        if(len){            
            waitGETTime = clock();

            for (i = 0; i < len; i++){
                recvQueue[idxFront[devUSB]++] = tmpBuff[i];
                if(idxFront[devUSB]==RECV_BUFF_SIZE) idxFront[devUSB] = 0;            
            }

            // for (i = 0; i < RECV_BUFF_SIZE; i++){
            for (i = 0; i < len; i++){
                if(recvQueue[(RECV_BUFF_SIZE + idxFront[devUSB]-i)%RECV_BUFF_SIZE] == 'A'&&recvQueue[(RECV_BUFF_SIZE + idxFront[devUSB]-i-1)%RECV_BUFF_SIZE] == 'K' &&
                                recvQueue[(RECV_BUFF_SIZE + idxFront[devUSB]-i-2)%RECV_BUFF_SIZE] == 'A'&&recvQueue[(RECV_BUFF_SIZE + idxFront[devUSB]-i-3)%RECV_BUFF_SIZE] == 'X'){
                    
                    *CheckOneTime = 1;

                    curIdxXAKA[devUSB] = (RECV_BUFF_SIZE + idxFront[devUSB] - i - 3)%RECV_BUFF_SIZE;
                    if(curIdxXAKA[devUSB] != preIdxXAKA[devUSB]){
                        *cntNoData = 0;

                        for (j = 0; j < RECV_BUFF_SIZE; j++){
                            if(recvQueue[(RECV_BUFF_SIZE + curIdxXAKA[devUSB]-j   - 4)%RECV_BUFF_SIZE] == 'A'
                            && recvQueue[(RECV_BUFF_SIZE + curIdxXAKA[devUSB]-j-1 - 4)%RECV_BUFF_SIZE] == 'K' 
                            && recvQueue[(RECV_BUFF_SIZE + curIdxXAKA[devUSB]-j-2 - 4)%RECV_BUFF_SIZE] == 'A'
                            && recvQueue[(RECV_BUFF_SIZE + curIdxXAKA[devUSB]-j-3 - 4)%RECV_BUFF_SIZE] == 'X')
                            {
                                preIdxXAKA[devUSB] = (RECV_BUFF_SIZE + curIdxXAKA[devUSB] - j - 3 - 4)%RECV_BUFF_SIZE;
                                break;
                            }
                        }
                        
                        switch (prntOptn){
                            case MAIN_OPT_RAW :
                                temp_4byte = curIdxXAKA[devUSB] - preIdxXAKA[devUSB];
                                if(temp_4byte<0){
                                    temp_4byte = (RECV_BUFF_SIZE + curIdxXAKA[devUSB]) - preIdxXAKA[devUSB];
                                } 
                                printf("received %d byte\n", temp_4byte);
                                
                                for (i = 0; i < temp_4byte; i++){
                                if(i==4) printf("[ ");
                                    printf("%d ", recvQueue[(preIdxXAKA[devUSB]+i)%RECV_BUFF_SIZE]);
                                if(i==7) printf("]");
                                }

                                absCmpCdtion = (recvQueue[(preIdxXAKA[devUSB] + 7)%RECV_BUFF_SIZE] - preSeqData);
                                if(absCmpCdtion < 0) absCmpCdtion * (-1);
                                if(absCmpCdtion > 1){ 
                                    printf("#");
                                }                            

                            break;

                            case MAIN_OPT_DEBUG :                        
                                printf("\n[%d/%d]!!!!\n", preIdxXAKA[devUSB],curIdxXAKA[devUSB]);
                                temp_4byte = curIdxXAKA[devUSB] - preIdxXAKA[devUSB];
                                if(temp_4byte<0){
                                    temp_4byte = (RECV_BUFF_SIZE + curIdxXAKA[devUSB]) - preIdxXAKA[devUSB];
                                } 
                                numArg = temp_4byte/4;

                                for(loopi=0;loopi<numArg;loopi++){
                                    for(loopj=0;loopj<4;loopj++){    
                                        rcvmsg.c[loopi * 4 + loopj] = recvQueue[(preIdxXAKA[devUSB]+(loopi * 4 + loopj))%RECV_BUFF_SIZE];
                                    }                                  
                                        dest_Data->m_nALLRadarData[devUSB][loopi] = rcvmsg.f[loopi];
                                }
                                *rID = rcvmsg.i[1];
                                *paramSize = rcvmsg.i[2];

                                printf("# XK system : [Info] %d Data received \n",temp_4byte);
                                if(numArg > MAX_PRINT_SIZE){
                                    printf("# XK system : [Warnning] Received data is longer than Maximum size\n");
                                    numArg = MAX_PRINT_SIZE;
                                } 
                                prntRow = numArg / prntCol_N;
                                prntRow_Tail = numArg % prntCol_N;

                                for(loopi=0;loopi<prntRow;loopi++){
                                    for(loopj=0;loopj<prntCol_N;loopj++){  
                                        if(loopi+loopj*prntRow == 0){
                                            printf("head =\t%c%c%c%c\t\t\t", rcvmsg.c[0], rcvmsg.c[1], rcvmsg.c[2], rcvmsg.c[3]);
                                        }
                                        else if(loopi+loopj*prntRow == 1){
                                            printf("rID =\t%d\t\t\t", rcvmsg.i[1]);
                                        }
                                        else if(loopi+loopj*prntRow == 2){
                                            printf("rsv =\t%d\t\t\t", rcvmsg.i[2]);
                                        }
                                        else if(loopi+loopj*prntRow == 3){
                                            printf("app =\t%d\t\t\t", (int)rcvmsg.f[3]);
                                        }
                                        else printf("a%02d =\t%6.6f\t\t", loopi+loopj*prntRow-1, rcvmsg.f[loopi+loopj*prntRow]);
                                    } 
                                    printf("\n");
                                }
                                for(loopi=0;loopi<prntRow_Tail;loopi++) printf("a%02d =\t%6.6f\t\t", loopi+loopj*prntRow-1, rcvmsg.f[loopi+loopj*prntRow]);
                                printf("\n");

                                for (i = 8; i < temp_4byte; i++){
                                    if(recvQueue[(preIdxXAKA[devUSB]+i)%RECV_BUFF_SIZE] != i%255&&frame_cnt>1000) cntErr1++;
                                }
                                // for (i = 8; i < temp_4byte; i++){
                                printf("@@@@@%f    %f\n", rcvmsg.f[4], tmp_err);
                                // if( (rcvmsg.f[4] - tmp_err > 1)&&frame_cnt>1000) cntErr++;
                                if( (*paramSize != numArg-3)&&frame_cnt>5) cntErr++;
                                tmp_err = rcvmsg.f[4];
                                // }
                                preSeqData = recvQueue[(preIdxXAKA[devUSB] + 7)%RECV_BUFF_SIZE];
                                printf("[/dev/ttyUSB%d error = %d]\n", devUSB,  cntErr);  
                                printf("[/dev/ttyUSB%d error2 = %d]\n", devUSB,  cntErr1);  
                            break;

                            case MAIN_OPT_DEBUG_V :                        
                                printf("\n\n[%d/%d]!!!!\n", preIdxXAKA[devUSB],curIdxXAKA[devUSB]);
                                temp_4byte = curIdxXAKA[devUSB] - preIdxXAKA[devUSB];
                                if(temp_4byte<0){
                                    temp_4byte = (RECV_BUFF_SIZE + curIdxXAKA[devUSB]) - preIdxXAKA[devUSB];
                                } 
                                numArg = temp_4byte/4;

                                for(loopi=0;loopi<numArg;loopi++){
                                    for(loopj=0;loopj<4;loopj++){    
                                        rcvmsg.c[loopi * 4 + loopj] = recvQueue[(preIdxXAKA[devUSB]+(loopi * 4 + loopj))%RECV_BUFF_SIZE];
                                    }                                  
                                        dest_Data->m_nALLRadarData[devUSB][loopi] = rcvmsg.f[loopi];
                                }
                                *rID = rcvmsg.i[1];
                                *paramSize = rcvmsg.i[2];

                                printf("# XK system : [Info] %d Data received \n",temp_4byte);
                                if(numArg > MAX_PRINT_SIZE){
                                    printf("# XK system : [Warnning] Received data is longer than Maximum size\n");
                                    numArg = MAX_PRINT_SIZE;
                                } 
                                prntRow = numArg / prntCol_N;
                                prntRow_Tail = numArg % prntCol_N;

                                for(loopi=0;loopi<prntRow;loopi++){
                                    for(loopj=0;loopj<prntCol_N;loopj++){  
                                        if(loopi+loopj*prntRow == 0){
                                            printf("head =\t%c%c%c%c\t\t\t", rcvmsg.c[0], rcvmsg.c[1], rcvmsg.c[2], rcvmsg.c[3]);
                                        }
                                        else if(loopi+loopj*prntRow == 1){
                                            printf("rID =\t%d\t\t\t", rcvmsg.i[1]);
                                        }
                                        else if(loopi+loopj*prntRow == 2){
                                            printf("rsv =\t%d\t\t\t", rcvmsg.i[2]);
                                        }
                                        else if(loopi+loopj*prntRow == 3){
                                            printf("app =\t%d\t\t\t", (int)rcvmsg.f[3]);
                                        }
                                        else printf("v%02d =\t%6.6f\t\t", loopi+loopj*prntRow-4, rcvmsg.f[loopi+loopj*prntRow]);
                                    } 
                                    printf("\n");
                                }
                                for(loopi=0;loopi<prntRow_Tail;loopi++) printf("v%02d =\t%6.6f\t\t", loopi+loopj*prntRow-1, rcvmsg.f[loopi+loopj*prntRow]);
                                printf("\n");

                                for (i = 8; i < temp_4byte; i++){
                                    if(recvQueue[(preIdxXAKA[devUSB]+i)%RECV_BUFF_SIZE] != i%255&&frame_cnt>1000) cntErr1++;
                                }
                                printf("@@@@@%f    %f\n", rcvmsg.f[4], tmp_err);
                                if( (*paramSize != numArg-3)&&frame_cnt>5) cntErr++;
                                tmp_err = rcvmsg.f[4];
                                // }
                                preSeqData = recvQueue[(preIdxXAKA[devUSB] + 7)%RECV_BUFF_SIZE];
                                printf("[/dev/ttyUSB%d error1 = %d]\n", devUSB,  cntErr);  
                                printf("[/dev/ttyUSB%d error2 = %d]\n", devUSB,  cntErr1);  
                                
                            break;

                            default :
                                temp_4byte = curIdxXAKA[devUSB] - preIdxXAKA[devUSB];
                                if(temp_4byte<0){
                                    temp_4byte = (RECV_BUFF_SIZE + curIdxXAKA[devUSB]) - preIdxXAKA[devUSB];
                                } 
                                numArg = temp_4byte/4;

                                if(rcvmsg.i[2] != (temp_4byte-4*3)/4 && frame_cnt>5) 
                                    LOG_E("Data Error", "DN: %d         Received N: %d       Received data: %d bytes\n", rcvmsg.i[2], (temp_4byte-4*3)/4, temp_4byte);
                                
                                for(loopi=0;loopi<numArg;loopi++){
                                    for(loopj=0;loopj<4;loopj++){    
                                        rcvmsg.c[loopi * 4 + loopj] = recvQueue[(preIdxXAKA[devUSB]+(loopi * 4 + loopj))%RECV_BUFF_SIZE];
                                    }                                  
                                        dest_Data->m_nALLRadarData[devUSB][loopi] = rcvmsg.f[loopi];
                                }                                                       
                                
                                *rID = rcvmsg.i[1];
                                *paramSize = rcvmsg.i[2];

                                if(temp_4byte > 0){
                                    *appNumber = (unsigned int)rcvmsg.f[3];
                                }
#if FALL_ENABLE
                                if(*appNumber == FallDetection_WallMount){
                                    usbStatus[numOfUSB] = XK_USB_STATUS_WMFD;
                                }
#endif
                                if(numArg > MAX_PRINT_SIZE){
                                    numArg = MAX_PRINT_SIZE;
                                } 
                                
                                prntRow = numArg / prntCol_N;
                                prntRow_Tail = numArg % prntCol_N;
                            break;
                        }
                        // *flgConn = 1;
                        flgCompGet = 1;  
                        *flgRcvable = 1;
                    } 
                    preIdxXAKA[devUSB] = curIdxXAKA[devUSB];  
                    dest_Data->m_nGetFlag[devUSB] = 1;
                    break;
                }
            }
        }
        // if(flgCompGet) return temp_4byte/4;
        gettimeofday(&newGETTimeT, NULL);
        if( newGETTimeT.tv_sec - waitGETTimeT.tv_sec > 1 && len == 0){
        // if( (clock() - waitGETTime)/CLOCKS_PER_SEC > 2 && len == 0){
            if(*cntNoData > 1) {
                //LOG_E("USB", "/dev/ttyUSB%d No Data!!", numOfUSB);
            }
            (*cntNoData)++;
            // LOG_E("error", "%d         %d", len, *flgConn);
            if((*CheckOneTime) == 0 && (*cntNoData)>1){
                // printf("AppNum is changed");
                *appNumber = OTHER_DEVICE;
            }
            return -3;
        } 

        
        // if(*flgConn==1 && len==0 && cntTimeout++>3){
        //     *flgConn==0;
        //     return -2;
        // }
        
        // if(len==0 && *flgConn==1){
        //     if(cntTimeout++ > 100){
        //         // *flgConn = 0;
        //         return -2;
        //     } 
        // }
        // if(len==0 && *flgConn==0){
        //     if(cntTimeout++ > 20){
        //         *flgRcvable = 0;
        //         return -3;
        //     } 
        // }

    }
    frame_cnt++;
    return numArg;
}

