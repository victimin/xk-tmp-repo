#ifndef __OTHER_DEVICE_H__
#define __OTHER_DEVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif
///////////////////////////////////////////////////////////////////
#include "otherDevice_type.h"

int otherDevHinit(XK_UARTHandle_t XK_UARTHandle[],short usbStatus[], pthread_t Get_XKRadar_Data_Thread_t[], XK_HTTPHandle_t *XK_HTTPHandle);
int otherDevHDeinit();


///////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif

