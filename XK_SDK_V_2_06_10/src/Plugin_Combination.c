#include "Plugin_Combination.h"
#include "mqtt.h"

typedef union {
	char c[4*10];
	int i[10];
	float f[10];
} metaMsg_t;

extern float rcvdData_shm[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];

int DataTrans_Z2Z(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo, int masterPort);
int DataTrans_I2Z(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo, int masterPort);

int CombinationProcess_Zone_Inout(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo){
    int i, j;
    int finalPort = -1;



    if (gFlgComboDebugging){
        printf("\n\n\nAppNum\tSerial\t\tPort\tZoneAccPplCnt\tInCnt\tOutCnt\n");
        for(i=0;i<MAX_NUM_USB_DEVICE;i++){
            if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE){
                printf("%d\t%s\t%d\t\x1b[33m%f\x1b[0m\n", combInfo[i].appNum,  radarInfo->serialNum[i], i, combInfo[i].zoneAccPplCnt);
                finalPort = i;
            }
        }
        for(i=0;i<MAX_NUM_USB_DEVICE;i++){
            if(combInfo[i].appNum == XK_COMBO_APP_NUM_INOUT){
                printf("%d\t%s\t%d\t\t\t\x1b[36m%d\t%d\x1b[0m\n", combInfo[i].appNum,  radarInfo->serialNum[i], i, (int)combInfo[i].inoutAccInCnt, (int)combInfo[i].inoutAccOutCnt);
            }
        }
        printf("--------------------------------------------------------------\n");
        if(finalPort == -1) printf("\t\t\t\t\t\tfinal: No Zone Combo\n");
        else printf("%d,%d\t\t\t\t\t\t\x1b[32mFinal: %d\x1b[0m\n",
                                                (int)rcvdData_shm[finalPort][47+4], 
                                                (int)rcvdData_shm[finalPort][48+4], 
                                                (int)rcvdData_shm[finalPort][PARAM_IDX_ZONE_COMBO_FINAL_CNT+4]);

    }
#if (0)
int in1=1;
int zone1=0;
int zone2=2;

    printf("# how many zone %f    how inout %f    //    how zone %f    how inout %f  cnt0 %f  cnt1 %f cnt2 %f\n", 
        radarInfo->rcvdRadarData[zone1][47+4],
        radarInfo->rcvdRadarData[zone1][48+4],
        radarInfo->rcvdRadarData[zone2][47+4],
        radarInfo->rcvdRadarData[zone2][48+4],
        radarInfo->rcvdRadarData[in1][4+4],
        radarInfo->rcvdRadarData[zone1][0+4],
        radarInfo->rcvdRadarData[zone2][0+4]
    );

    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE)
            printf("## %d zone :%f     \n", i,  combInfo[i].zoneAccPplCnt);
        if(combInfo[i].appNum == XK_COMBO_APP_NUM_INOUT)
            printf("## %d in :%f     out :%f  \n", i,  combInfo[i].inoutAccInCnt, combInfo[i].inoutAccOutCnt);
    }

    printf("### Final is %f     %f\n", 
        radarInfo->rcvdRadarData[zone1][PARAM_IDX_ZONE_COMBO_FINAL_CNT+4],
        radarInfo->rcvdRadarData[zone2][PARAM_IDX_ZONE_COMBO_FINAL_CNT+4]);
#endif

