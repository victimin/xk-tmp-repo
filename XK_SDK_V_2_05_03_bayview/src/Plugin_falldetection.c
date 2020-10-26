
#define _GNU_SOURCE
#include <stdio.h>
#include "XK_tools.h"
#include "radar.h"
#include <string.h>

#include "XK_CommonDefineSet.h"
#include "Plugin_falldetection.h"

#define RESULT_NONE                             0
#define RESULT_OK                               1
#define RESULT_ERRO                             -1

#define WMFD_RADAR_ID_HIGHER                    11
#define WMFD_RADAR_ID_LOWER                     22

#define WMFD_PARAM_IDX_ID                       0
#define WMFD_PARAM_IDX_ENDRANGE                 2
#define WMFD_PARAM_IDX_HEIGHT                   30

typedef struct
{
    int port;
    float height;
    float range;
} WMFD_INFO_t;

typedef struct
{
    WMFD_INFO_t gWmfdInfoHigher;
    WMFD_INFO_t gWmfdInfoLower;
} WMFD_HANDLE_t;

WMFD_HANDLE_t XK_WMFDHandle;

int ParseRadarInfo(WMFD_HANDLE_t *XK_WMFDHandle, float *dataArr, int port);

void *WMFDThread(XK_HTTPHandle_t *XK_HTTPHandle)
{
    int i, j;
    int res;
    
    LOG_I("Plugin_WMFD","WMFD logic loading...");

    delay(3000);
    while(1)
    {
        res=0;
        for (i = 0; i < MAX_NUM_USB_DEVICE; i++)
        {
            if(usbStatus[i] == XK_USB_STATUS_WMFD){
                res += ParseRadarInfo(&XK_WMFDHandle, XK_HTTPHandle->rcvdRadarData[i], i);

                // printf("%d,\n", XK_WMFDHandle.gWmfdInfoHigher.port);
                // printf("%f,\n", XK_WMFDHandle.gWmfdInfoHigher.height);
                // printf("%f,\n", XK_WMFDHandle.gWmfdInfoHigher.range);

                // printf("%d,\n", XK_WMFDHandle.gWmfdInfoLower.port);
                // printf("%f,\n", XK_WMFDHandle.gWmfdInfoLower.height);
                // printf("%f,\n", XK_WMFDHandle.gWmfdInfoLower.range);

                // for (j = 0; j < 20; j++)
                // {
                //     // printf("%f,\n", XK_HTTPHandle->rcvdRadarData[i][j]);
                // }
            }
        }

        if(res==RESULT_OK*2){
            printf("Secceed\n");
        }
        else{
            printf("Failed (%d)\n", res);
        }

        delay(10);
    }
}

int ParseRadarInfo(WMFD_HANDLE_t *XK_WMFDHandle, float *dataArr, int port){
    int rtn=0;

    if(dataArr[WMFD_PARAM_IDX_ID + PROTOCOL_INFO_MAX_SZ] == WMFD_RADAR_ID_HIGHER){
        XK_WMFDHandle->gWmfdInfoHigher.port = port;
        XK_WMFDHandle->gWmfdInfoHigher.height = dataArr[WMFD_PARAM_IDX_HEIGHT + PROTOCOL_INFO_MAX_SZ];
        XK_WMFDHandle->gWmfdInfoHigher.range = dataArr[WMFD_PARAM_IDX_ENDRANGE + PROTOCOL_INFO_MAX_SZ];
        rtn=RESULT_OK;
    }
    else if(dataArr[WMFD_PARAM_IDX_ID + PROTOCOL_INFO_MAX_SZ] == WMFD_RADAR_ID_LOWER){
        XK_WMFDHandle->gWmfdInfoLower.port = port;
        XK_WMFDHandle->gWmfdInfoLower.height = dataArr[WMFD_PARAM_IDX_HEIGHT + PROTOCOL_INFO_MAX_SZ];
        XK_WMFDHandle->gWmfdInfoLower.range = dataArr[WMFD_PARAM_IDX_ENDRANGE + PROTOCOL_INFO_MAX_SZ];
        rtn=RESULT_OK;
    }
    else{

    }
    return rtn;
}



