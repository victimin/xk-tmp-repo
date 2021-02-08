#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<wiringPi.h>
#include<wiringSerial.h>
#include<stdint.h>
#include<errno.h>
#include <termios.h>

#include <pthread.h>
#include <sched.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include <termios.h> /*termio.h for serial IO api*/
#include <signal.h>
#include <fcntl.h>


/*********************************************** 
 *  Version Info section
************************************************/
#define VERSION                     "2.0"


#define MAX_BUFF_LEN     (100)
#define MAX_STR_LEN     (512)
#define UART_BAUDRATE   (115200)
#define UART_TIMEOUT    (20)
#define PACKET_SZ       (9)
#define SIZE_OF_DATA    10

char *help = 
"================ How to execute XK-SDK ================\n"
"   -default:   s2r [port] [payload]\n"
"   -b:         s2r [baudrate] [port] [payload]\n"
"   -c:         char\n"
"   -d:         int\n"
"   -f:         float\n"
"   -v:         version\n"
"=======================================================\n";


FILE *fd0;

typedef union {
	char c[256];
	int i[64];
	float f[64];
} sendmsg_t;
sendmsg_t sendmsg;

// char device[]="/dev/ttyACM0";
char device[100]="/dev/ttyACM";

short usbstatus;
int fd;
char gStrBaudRate[10];
int gBaudRate=UART_BAUDRATE;
int gCnvtdBaudRate = 0;
int gPortNum;
// char gPortNum[4];
int gIdxPayload=0;

void UART_init(int * fd, char *argv);
int SetInterfaceAttribs(int fd, int speed, int parity, int waitTime);

int main(int argc, char *argv[]){
    int param_opt;
    char tmpCmdLine[100];
    int prePid_i=0;
    char* tmpPtr;

        if(argc > 1){
                if(strcmp(argv[1],"-v")==0){        
                        printf("- s2r version: %s\nCopyright(c) 2020. XandarKardian. All rights reserved.\n", VERSION);
                        return 0;
                }
                else if(strcmp(argv[1],"-h")==0){
                        printf("%s\n", help);
                        return 0;
                }
                else if(strcmp(argv[1],"-b")==0){
                        gBaudRate = atoi(argv[2]);
                        printf("Baudrate: [%d]         Port: [%d]", gBaudRate, gPortNum);
                        if(gBaudRate % 9600 != 0) {
                                printf("Wrong baudrate\n");
                                exit(1);
                        }
                        gPortNum = atoi(argv[3]);
                        gIdxPayload = 4;
                }
                else {
                        printf("Baudrate: [%d]         Port: [%d]", gBaudRate, gPortNum);
                        // gBaudRate = atoi(argv[1]);
                        gPortNum = atoi(argv[1]);
                        gIdxPayload = 2;
                }
        }

        switch( gBaudRate )
        {
                case 921600 : gCnvtdBaudRate |= B921600; break;
                case 115200 : gCnvtdBaudRate |= B115200; break;
                case 57600 : gCnvtdBaudRate |= B57600; break;
                case 38400 : gCnvtdBaudRate |= B38400; break;
                case 19200 : gCnvtdBaudRate |= B19200; break;
                case 9600 : gCnvtdBaudRate |= B9600; break;
                case 4800 : gCnvtdBaudRate |= B4800; break;
                case 2400 : gCnvtdBaudRate |= B2400; break;
                default : gCnvtdBaudRate |= B115200; break;
        }

        int loopi, loopj;
        fflush(stdout);

        sprintf(device, "/dev/ttyACM%d", gPortNum);
        printf("        device: [%s]\n", device);

        UART_init(&fd, device);

        if(fd == -1){
                printf("%s invalid device\n", device);
                exit(1);
        }

        int LengthMsg = argc-gIdxPayload;
        for(loopi=0;loopi<LengthMsg;loopi++){
                sendmsg.f[loopi] = atof(argv[loopi+gIdxPayload]);
        }

        for(loopi=0 ; loopi<LengthMsg ; loopi++){
                printf("%2d:%f\n",loopi,sendmsg.f[loopi]);

                for(loopj=0;loopj<4;loopj++) 
                {
                        printf("  %2d:%d(%02x)\n", loopi*4+loopj, sendmsg.c[loopi*4+loopj], sendmsg.c[loopi*4+loopj]);
                }
                write(fd, &sendmsg.c[0], LengthMsg*4);
        }
        return 0;
}

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

void UART_init(int * fd, char *argv)
{
        int k,i;
        char deviceName[MAX_STR_LEN];

        memset(&deviceName[0], 0, MAX_STR_LEN);
        snprintf(&deviceName[0], MAX_STR_LEN, argv);

        k = 1;

        *fd = open(&deviceName[0], O_RDWR | O_NOCTTY);
        if (0 > *fd)
        {
                printf("[%s] Open error!!!!\n", deviceName);
                exit(-1);
        }/*if */ 
        else{
                SetInterfaceAttribs(*fd, gCnvtdBaudRate, 0, UART_TIMEOUT); 
                // SetInterfaceAttribs(*fd, UART_BAUDRATE, 0, UART_TIMEOUT); 
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



int SetInterfaceAttribs(int fd, int speed, int parity, int waitTime)
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
                close(fd);
                printf("__LINE__ = %d, error %s\n", __LINE__, strerror(errno));
                return -1;
        }

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

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
                printf("__LINE__ = %d, error %s\n", __LINE__, strerror(errno));
                return -1;
        }
        return 0;
} /*SetInterfaceAttribs*/





