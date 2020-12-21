#ifndef __RADAR_H__
#define __RADAR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "XK_CommonDefineSet.h"

int Get_XKRadar_Data_Packet(XK_UARTHandle_t *XK_UARTHandle, stGetRadar_Data_t * g_Radar_Data, unsigned int * idxFront, unsigned int *preIdxXAKA, unsigned int *curIdxXAKA, unsigned char numOfUSB);

#ifdef __cplusplus
}
#endif


#endif

