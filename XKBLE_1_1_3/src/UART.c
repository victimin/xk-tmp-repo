#include "UART.h"
#include <stdio.h>
#include <errno.h>

#define SIZE_OF_DATA    10

unsigned int recvCntIdx = 0;
unsigned int recvCntIdx2 = 0;
unsigned int recvCntIdx3 = 0;
unsigned char flgCheckXAKA =0;
unsigned char flgReadyToSave =0;
unsigned char flgCheckToSave =0;
FILE *fd0;

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
}

void UART_init(int * fd, char *argv, short *usbStatus)
{
    int k,i;
    char deviceName[MAX_STR_LEN];

    memset(&deviceName[0], 0, MAX_STR_LEN);
    snprintf(&deviceName[0], MAX_STR_LEN, argv);

    k = 1;

    if(*usbStatus == -1) *fd = open(&deviceName[0], O_RDWR | O_NOCTTY);
    if (0 > *fd)
    {
        //     perror(&deviceName[0]);
        //     exit(-1);
    }/*if */ 
    else{
        *usbStatus = 0;
        SetInterfaceAttribs_(*fd, usbStatus, UART_BAUDRATE, 0, UART_TIMEOUT); 
        // set_blocking(*fd, 0);
    }
}

void UartInterruptHandler(int fd, int status)
{
        int i, j;
        unsigned short chksum, tmpChksum, tmpTimeoutValue;
        unsigned char test_in_gest = 5;
        char read1stBuff[MAX_BUFF_LEN][MAX_STR_LEN];
        char readBuff[MAX_STR_LEN];
        char intTmpBuff[MAX_STR_LEN];
        char data[512];
        char realData[512];
        char tmpBuff[SIZE_OF_DATA], tmpIdxCnt;
        ssize_t len;
        unsigned int readCnt_x, readCnt_y, rcvDataSize;
        unsigned char rcvDataLen[MAX_BUFF_LEN];

        readCnt_x=0;    readCnt_y=0;
        tmpIdxCnt = 0;
        chksum = 0;
        memset(&intTmpBuff[0], 0, MAX_STR_LEN);
        // len = read(fd, &intTmpBuff[0], MAX_STR_LEN);
        len = read(fd, &intTmpBuff[0], MAX_STR_LEN);
        // recvCntIdx += len;
        for (i = 0; i < len; i++)
                fprintf(fd0,"%d ",intTmpBuff[i]);
}



int SetInterfaceAttribs_(int fd, short *usbStatus, int speed, int parity, int waitTime)
{
        int isBlockingMode;
        struct termios tty;
        struct sigaction saio;

        isBlockingMode = 0;
        if (waitTime < 0 || waitTime > 255)
                isBlockingMode = 1;

        memset(&tty, 0, sizeof tty);
        if (tcgetattr(fd, &tty) != 0) /* save current serial port settings */
        {
                *usbStatus = -1;
                close(fd);
                printf("__LINE__ = %d, error %s\n", __LINE__, strerror(errno));
                return -1;
        }

#if 0   //for interrupt
        saio.sa_handler = UartInterruptHandler;
        saio.sa_flags = 0;
        saio.sa_restorer = NULL;
        sigaction(SIGIO, &saio, NULL);

        fcntl(fd, F_SETFL, FNDELAY);
        fcntl(fd, F_SETOWN, getpid());
        fcntl(fd, F_SETFL, O_ASYNC); /**<<<<<<------This line made it work.**/
#endif
        cfsetospeed(&tty, speed);
        cfsetispeed(&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;                                 // disable break processing
        tty.c_lflag = 0;                                        // no signaling chars, no echo,
                                                                // no canonical processing
        tty.c_oflag = 0;                                        // no remapping, no delays
        tty.c_cc[VMIN] = (1 == isBlockingMode) ? 1 : 0;         // read doesn't block
        tty.c_cc[VTIME] = (1 == isBlockingMode) ? 0 : waitTime; // in unit of 100 milli-sec for set timeout value

        tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);   // ignore modem controls,
                                           // enable reading
        tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        // tty.c_cc[VMIN] =        1; /* Read at least 10 characters */
        // tty.c_cc[VTIME] =       1; /* Wait indefinetly   */
        // tty.c_cc[VMIN] =        0; /* Read at least 10 characters */
        // tty.c_cc[VTIME] =       1; /* Wait indefinetly   */

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
                printf("__LINE__ = %d, error %s\n", __LINE__, strerror(errno));
                return -1;
        }
        return 0;
} /*SetInterfaceAttribs_*/



