#ifndef __PLUGIN_INOUT_COUNTING_H__
#define __PLUGIN_INOUT_COUNTING_H__

#ifdef __cplusplus
extern "C"
{
#endif
///////////////////////////////////////////////////////////////////

#define FPS_INOUT (20)

#define INOUT_MAX_N 1
#define NUM_RADAR_INOUT 2

#define GET_INFO_IO_ID 0
#define GET_INFO_IO_SERIAL1 1
#define GET_INFO_IO_SERIAL2 2
#define GET_INFO_IO_PORT 3
#define GET_INFO_IO_NODATA 4

int inoutInit(stGetRadar_Data_t g_Radar_Data[],XK_UARTHandle_t XK_UARTHandle[], int nShearBufferIdx[],short usbStatus[]);
int inoutDeinit();

void makeJsonInoutResult(char * data,int Nset);
int  makeUIInoutResult(char * data, int Nset);

int isEnable_Inout();
int getNumofSet_Inout();
int getNoData_Inout(int Nset);
float getRadarInfo_Inout(int infoN, int numR, int Nset);
int cmd_Inout_system_legacy(float RRxdata[],int Nset,int len);


// typedef struct { // TODO: for connection of PC
// 	char header[4];
// 	float f_status;
// 	float breathingRate;
// 	float breathingRateFlag;
// 	float heartRate;
// 	float heartRateFlag;
// 	float movement;
// 	float humanPresence;
// 	float noMovementTime;
// 	float f_cursor;
// 	float radius;
// 	float led_on_off;
// 	float sensitivityAnalog;
// 	float autoSensitivity_on_off;
// 	float autoSensitivityIndex1;
// 	float autoSensitivityIndex2;
// 	float autoSensitivityRemainingTime;
// 	float Rheight[2];
// 	float ID[2];
// 	char ender[4];
// } sendPCInOutmsg_t;
///////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif

