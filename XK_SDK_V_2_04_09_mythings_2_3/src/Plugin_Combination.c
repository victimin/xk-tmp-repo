#include "Plugin_Combination.h"

typedef union {
	char c[4*10];
	int i[10];
	float f[10];
} metaMsg_t;

int DataTrans_Z2Z(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo, int masterPort);
int DataTrans_I2Z(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo, int masterPort);

int CombinationProcess_Zone_Inout(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo){
    int i, j;

#if (0)
int in1=1;
int zone1=2;
int zone2=0;

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
        radarInfo->rcvdRadarData[zone1][COMBO_FINAL_PPL_CNT_Z_IO_COMBO+4],
        radarInfo->rcvdRadarData[zone2][COMBO_FINAL_PPL_CNT_Z_IO_COMBO+4]);
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
                radarInfo->rcvdRadarData[i][COMBO_FINAL_PPL_CNT_Z_IO_COMBO+4]
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
                delay(50);
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
                delay(50);
            }
        }
    }
    
    // printf("\n");
}

