#ifndef __XK_USB_H__
#define __XK_USB_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdio.h>
#include "XK_CommonDefineSet.h"

#define USB_DEV_PATH                    "/sys/bus/usb/devices/"
#define USB_DEV_BUS_PATH                "/dev/bus/usb"
#define USB_DEV_HUB_PATH                "\:1.0"
#define USB_DEV_DEVNUM_FILE             "devnum"
#define USB_DEV_BUSNUM_FILE             "busnum"
#define USB_DEV_ADAPTER_START           (1u)
#define USB_DEV_ADAPTER_END             (5u)
#define USB_DEV_MAIN_ADAPTER_SIZE       USB_DEV_ADAPTER_END - USB_DEV_ADAPTER_START + 1
#define USB_DEV_PORT_MAX_SIZE           30
#define USB_DEV_HUB_ADAPTER_SIZE        MAX_NUM_USB_DEVICE

typedef struct
{
        int busnum;
        int devnum;
        int portnum;

        int hubN;

        int hubNum[10];
} XK_USBinfo_t;

typedef struct
{
        XK_USBinfo_t USBinfo;
        int mainHubNum;
        int subHubNum;
} XK_USBinfoBySymlink_t;

int XK_USB_Get_Devnum(int *hubArray, int deep);
int XK_USB_Get_Busnum(int *hubArray, int deep);
int XK_USB_Get_Portnum(char * path);
int XK_USB_Get_Devnum3(int mainHubNum, int subHubNum, int subSubHubNum);
int XK_USB_Get_Busnum3(int mainHubNum, int subHubNum, int subSubHubNum);
int XK_USB_Get_Portnum3(int mainHubNum, int subHubNum, int subSubHubNum);
int XK_USB_Get_Devnum4(int mainHubNum, int subHubNum, int subSubHubNum, int subSubSubHubNum);
int XK_USB_Get_Busnum4(int mainHubNum, int subHubNum, int subSubHubNum, int subSubSubHubNum);
int XK_USB_Get_Portnum4(int mainHubNum, int subHubNum, int subSubHubNum, int subSubSubHubNum);


#ifdef __cplusplus
}
#endif


#endif
