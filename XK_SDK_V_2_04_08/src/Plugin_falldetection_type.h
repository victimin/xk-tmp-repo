#ifndef __FALL_DETECTION_TYPE_H__
#define __FALL_DETECTION_TYPE_H__

#define PATH_SAVE "/var/log/xk/"

#define EMERGENCY_STATUS 				11
#define FALLOFF_STATUS 					10

#define F_CURSOR 						51
#define F_STATUS 						49
#define BREADTHING_RATE 				30
#define BREADTHING_SCORE 				28
#define HEART_RATE 						31
#define HEART_SCORE 					29
#define MOVEMENT_Q 						20
#define NO_MOVEMENT_PERIOD 				59
#define HUMAN_PRESENCE 					15
#define LED_ONOFF 						8
#define ANALOG_SENSITIVITY 				9
#define AUTO_SENSITIVITY_ONOFF 			10
#define AUTO_SENSITIVITY_INDEX 			16
#define AUTO_SENSITIVITY_REMAING_TIME 	17
#define R_HEIGHT 						34
#define MOVEMETN_SCORE 					11
#define RADAR_STATUS 					14
#define F_RES_POSITION 					44
#define F_ID_LOWER_RADAR 				47
#define F_ID_HIGHER_RADAR 				48
#define BED_EXT_STATUS	 				75
#define FIDGETING_STATUS 				76
#define RADAR_ENDRANGE 					6
#define RADAR_DEV_ID 					4

#define SERIAL_1 		24
#define SERIAL_2 		25
#define SERIAL_USER1 	26
#define SERIAL_USER2 	27

#define F_RELAY_DATA_0 	23
#define F_RELAY_DATA_1 	61
#define F_RELAY_DATA_2 	68

#define PAIR_N 			32

static int SendtoRadarP[] = {
	HUMAN_PRESENCE,
	F_RELAY_DATA_0,
	R_HEIGHT,
	MOVEMENT_Q,
	F_RELAY_DATA_1,
	F_RELAY_DATA_2,
};

#define RELAY_MSG_LEN (sizeof(SendtoRadarP)/4*2+3)

typedef struct {
    int *fd;
	int port;

    int RadarID;
    int NumData;
    
    float *data[GETDATA_BUFFER_NUMBER];
    int *BufferIdx;

	int NumPair;
	int *cntNodata;

	float EachR;
} rserial_t;

typedef struct {
    rserial_t r[NUM_RADAR_FALL];
    float Send2Rbuf[RELAY_MSG_LEN];
	int msglen;
	float RRadius;
} result_t;


typedef struct {
	XK_UARTHandle_t * UARTHandle_Temp;
	stGetRadar_Data_t *g_Data;
	int *nShearBufferIdx;
    int *appNum[MAX_NUM_USB_DEVICE];
    short* usbStatus;

	int NumSystem;
} initDataSet_t;


typedef enum{
  CMD_F_UI_END_RANGE = 1,
  CMD_F_UI_LED,
  CMD_F_UI_DEFAULT_SET,
  CMD_F_UI_CALIBRATION,
  CMD_F_UI_AUTO_S_ONOFF,
  CMD_F_UI_RADAR_H_1,
  CMD_F_UI_RADAR_H_2,
  CMD_F_UI_CHG_ID_BOTH1,
  CMD_F_UI_CHG_ID_BOTH2,
} RECV_CMD_FALL_t;

typedef struct {
    int fd;
	int port;
    int cursor;
	int status;

	char func_ver[100];

	int justSpace;

    int RadarID;
    int NumData;
	
    int DataRefreshF;

	unsigned long timeout;
	unsigned long timeout_cnt;
    
    float para[62+10];

	unsigned long currTime;

	float EachR;

} rSaveData_t; // legacy

typedef struct {
    int Radar1;
    int Radar2;
	
  	int SelR;
	
	int justSpace;
	int msglen;

	float RRadius;

	unsigned long currTime;

	int Buffer[10];
} fSaveData_t; // legacy


#endif