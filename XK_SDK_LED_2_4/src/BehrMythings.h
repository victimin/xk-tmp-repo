// #ifndef __BEHRMYTHINGS_H__
// #define __BEHRMYTHINGS_H__

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// // #include "XK_CommonDefineSet.h"

// #define MYTHINGS_MSG_SZ                     200
// #define MYTHINGS_MSG_SEND_LIMIT_SZ          110

// #define MYTHINGS_SEND_PERIOD                20  //sec

// #define PARAM_IDX_ZONE_PRE_STS              4
// #define PARAM_IDX_ZONE_NUM_PEOPLE           15
// #define PARAM_IDX_ZONE_LONG_T_IDX           7
// #define PARAM_IDX_ZONE_VER                  5
// #define PARAM_IDX_ZONE_RDR_MAP_IDX          25
// #define PARAM_IDX_ZONE_ENV_MAP_IDX          26

// #define PARAM_IDX_ZONE_COMBO_FINAL_CNT      46
// #define PARAM_IDX_ZONE_COMBO_NUM_ZONE_SENSOR     47
// #define PARAM_IDX_ZONE_COMBO_NUM_INOUT_SENSOR    48

// #define PARAM_IDX_INOUT_CONN_STS            6
// #define PARAM_IDX_INOUT_IN_CNT              14
// #define PARAM_IDX_INOUT_OUT_CNT             15
// #define PARAM_IDX_INOUT_UPDT_CNT            22
// #define PARAM_IDX_INOUT_VER                 7

// #define PARAM_IDX_PRES_PRES_STS             11
// #define PARAM_IDX_PRES_VER                  15

// #define PARAM_IDX_PREVITAL_PRES_STS         11
// #define PARAM_IDX_PREVITAL_BR               26
// #define PARAM_IDX_PREVITAL_HR               27
// #define PARAM_IDX_PREVITAL_MOVEMT           16
// #define PARAM_IDX_PREVITAL_STBLLT_BR        24
// #define PARAM_IDX_PREVITAL_STBLLT_HR        25
// #define PARAM_IDX_PREVITAL_VER              15

// ////SEND_PERIOD
// //common
// #define SEND_PERIOD_COMMON_VERSION          2000

// #define SEND_SEPARATOR_COMMON               ','

// #define SEND_CMD_COMMON_NORMAL              '0'
// #define SEND_CMD_COMMON_VERSION             '1'
// #define SEND_CMD_COMMON_ERROR               '9'

// #define SEND_CMD_COMMON_ERROR               '9'
// typedef enum
// {
//     SEND_ERROR_INVALID_APPNUM = 0,
//     SEND_ERROR_OVERFLOW_DATA  = 1,
//     SEND_ERROR_EMPTY  = 9,
// } SEND_ERROR;

// //single inout
// #define SEND_CMD_INOUT_NORMAL               SEND_CMD_COMMON_NORMAL
// #define SEND_CMD_INOUT_VERSION              SEND_CMD_COMMON_VERSION

// #define SEND_PERIOD_INOUT_NORMAL            1
// #define SEND_PERIOD_INOUT_VERSION           SEND_PERIOD_COMMON_VERSION
// // typedef enum
// // {
// //     SENDING_STATE_INOUT_NORMAL = 0,
// //     SENDING_STATE_INOUT_VER,
// //     SENDING_STATE_INOUT_ERROR
// // } SENDING_STATE_INOUT;

// //single presence
// #define SEND_CMD_PRES_NORMAL               SEND_CMD_COMMON_NORMAL
// #define SEND_CMD_PRES_VERSION              SEND_CMD_COMMON_VERSION

// #define SEND_PERIOD_PRES_NORMAL            1
// #define SEND_PERIOD_PRES_VERSION           SEND_PERIOD_COMMON_VERSION
// // typedef enum
// // {
// //     SENDING_STATE_PRES_NORMAL = 0,
// //     SENDING_STATE_PRES_VER,
// // } SENDING_STATE_PRES;

// // presence-vatial
// #define SEND_CMD_PREVITAL_NORMAL                SEND_CMD_COMMON_NORMAL
// #define SEND_CMD_PREVITAL_VERSION               SEND_CMD_COMMON_VERSION

// #define SEND_PERIOD_PREVITAL_NORMAL             1
// #define SEND_PERIOD_PREVITAL_VERSION            SEND_PERIOD_COMMON_VERSION
// typedef enum
// {
//     SENDING_STATE_PREVITAL_NORMAL = 0,
//     SENDING_STATE_PREVITAL_VER,
// } SENDING_STATE_PREVITAL;

// //single zone
// #define SEND_CMD_ZONE_NORMAL               SEND_CMD_COMMON_NORMAL
// #define SEND_CMD_ZONE_VERSION              SEND_CMD_COMMON_VERSION
// #define SEND_CMD_ZONE_MAP_VAL              '2'

// #define SEND_PERIOD_ZONE_NORMAL            1
// #define SEND_PERIOD_ZONE_MAP_VAL           200
// #define SEND_PERIOD_ZONE_VERSION           SEND_PERIOD_COMMON_VERSION
// // typedef enum
// // {
// //     SENDING_STATE_ZONE_NORMAL = 0,
// //     SENDING_STATE_ZONE_MAP_VAL,
// //     SENDING_STATE_ZONE_VER,
// // } SENDING_STATE_ZONE;

// // typedef enum
// // {
// //     MAIN_IDX_CMD = 0,
// //     MAIN_IDX_SERIAL = 1,
// // } MAIN_IDX;

// // typedef enum
// // {
// //     INOUT_IDX_CMD = 0,
// //     INOUT_IDX_SERIAL = 1,
// //     INOUT_IDX_APPNUM = 13,
// //     INOUT_IDX_CONN_STATUS = 15,
// //     INOUT_IDX_IN_CNT = 16,
// //     INOUT_IDX_OUT_CNT = 19,
// //     INOUT_IDX_UPDATE_CNT = 22,
// // } INOUT_IDX;

// // typedef enum
// // {
// //     PRESENCE_IDX_CMD = 0,
// //     PRESENCE_IDX_SERIAL = 1,
// //     PRESENCE_IDX_APPNUM = 13,
// //     PRESENCE_IDX_PRE_STATU = 15,
// // } PRESENCE_IDX;

// // typedef enum
// // {
// //     ZONE_IDX_CMD = 0,
// //     ZONE_IDX_SERIAL = 1,
// //     ZONE_IDX_APPNUM = 13,
// //     ZONE_IDX_PRE_STATU = 15,
// //     ZONE_IDX_ZONE_CNT = 16,
// //     ZONE_IDX_LONG_T_IDX = 19,
// // } ZONE_IDX;

// // typedef enum
// // {
// //     MAPPING_VAL_CMD = 0,
// //     MAPPING_VAL_SERIAL = 1,
// //     MAPPING_VAL_RADAR = 13,
// //     MAPPING_VAL_ENV = 17,
// // } MAPPING_VAL_IDX;

// // typedef enum
// // {
// //     VERSION_SECTION_CMD = 0,
// //     VERSION_SECTION_SERIAL = 1,
// // } VERSION_SECTION_IDX;


// #ifdef __cplusplus
// }
// #endif


// #endif

