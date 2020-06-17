#ifndef __PLUGIN_FALL_DETECTION_H__
#define __PLUGIN_FALL_DETECTION_H__

#ifdef __cplusplus
extern "C"
{
#endif
///////////////////////////////////////////////////////////////////

#define FPS_WMFALL (5)

#define WMFALL_MAX_N 2
#define NUM_RADAR_FALL 2

typedef enum {
	GET_INFO_F_ID,
	GET_INFO_F_SERIAL1,
	GET_INFO_F_SERIAL2,
	GET_INFO_F_SERIAL_USER1,
	GET_INFO_F_SERIAL_USER2,
	GET_INFO_F_PORT,
	GET_INFO_F_NODATA,
	GET_INFO_F_SELR,
	GET_INFO_F_SELR_HEART,
	GET_INFO_F_SELR_BREATHING,
} getDataFromFALL;

int wmFallinit(stGetRadar_Data_t g_Radar_Data[],XK_UARTHandle_t XK_UARTHandle[], int nShearBufferIdx[],short usbStatus[]);
int wmFallDeinit();

void makeJsonFallResult(char * data,int Nset);
void makeUIFallResult(char * data,int Nset);

int isEnable_fall();
int getNumofSet_fall();
int getNoData_fall(int Nset);
float getRadarInfo_fall(int infoN, int numR, int Nset);
int cmd_fall_system_legacy(float RRxdata[],int Nset);
int getRdataBuf(int Nset,int SelR, float** outbuf);



typedef struct { // TODO: for connection of PC
	char header[4];
	float f_status;
	float breathingRate;
	float breathingRateFlag;
	float heartRate;
	float heartRateFlag;
	float movement;
	float humanPresence;
	float noMovementTime;
	float f_cursor;
	float radius;
	float led_on_off;
	float sensitivityAnalog;
	float autoSensitivity_on_off;
	float autoSensitivityIndex1;
	float autoSensitivityIndex2;
	float autoSensitivityRemainingTime;
	float Rheight[2];
	float ID[2];
	float bedExitStatus;
	float fidgetingStatus;
	char ender[4];
} sendPCFallmsg_t;

///////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif

