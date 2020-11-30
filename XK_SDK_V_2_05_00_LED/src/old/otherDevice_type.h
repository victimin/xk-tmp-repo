#ifndef __OTHER_DEVICE_TYPE_H__
#define __OTHER_DEVICE_TYPE_H__


typedef struct {
	XK_UARTHandle_t * UARTHandle_Temp;
    pthread_t * Data_Thread_t;
	// stGetRadar_Data_t *g_Data;
	// int *nShearBufferIdx;
    int *appNum[MAX_NUM_USB_DEVICE];
    short* usbStatus;
    short* usbStatus_http;
} initDataSetOD_t;

int fps_forCheck[MAX_NUM_USB_DEVICE];

typedef struct {
    int fd;
    char *rcvbuf;
    int rcvlen;
    char *sndbuf;
    int sndlen;
} connectPC_t;


#endif