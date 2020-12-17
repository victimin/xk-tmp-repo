#ifndef __RADARCOMMAND_H__
#define __RADARCOMMAND_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "XK_CommonDefineSet.h"
#include "parson.h"

#define RADAR_CMD_Q_MAX         (10u)
#define RADAR_CMD_PARAM_MAX     (100u)

typedef union {
	char c[512];
	int i[128];
	float f[128];
} Sendmsg_t;


enum COMMAND_TARGET_t {
   CMD_TO_RADAR = 0,
   CMD_TO_RPI,
   CMD_TO_MAX_NUM
};

enum RPI_CMD_t {
   RPI_CMD_CHG_RCV_DATA = 0,
   RPI_CMD_OTA_RADAR,
   RPI_CMD_RID_FORCE_CHANGE,
   RPI_CMD_RID_TYPE_CHANGE,
   RPI_CMD_DATA_TYPE_CHANGE,
   RPI_CMD_SYSTEM_SCRIPT,
   RPI_CMD_TEAMVEIWER_RESET,
   RPI_CMD_WIFI_RESET,
   RPI_CMD_SYSTEM_REBOOT,
   RPI_CMD_MAX_NUM
};

typedef struct
{
   double m_fRcvParamQ[RADAR_CMD_Q_MAX][RADAR_CMD_PARAM_MAX];               // 
   int m_nParamSZ[RADAR_CMD_Q_MAX];
   int m_nFrontIdx;
   int m_nRearIdx;
   int m_nNum;
} ParamQ_Handle_t;

typedef struct
{
   int ID;
   int port;
} TartgetRadar_t;

void RadarCommandInit(ParamQ_Handle_t handle);
void RadarCommandUART(int fd, Sendmsg_t * arrParam, int numParam);
void CommandParser(XK_HTTPHandle_t * HTTPHandle, JSON_Array *array);

#ifdef __cplusplus
}
#endif


#endif

