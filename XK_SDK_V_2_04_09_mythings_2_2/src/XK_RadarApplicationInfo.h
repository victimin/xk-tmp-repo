#ifndef __XK_RADARAPPLICATIONINFO_H__
#define __XK_RADARAPPLICATIONINFO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "XK_CommonDefineSet.h"

#define SEND_PARAM_SIZE_MAX             (20u)
#define APP_SIZE                        (8u)        //it is not max

typedef enum
{
   PERS = 10,
   IN_OUT = 20,
   IN_OUT_Single = 21,
   Zone_PeopleCounting = 30,
   Zone_PeopleCounting_Combo = 31,
   //Fall_Detection,
   Presence_Detection = 40,
   PresenceVital = 41,
   DwellTime = 42,
   Level3_Presence_Detection = 43,
   FallDetection_WallMount = 50,
   Foot_Traffic = 60,
   Presence_and_Object = 70,
   Skimmer = 71,
   RHRBR = 80,
} ApplicationNumber;


#define USE_LEGACY_APPNUM 1
typedef enum  {
    RADAR_APP_WMFALL = 1, // DH
    RADAR_APP_INOUT, // DH
    OTHER_DEVICE, // DH
    RADAR_APP_MAX_SZ = OTHER_DEVICE
}RADAR_APP_NUM_t;

typedef enum {
    PROTOCOL_INFO_XAKA = 0,
    PROTOCOL_INFO_RESERVED_1,
    PROTOCOL_INFO_RESERVED_2,
    PROTOCOL_INFO_APP_NUM,
    PROTOCOL_INFO_MAX_SZ
}PROTOCOL_INFO_t;

typedef enum {	
    ZONE_IDX_SERIAL_NUMBER1 = 27,
    ZONE_IDX_SERIAL_NUMBER2 = 28,
	ZONE_IDX_UI_MAX_SZ
}ZONE_IDX_t;

typedef enum {	
    PERS_IDX_SERIAL_NUMBER1 = 27,
    PERS_IDX_SERIAL_NUMBER2 = 28,
	PERS_IDX_UI_MAX_SZ
}PERS_IDX_t;

typedef enum {	
    FALL_IDX_SERIAL_NUMBER1 = 27,
    FALL_IDX_SERIAL_NUMBER2 = 28,
	FALL_IDX_UI_MAX_SZ
}FALL_IDX_t;

typedef enum {	
    PRESENCE_IDX_SERIAL_NUMBER1 = 20,
    PRESENCE_IDX_SERIAL_NUMBER2 = 21,
	PRESENCE_IDX_UI_MAX_SZ
}PRESENCE_IDX_t;

typedef enum {	
    OSR_IDX_SERIAL_NUMBER1,
    OSR_IDX_SERIAL_NUMBER2,
	OSR_IDX_UI_MAX_SZ
}OSR_IDX_t;

typedef enum {	//Foot 
    FOOT_IDX_SERIAL_NUMBER1=19,
    FOOT_IDX_SERIAL_NUMBER2=20,
	FOOT_IDX_UI_FinalStateFlag
}FOOTTRAFFIC_IDX_t;

typedef enum { 
    WMFALL_IDX_SERIAL_NUMBER1=20,
    WMFALL_IDX_SERIAL_NUMBER2=21,
	WMFALL_IDX_UI_FinalStateFlag
}WMFALL_IDX_t;

typedef enum { 
    INOUT_IDX_SERIAL_NUMBER1=19,
    INOUT_IDX_SERIAL_NUMBER2=20,
	INOUT_IDX_UI_FinalStateFlag
}INOUT_IDX_t;

typedef enum { 
    RHRBR_IDX_SERIAL_NUMBER1=16,
    RHRBR_IDX_SERIAL_NUMBER2=17,
	RHRBR_IDX_UI_FinalStateFlag
}RHRBR_IDX_t;

// extern char appParamName[PRESENCE_IDX_UI_MAX_SZ][50];
extern char appParamName[RADAR_APP_MAX_SZ][PRESENCE_IDX_UI_MAX_SZ][50];
extern unsigned int appParamSerialNumInfo[RADAR_APP_MAX_SZ][2];

typedef struct {
    int flgSIC;                 //Send if changed
    int paramObjNum_V;
    char paramName[50];
} param_info_t;

typedef struct {
    int appNum;
    int appParamSZ;
	int appSerialNumIdx;
    char appName[30];
    param_info_t param[SEND_PARAM_SIZE_MAX];
} app_info_t;

app_info_t gAppInfo[APP_SIZE];

#ifdef __cplusplus
}
#endif


#endif

