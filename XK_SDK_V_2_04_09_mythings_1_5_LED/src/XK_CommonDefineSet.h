#ifndef __COMMON_DEFINE_SET_H__
#define __COMMON_DEFINE_SET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <sys/time.h>
#include "XK_tools.h"
#include "XK_RadarApplicationInfo.h"

/*********************************************** 
 *  Version Info section
************************************************/
#define SDK_VERSION                     "2.04.09.mythings.1.5_LED"
#define API_VERSION                     "1.1"


/*********************************************** 
 *  Algorithm Plugin & Program ON/OFF section
************************************************/
#define FALL_ENABLE                     0
#define FALL_DEBUG_ENABLE               0

#define INOUT_ENABLE                    0
#define INOUT_DEBUG_ENABLE              0


/*********************************************** 
 *  Communication Module
************************************************/
#define MYTHINGS_ENABLE                 1


/*********************************************** 
 *  
************************************************/
#define MODE_DEBUG                      0
#define ONOFF_SUB_EPOINT                0
#define ONOFF_SEND_TO_GCP               0


/*********************************************** 
 *  Parameter setting section
************************************************/
// #define NUM_PARA_ADD                 100
#define NUM_RCV_PARA                    400
#define NUM_THREAD                      2
#define MAX_NUM_CPU_CORE                4
#define MAX_PRINT_SIZE                  200
#define PRINT_COL                       4
#define MAX_NUM_USB_DEVICE              20
// #define MAX_NUM_USB_DEVICE           20

#define RECV_BUFF_SIZE                  (3000U)

#define GETDATA_BUFFER_NUMBER           (10U)
#define PROCESS1_BUFFER_NUMBER          (5U)

#define CMD_NEW_DATA                    (0x0001)
#define WAIT_GET_TIME_OUT               (5u)
#define WAIT_NETWORKING_TIME_OUT        (5u)    //minute

#define HTTP_SEND_SIZE                  50
#define HTTP_SEND_ALL                   0
#define HTTP_SEND_CUST                  1
#define HTTP_SEND_TYPE                  HTTP_SEND_CUST

#define HTTP_HEADER                     "POST /%s HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n%s"
#define HTTP_STR_infoHeader             "{\
                                        \r\n\t\"client\":\"%s\", \
                                        \r\n\t\"function\":\"%s\", \
                                        \r\n\t\"device\":\"%s\", \
                                        \r\n\t\"pi-serial\":\"%s\", \
                                        \r\n\t\"free-mem\":%d, \
                                        \r\n\t\"cpu-usage\":%d, \
                                        \r\n\t\"temperature\":%3.3f, \
                                        \r\n\t\"start-date\":\"%s\""

//Command header
#define RADAR_CMD_HEADER                (1010.1010f)
#define RPI_CMD_HEADER                  (4040.4040f)
#define RPI_CMD_FOOTER                  (255.255f)

//config
#define CONFIG_PATH                     "/boot/XandarKardian/config.json"
#define PARAM_PATH                      "/boot/XandarKardian/param.json"
#define GATEWAY_OTA_PATH                "/boot/XandarKardian/tools/ota"
#define SCRIPT_FILE                     "/boot/XandarKardian/script/xk.sh"
#define MYPID_PATH                      "/etc/xksdk/pre-pid"

//logging
#define LOG_PATH                        "/var/log/xk/%s"
#define SYS_LOG_PATH                    "/var/log/xk/sys/syslog_%04d%02d%02d.xkl"
#define LOG_NAME                        "XK_log_%d_%04d%02d%02d.csv"
#define LOG_INFO                        "* devie: %s\n* function: %s\n* client: %s\n* pi-serial: %s\n* host: %s\n* page: %s\n* port: %d\n\n*** This file was created on %s %02d %02d:%02d:%02d. \n\n"

//Data
#define KB                              (1024)
#define MB                              (1024 * KB)
#define GB                              (1024 * MB)

//Time
#define NS                              (1u)
#define US                              (1000 * NS)
#define MS                              (1000 * US)
#define SIZE_NSEC                       (1000000000)

//PROCESS
#define XK_PROCESS_ID                   "XK_SEMAPHORE"

//Edef
#define ESUCCESS                        0
#define ENOPROC                         1
#define ENONAME                         2
#define MAX_PROCESS_NAME                512

//server
#define HTTP_SERVER_HEADER              "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n%s"
#define CONNMAX                         1000
#define BYTES                           1024
#define PAGE_ITEM_GET                   "/api/v1/item-get"
#define PAGE_RADAR_CMD                  "/api/v1/radar-cmd"

//system
#define MAC_E_PATH                      "/sys/class/net/eth0/address"
#define MAC_W_PATH                      "/sys/class/net/wlan0/address"

#if ONOFF_SUB_EPOINT
        #define SUB_E_POINT             "show.xandarkardian.com"
        #define SUB_E_PAGE              "sensor/jerry"
        #define SUB_PORT                (80)
#endif


