#ifndef __PLUGIN_INOUT_COUNTING_TYPE_H__
#define __PLUGIN_INOUT_COUNTING_TYPE_H__

#define R_INDEX_OFFSET 4

#define SERIAL_1 26
#define SERIAL_2 27

#define RESULT_IN 27
#define RESULT_OUT 28
#define RESULT_UPDATE_COUNTER 32

#define PARA_PERIOD_N 36
#define PARA_IDX_NUM_N 6

// #define ResP2 28
// #define ResP3 31
// #define ResP4 29
// #define ResP5 20
// #define ResP6 15
// #define ResP7 44
// #define ResP8 59
// #define ResP9 51
#define ParaCursor 15

#define Para0 4
#define Para1 5
#define Para2 6
// #define Para3 20
// #define Para4 61
// #define Para5 13
// #define Para6 47
// #define Para7 48
// #define Para8 44
// #define Para9 14
// #define Para10 49
// #define Para11 51
#define NUM_PARA_ADD 70

#define PAIR_N 32

// static int ResRadarP[] = {
// 	Para6,
// 	Para7,
// 	Para8,
// 	Para9,
// 	Para10,
// 	Para11,
// };

static int SendtoRadarP[] = {
	Para0,
	Para1,
	Para2,
	// Para3,
	// Para4,
};

// static int forServerMsg[] = {
// 	ResP0,
// 	ResP1,
// 	ResP2,
// 	ResP3,
// 	ResP4,
// 	ResP5,
// 	ResP6,
// 	ResP7,
// 	ResP8,
// 	ResP9,
// };

// #define	ResUI0	49
// #define	ResUI1	30
// #define	ResUI2	28
// #define	ResUI3	31
// #define	ResUI4	29
// #define	ResUI5	20
// #define	ResUI6	15
// #define	ResUI7	59
// #define	ResUI8	51
// #define	ResUI9	8
// #define	ResUI10	9
// #define	ResUI11	10
// #define	ResUI12	16
// #define	ResUI13	4
// #define	ResUI14	17
// #define	ResUI15	14
// #define	ResUI16	6

// #define R_HEIGHT 34
#define RELAY_S_LENGTH_MAX (86)//(250)
#define RELAY_MSG_LEN ((6+RELAY_S_LENGTH_MAX*2)*4)//(sizeof(SendtoRadarP)/4*2+3)

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
} rserial_inout_t;

typedef struct {
    rserial_inout_t r[NUM_RADAR_INOUT];
    float Send2Rbuf[RELAY_MSG_LEN];
	int msglen;
	// float RRadius; // TODO: for connection of PC
	float RRadius;
} result_inout_t;


typedef struct {
	XK_UARTHandle_t * UARTHandle_Temp;
	stGetRadar_Data_t *g_Data;
	int *nShearBufferIdx;
    int *appNum[MAX_NUM_USB_DEVICE];
    short* usbStatus;

	int NumSystem;
} initDataSet_t;


// #define BUFF_SIZE 1024
// typedef struct {
// //    long  data_type;
// //    int   data_num;
// //    char  data_buff[BUFF_SIZE];
//    float a36_Fall_Flag;
//    float F_res_Final;
//    float br_res;
//    float bst_res;
//    float hr_res;
//    float hst_res;
//    float mvi;
//    float Pres_res;
//    float testdata;
//    float position;
//    float NoMVPeriod;
//    float f_cursor;
// } msg_Result_data_t;


// typedef enum{
//   CMD_F_UI_END_RANGE = 1,
//   CMD_F_UI_LED,
//   CMD_F_UI_DEFAULT_SET,
//   CMD_F_UI_CALIBRATION,
//   CMD_F_UI_AUTO_S_ONOFF,
//   CMD_F_UI_RADAR_H_1,
//   CMD_F_UI_RADAR_H_2,
//   CMD_F_UI_CHG_ID_BOTH1,
//   CMD_F_UI_CHG_ID_BOTH2,
// } RECV_CMD_FALL_t;

#endif