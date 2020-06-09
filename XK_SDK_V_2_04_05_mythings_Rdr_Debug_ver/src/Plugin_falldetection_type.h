#ifndef __FALL_DETECTION_TYPE_H__
#define __FALL_DETECTION_TYPE_H__

#define PATH_SAVE "/var/log/xk/"

#define EMERGENCY_STATUS 11 // To do: Change the value.
#define FALLOFF_STATUS 10 // To do: Change the value.

#define SERIAL_1 24
#define SERIAL_2 25
#define SERIAL_USER1 26
#define SERIAL_USER2 27

#define ResP0 49
#define ResP1 30
#define ResP2 28
#define ResP3 31
#define ResP4 29
#define ResP5 20
#define ResP6 15
#define ResP7 44
#define ResP8 59
#define ResP9 51
#define ResP10 11

#define Para0 15
#define Para1 23
#define Para2 34
#define Para3 20
#define Para4 61
#define Para5 13
#define Para6 47
#define Para7 48
#define Para8 44
#define Para9 14
#define Para10 49
#define Para11 51
#define Para12 11
#define NUM_PARA_ADD 70

#define PAIR_N 32

static int ResRadarP[] = {
	Para6,
	Para7,
	Para8,
	Para9,
	Para10,
	Para11,
};

static int SendtoRadarP[] = {
	Para0,
	Para1,
	Para2,
	Para3,
	Para4,
};

static int forServerMsg[] = {
	ResP0,
	ResP1,
	ResP2,
	ResP3,
	ResP4,
	ResP5,
	ResP6,
	ResP7,
	ResP8,
	ResP9,
	ResP10,
};

#define	ResUI0	49
#define	ResUI1	30
#define	ResUI2	28
#define	ResUI3	31
#define	ResUI4	29
#define	ResUI5	20
#define	ResUI6	15
#define	ResUI7	59
#define	ResUI8	51
#define	ResUI9	8
#define	ResUI10	9
#define	ResUI11	10
#define	ResUI12	16
#define	ResUI13	4
#define	ResUI14	17
#define	ResUI15	14
#define	ResUI16	6

#define R_HEIGHT 34
#define RELAY_MSG_LEN (sizeof(SendtoRadarP)/4*2+3)

typedef struct {
    int *fd;
	int port;

    int RadarID;
    int NumData;
    // int *appNum;
    
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
	// float RRadius; // TODO: for connection of PC
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


#define BUFF_SIZE 1024
typedef struct {
//    long  data_type;
//    int   data_num;
//    char  data_buff[BUFF_SIZE];
   float a36_Fall_Flag;
   float F_res_Final;
   float br_res;
   float bst_res;
   float hr_res;
   float hst_res;
   float mvi;
   float Pres_res;
   float testdata;
   float position;
   float NoMVPeriod;
   float f_cursor;
   float vitalScore;
} msg_Result_data_t;



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