#define XK_USB_CMD_GET_INFO                  "INFO"
#define XK_USB_CMD_IS_CONNECTED              "CNT?"
#define XK_USB_CMD_STRAT                     "STRT"
#define XK_USB_CMD_EXIT                      "EXIT"
#define XK_USB_CMD_DATA                      "DATA"
#define XK_USB_CMD_DONE                      "DONE"


// #define MAX_PARAM_SZ        200
// extern unsigned char HTTP_send_param[MAX_PARAM_SZ];
// extern unsigned char HTTP_send_switch[200];
extern int HTTP_send_switch[20][200];

typedef struct
{
        int *fd;
        int numUSRCMD;
        int devUSB;
        int radarID;
        int rcvDataSize;
        unsigned int appNum;
        unsigned int paramSize;
        unsigned int cntNodata;
        char prntOptn;
        char flgConnection;
        char flgDataAble;
        unsigned char *recvQueue;
        unsigned int CheckOneTime; // Check the having recieved data before no data states.
} XK_UARTHandle_t;

typedef struct
{
        //config
        int port;
        char host[100];
        char page[100];
        char device[100];
        char auth[100];
        char accID[100];
        char hardware[20];
        char serial[20];
        // int send_if_changed;

        //info
        char name[20];
        char version[20];
        char function[30];
        char client[100];

        int host_type;
        int server_port;        
} XK_HTTPinfo_t;

typedef struct
{
        int rcvDataSize[MAX_NUM_USB_DEVICE];
        unsigned int appNum[MAX_NUM_USB_DEVICE];
        char serialNum[MAX_NUM_USB_DEVICE][20];
        int radarID[MAX_NUM_USB_DEVICE];
        float rcvdRadarData[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];
        short USBStatus[MAX_NUM_USB_DEVICE];
        unsigned int cntNodata[MAX_NUM_USB_DEVICE];
        int fd[MAX_NUM_USB_DEVICE];
        int idxApp[MAX_NUM_USB_DEVICE];
        int memHeap;
        int cpuUsage;
        double piTemperature;
        char optDisp;
        char flgParamSizeErr;
        char flgSIC;
        char bootedTime[100];
        char MAC_e_Addr[20];
        char MAC_w_Addr[20];
        char IPAddr_e[25];
        char IPAddr_w[25];
        XK_HTTPinfo_t info;
} XK_HTTPHandle_t;

typedef struct
{
        int info_send_period;
        int info_SIC_interval;
        char mode[20];
        char flgSendData;
        char flgSystemLog;
        char flgDataLog;
        char flgAutoReboot;
        char typeData;
        char typeRID;
        int send_if_changed;

} XK_InitInfo_t;

typedef struct
{
        char host[100];
        char page[50];
        int port;
        int send_period;
        char data_type;
        char MAC_e_Addr[20];
        char MAC_w_Addr[20];
        char teamviewerID[11];
        char teamviewerVer[20];
        char xksdkVer[20];
} XK_UsbCmdData_t;

typedef enum
{
        GET_DATA_THREAD = 0x0000,
        SEND_TO_SEVER_THREAD = 0x0001
} XK_THREAD_ID;

typedef enum
{
        HTTP_DISP_OPT_SEND = 0x00,
        HTTP_DISP_OPT_RECV = 0x01,
} XK_HTTP_DISP_OPT;

typedef enum
{
        MAIN_OPT_DEFAULT = 0,
        MAIN_OPT_DEBUG,
        MAIN_OPT_RAW,
        MAIN_OPT_DEBUG_V,
        MAIN_OPT_HELP,
} XK_MAIN_OPTION;

typedef enum
{
        XK_HOST_TYPE_DOMAIN = 0,
        XK_HOST_TYPE_IPADDR,
        XK_HOST_TYPE_NONE,
} XK_HOST_TYPE;

typedef enum
{
        OFF,
        ON
} XK_ONOFF;

typedef enum
{
        USB_DATA_TYPE_NORMAL = 0,
        USB_DATA_TYPE_COMMAND,
} XK_USB_DATA_TYPE;


typedef struct
{
        float m_nALLRadarData[MAX_NUM_USB_DEVICE][NUM_RCV_PARA];
        int m_nGetFlag[MAX_NUM_USB_DEVICE];
        
}stGetRadar_Data_t;

typedef struct
{
        unsigned char m_nFrameImage[1280 * 720];
        int m_nFrameProcessFlag[PROCESS1_BUFFER_NUMBER];
        int m_nProcessBufferIdx;
        
}stProcess1_Data;


typedef struct
{
        long  m_nData_type;
        int   m_nCommand_type;
        int   m_nData_Buffer_Num;
        void *m_nMetaDataAddr;
}stMsgQueues;

typedef struct
{
        int m_nStatus;               // 
        int m_nCommand;              // 1 : Send 
        int m_nDestCore;        // 0 : CPU0, 1 : CPU1
//    void *m_nMetaDataAddr;         // Address
   stGetRadar_Data_t *m_nMetaDataAddr;         // Address
}stMsgHandler;

struct xktime
{
        int t_sec;  // 
        int t_msec; // 
};
    
// int Register_Callback(DrvCbParams *CbParams, CbFunction cbFxn, void *Arg1);


#ifdef __cplusplus
}
#endif


#endif


