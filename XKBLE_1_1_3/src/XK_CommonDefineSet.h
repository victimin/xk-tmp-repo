#ifndef __COMMON_DEFINE_SET_H__
#define __COMMON_DEFINE_SET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#include "XK_ble_parser.h"
#include "XK_thread.h"
#include "ble_hci.h"
#include "XK_tools.h"
#include "logging.h"


/*********************************************** 
 *  Version Info section
************************************************/
#define XKBLE_VERSION                     "1.1.3"


/*********************************************** 
 *  Common
************************************************/
//Data
#define BYTES                           1024
#define KB                              (1024)
#define MB                              (1024 * KB)
#define GB                              (1024 * MB)

//Time
#define NS                              (1u)
#define US                              (1000 * NS)
#define MS                              (1000 * US)
#define SIZE_NSEC                       (1000000000)


/*********************************************** 
 *  Parameter setting section
************************************************/
#define NUM_RCV_PARA                    400
#define NUM_THREAD                      2
#define MAX_NUM_CPU_CORE                4
#define MAX_PRINT_SIZE                  200
#define PRINT_COL                       4
#define MAX_DEVICE_NUM                  50

#define WAIT_GET_TIME_OUT               (5u)
#define WAIT_NETWORKING_TIME_OUT        (5u)    //minute
#define IBCON_TIMEOUT_SEC               (30)    //second



/*********************************************** 
 *  Command header
************************************************/
#define RPI_CMD_HEADER                  (4040.4040f)
#define RPI_CMD_FOOTER                  (255.255f)


/*********************************************** 
 *  config
************************************************/
#define CONFIG_PATH                     "/boot/XandarKardian/config.json"
#define GATEWAY_OTA_PATH                "/boot/XandarKardian/tools/ota"
#define MYPID_PATH                      "/etc/xkble/pre-pid"
//PROCESS
#define XK_PROCESS_ID                   "XK_BLE_SEMAPHORE"


/*********************************************** 
 *  API
************************************************/
#define PAGE_ITEM_GET                   "/api/v1/item-get"
#define PAGE_RADAR_CMD                  "/api/v1/radar-cmd"


/*********************************************** 
 *  logging
************************************************/
#define LOG_PATH                        "/var/log/xk/xkble"
#define SYS_LOG_PATH                    "/var/log/xk/xkble/sys/syslog_%04d%02d%02d.xkl"
#define LOG_NAME                        "XK_log_%04d%02d%02d.csv"
#define LOG_INFO                        "* devie: %s\n* function: %s\n* client: %s\n* pi-serial: %s\n* host: %s\n* page: %s\n* port: %d\n\n*** This file was created on %s %02d %02d:%02d:%02d. \n\n"


/*********************************************** 
 *  Edefine
************************************************/
#define ESUCCESS                        0
#define ENOPROC                         1
#define ENONAME                         2
#define MAX_PROCESS_NAME                512

/*********************************************** 
 *  system
************************************************/
#define MAX_NUM_USB_DEVICE              20

/*********************************************** 
 *  system
************************************************/
#define MAC_E_PATH                      "/sys/class/net/eth0/address"
#define MAC_W_PATH                      "/sys/class/net/wlan0/address"


/*********************************************** 
 *  enum
************************************************/
typedef enum
{
        OFF,
        ON
} XK_ONOFF;


typedef enum
{
        XK_HOST_TYPE_DOMAIN = 0,
        XK_HOST_TYPE_IPADDR,
        XK_HOST_TYPE_NONE,
} XK_HOST_TYPE;


/*********************************************** 
 *  struct
************************************************/
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
        char address[50];
        char topic[100];
        char username[50];
        char password[50];
        char cert[50];
        int mqttPort;
        int frequency;
} XK_MQTTinfo_t;

typedef struct
{
        //system
        char cpuSerial[20];
        char cpuHardware[20];
        char MAC_e_Addr[20];
        char MAC_w_Addr[20];

        //config
        int port;
        char host[100];
        char page[100];
        char device[100];
        char auth[100];
        char accID[100];
        char hardware[20];
        char serial[20];

        //info
        char name[20];
        char version[20];
        char function[30];
        char client[100];

        int host_type;
        int server_port;

        char radarDeviceList[MAX_DEVICE_NUM][20];
        int radarDeviceNum;
        
        char mode[20];
        int info_send_period;
        int info_SIC_interval;
        char http_onoff;
        char mqtt_onoff;
        char behr_myts_onoff;
        char flgSystemLog;
        char flgDataLog;
        char flgAutoReboot;
        char typeData;
        char typeRID;
        char isNewWorld;
        int send_if_changed;
        XK_MQTTinfo_t MqttInfo;
       
} XK_ConfigInfo_t;

typedef struct
{
        int rcvDataSize[MAX_DEVICE_NUM];
        unsigned int appNum[MAX_DEVICE_NUM];
        char serialNum[MAX_DEVICE_NUM][20];
        int radarID[MAX_DEVICE_NUM];
        float rcvdRadarData[MAX_DEVICE_NUM][NUM_RCV_PARA];
        short USBStatus[MAX_DEVICE_NUM];
        unsigned int cntNodata[MAX_DEVICE_NUM];
        int fd[MAX_DEVICE_NUM];
        int idxApp[MAX_DEVICE_NUM];
        char flgParamSizeErr;
        char flgSIC;
        
        int memHeap;
        int cpuUsage;
        double piTemperature;
        char optDisp;
        char bootedTime[100];
        char MAC_e_Addr[20];
        char MAC_w_Addr[20];
        char IPAddr_e[25];
        char IPAddr_w[25];
        XK_ConfigInfo_t * info;
} XK_HTTPHandle_t;



#define XK_USB_CMD_GET_INFO                  "INFO"
#define XK_USB_CMD_IS_CONNECTED              "CNT?"
#define XK_USB_CMD_STRAT                     "STRT"
#define XK_USB_CMD_EXIT                      "EXIT"
#define XK_USB_CMD_DATA                      "DATA"
#define XK_USB_CMD_DONE                      "DONE"
#define MAX_STR_LEN                          100


typedef enum
{
        USB_DATA_TYPE_NORMAL = 0,
        USB_DATA_TYPE_COMMAND,
} XK_USB_DATA_TYPE;


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


/*********************************************** 
 *  Meta Data
************************************************/
stIBconData iBconDate[MAX_DEVICE_NUM];
XK_ConfigInfo_t configInfo;
XK_HTTPHandle_t httpHandle;

struct tm gLocalTime;
struct tm gUtcTime;

char logo[4000];

#ifdef __cplusplus
}
#endif
#endif


