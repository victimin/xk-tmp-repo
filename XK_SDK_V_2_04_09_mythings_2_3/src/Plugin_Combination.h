#ifndef __PLUGIN_COMBINATION_H__
#define __PLUGIN_COMBINATION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "XK_RadarApplicationInfo.h"

#define COMBO_DATA_HEADER_ZONE              501111
#define COMBO_DATA_HEADER_INOUT             105555
#define COMBO_DATA_MAX_SZ                   20

#define COMBO_METADATA_Z_DEV_ID             12
#define COMBO_METADATA_Z_ACCMLTVPPLCNT      15

#define COMBO_METADATA_IO_IN_CNTACC         16
#define COMBO_METADATA_IO_OUT_CNTACC        17

#define COMBO_FINAL_PPL_CNT_Z_IO_COMBO      46

typedef enum
{
    XK_COMBO_APP_NUM_INOUT = IN_OUT_Single,
    XK_COMBO_APP_NUM_ZONE = Zone_PeopleCounting_Combo
} XK_COMBO_APP_NUM;

typedef enum
{
    XK_COMB_ACTVT_OFF = 0,
    XK_COMB_ACTVT_ON = 1,
} XK_COMB_ACTVT;


typedef struct {
    int flgActivate;
    int appNum;
    int portNum;
    float zoneAccPplCnt;
    float zoneDevId;
    float inoutAccInCnt;
    float inoutAccOutCnt;
    float metaData[COMBO_DATA_MAX_SZ];
} XK_ComboInfo_t;


int CountActvSensor(void);
int CombinationProcess_Zone_Inout(XK_HTTPHandle_t * radarInfo, XK_UARTHandle_t * uartInfo);

XK_ComboInfo_t combInfo[MAX_NUM_USB_DEVICE];
int gCntCombinationActivate;

#ifdef __cplusplus
}
#endif


#endif

