#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>
#include <stdlib.h>
#include <termios.h> /*termio.h for serial IO api*/
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BUFF_LEN     (100)
#define MAX_STR_LEN     (512)
#define UART_BAUDRATE   (B115200)
#define UART_TIMEOUT    (1)
#define PACKET_SZ       (9)

void UART_init(int * fd, char *argv, short *usbStatus);
void WriteDataECU2IVI(
        unsigned char Soh, 
        unsigned char Req, 
        unsigned char GrpID, 
        unsigned char CmdID,
        unsigned char Payload_upp, 
        unsigned char Payload_low, 
        unsigned char Eot
);
int SetInterfaceAttribs(int fd, short *usbStatus, int speed, int parity, int waitTime);

#ifdef __cplusplus
}
#endif


#endif