#if (0)
    // printf("# how many zone %f    how inout %f    //    how zone %f    how inout %f  cnt0 %f  cnt1 %f cnt2 %f\n", 
    //     radarInfo->rcvdRadarData[1][47+4],
    //     radarInfo->rcvdRadarData[1][48+4],
    //     radarInfo->rcvdRadarData[2][47+4],
    //     radarInfo->rcvdRadarData[2][48+4],
    //     radarInfo->rcvdRadarData[0][4+4],
    //     radarInfo->rcvdRadarData[1][0+4],
    //     radarInfo->rcvdRadarData[2][0+4]
    // );

    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE){
            printf("## %d zone :%f     ", i,  combInfo[i].zoneAccPplCnt);
            printf("# how many zone %f    how inout %f       cnt: %f       longterm: %f\t", 
                radarInfo->rcvdRadarData[i][47+4],
                radarInfo->rcvdRadarData[i][48+4],
                radarInfo->rcvdRadarData[i][0+4],
                radarInfo->rcvdRadarData[i][7+4]
                
            );
        }
        if(combInfo[i].appNum == XK_COMBO_APP_NUM_INOUT){
            printf("## %d in :%f     out :%f     cnt: %f\n", i,  combInfo[i].inoutAccInCnt, combInfo[i].inoutAccOutCnt, radarInfo->rcvdRadarData[i][4+4]);
        }
    }
    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE){
            printf("### Final is %f\n", 
                radarInfo->rcvdRadarData[i][PARAM_IDX_ZONE_COMBO_FINAL_CNT+4]
                );
        }
    }
#endif

    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        if(combInfo[i].flgActivate == 1){
            if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE){
                DataTrans_Z2Z(radarInfo, uartInfo, i);
            }
            else if(combInfo[i].appNum == XK_COMBO_APP_NUM_INOUT){
                DataTrans_I2Z(radarInfo, uartInfo, i);
            }
        }
    }
}

int CountActvSensor(void){
    int i;
    int temp=0;

    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        temp += combInfo[i].flgActivate;
    }
    return temp;
}

int DataTrans_Z2Z(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo, int masterPort){
    int i;
    char tmpSerial[10]={0, };
    int idxMeta=0;
    metaMsg_t meta;

    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        idxMeta=0;
        if(combInfo[i].flgActivate == 1){
            if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE){
                memcpy(tmpSerial, &radarInfo->serialNum[masterPort][6], 6);
                meta.f[idxMeta++] = COMBO_DATA_HEADER_ZONE;
                meta.f[idxMeta++] = (float)atoi(tmpSerial);
                meta.f[idxMeta++] = combInfo[masterPort].zoneDevId;
                meta.f[idxMeta++] = combInfo[masterPort].zoneAccPplCnt;

                // printf("###### master %d,     slave %d\n", masterPort, i);
                // printf("@@@@@ %f \t", meta.f[0]);
                // printf("@@@@@ %f \t", meta.f[1]);
                // printf("@@@@@ %f \n", meta.f[2]);
                write(*uartInfo[i].fd, &meta.c[0], idxMeta*4);
                delay(150);
            }
        }
    }
    // printf("\n");
}

int DataTrans_I2Z(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo, int masterPort){
    int i;
    char tmpSerial[10]={0, };
    int idxMeta=0;
    metaMsg_t meta;

    for(i=0;i<MAX_NUM_USB_DEVICE;i++){
        idxMeta=0;
        if(combInfo[i].flgActivate == 1){
            if(combInfo[i].appNum == XK_COMBO_APP_NUM_ZONE){
                memcpy(tmpSerial, &radarInfo->serialNum[masterPort][6], 6);
                meta.f[idxMeta++] = COMBO_DATA_HEADER_INOUT;
                meta.f[idxMeta++] = (float)atoi(tmpSerial);
                meta.f[idxMeta++] = combInfo[masterPort].inoutAccInCnt;
                meta.f[idxMeta++] = combInfo[masterPort].inoutAccOutCnt;

                // printf("###### master %d,     slave %d \n", masterPort, i);
                // printf("@@@@@ %f \t", meta.f[0]);
                // printf("@@@@@ %f \t", meta.f[1]);
                // printf("@@@@@ %f \t", meta.f[2]);
                // printf("@@@@@ %f \n", meta.f[3]);
                write(*uartInfo[i].fd, &meta.c[0], idxMeta*4);
                delay(150);
            }
        }
    }
    
    // printf("\n");
}

