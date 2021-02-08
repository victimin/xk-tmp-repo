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
#include <time.h>


/*********************************************** 
 *  Version Info section
************************************************/
#define VERSION                     "2.2"


#define MAX_BUFF_LEN     (100)
#define MAX_STR_LEN     (512)
#define UART_BAUDRATE   (115200)
#define UART_TIMEOUT    (0)
#define PACKET_SZ       (9)
#define SIZE_OF_DATA    10

#define APP_SIZE                        (9u)        //it is not max
#define SEND_PARAM_SIZE_MAX             (20u)

char *help = 
"================ How to execute XK-SDK ================\n"
"   -default:   s2r [port] [payload]\n"
"   -b:         s2r [baudrate] [port] [payload]\n"
"   -c:         char\n"
"   -d:         int\n"
"   -f:         float\n"
"   -s:         float\n"
"   -v:         version\n"
"=======================================================\n";

#define RECV_BUFF_SIZE                  (3000U)

FILE *fd0;

typedef union {
	char c[4*RECV_BUFF_SIZE];
	int i[RECV_BUFF_SIZE];
	float f[RECV_BUFF_SIZE];
} rcvmsg_t;


typedef struct {
    int flgSIC;                 //Send if changed
    int paramObjNum_V;
    char paramName[50];
} param_info_t;

typedef struct {
    int appNum;
    int appParamSZ;
	int appSerialNumIdx;
    char appName[30];
    param_info_t param[SEND_PARAM_SIZE_MAX];
} app_info_t;

typedef enum {
    PROTOCOL_INFO_XAKA = 0,
    PROTOCOL_INFO_RESERVED_1,
    PROTOCOL_INFO_RESERVED_2,
    PROTOCOL_INFO_APP_NUM,
    PROTOCOL_INFO_MAX_SZ
}PROTOCOL_INFO_t;

rcvmsg_t rcvmsg;

typedef union {
	char c[256];
	int i[64];
	float f[64];
} sendmsg_t;
sendmsg_t sendmsg;

// char device[]="/dev/ttyACM0";
char device[100]="/dev/ttyACM";

char flgBySerialMode = 0;
char SrcSerialNum[15] = {0,};

short usbstatus;
int fd;
char gStrBaudRate[10];
int gBaudRate = UART_BAUDRATE;
int gCnvtdBaudRate = 0;
int gPortNum;
// char gPortNum[4];
int gIdxPayload=0;


unsigned char tmpBuff[3000]={0,};
unsigned char recvQueue[3000]={0,};
float dest_Data[3000]={0,};
unsigned int idxFront, preIdxXAKA, curIdxXAKA;
int appNumber=0;
int flgSuccess = 0;
int flgFound = 0;
int flgPort = -1;

void UART_init(int * fd, char *argv);
int SetInterfaceAttribs(int fd, int speed, int parity, int waitTime);


app_info_t gAppInfo[APP_SIZE] = {
	{
		.appNum = 40,
		.appName = "presence",
		.appSerialNumIdx = 20,
		.appParamSZ = 2,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 41,
		.appName = "presence_vital",
		.appSerialNumIdx = 20,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 26,
			.paramName = "BR"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 27,
			.paramName = "HR"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 16,
			.paramName = "movement_index"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 24,
			.paramName = "stability_BR"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 25,
			.paramName = "stability_HR"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 28,
			.paramName = "stability_move"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 30,
		.appName = "zone",
		.appSerialNumIdx = 27,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 24,
			.paramName = "peoplecount_instant"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "peoplecount_average"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "index_longterm"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "index_shortterm"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "version"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 25,
			.paramName = "radarmappingvalue"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 26,
			.paramName = "environmappingvalue"
		}
	},
	{
		.appNum = 80,
		.appName = "vital",
		.appSerialNumIdx = 16,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "BR"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "HR"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 3,
			.paramName = "movement_index"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "stability_BR"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "stability_HR"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 23,
			.paramName = "stability_move"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 2,
			.paramName = "connectionstatus"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 60,
		.appName = "foot",
		.appSerialNumIdx = 19,
		.appParamSZ = 5,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "flowcount_instant"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "flowcount_accumulated"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 17,
			.paramName = "updatecounter"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "connectionstatus"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 7,
			.paramName = "version"
		}
	},
	{
		.appNum = 70,
		.appName = "OSR",
		.appSerialNumIdx = 20,
		.appParamSZ = 3,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 10,
			.paramName = "OSR_result"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 71,
		.appName = "Skimmer",
		.appSerialNumIdx = 20,
		.appParamSZ = 3,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 11,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 10,
			.paramName = "OSR_result"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "version"
		}
	},
	{
		.appNum = 21,
		.appName = "INOUT_single",
		.appSerialNumIdx = 26,
		.appParamSZ = 6,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "connectionstatus"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 14,
			.paramName = "INcount"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 15,
			.paramName = "OUTcount"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 16,
			.paramName = "INcount_accumulated"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 17,
			.paramName = "OUTcount_accumulated"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 22,
			.paramName = "updatecounter"
		}
	},
	{
		.appNum = 50,
		.appName = "WMFD",
		.appSerialNumIdx = 20,
		.appParamSZ = 8,
		.param[0] = {
			.flgSIC	= 0,
			.paramObjNum_V = 6,
			.paramName = "presence"
		},
		.param[1] = {
			.flgSIC	= 0,
			.paramObjNum_V = 0,
			.paramName = "fall_status"
		},
		.param[2] = {
			.flgSIC	= 0,
			.paramObjNum_V = 1,
			.paramName = "BR"
		},
		.param[3] = {
			.flgSIC	= 0,
			.paramObjNum_V = 3,
			.paramName = "HR"
		},
		.param[4] = {
			.flgSIC	= 0,
			.paramObjNum_V = 5,
			.paramName = "movement_index"
		},
		.param[5] = {
			.flgSIC	= 0,
			.paramObjNum_V = 2,
			.paramName = "stability_BR"
		},
		.param[6] = {
			.flgSIC	= 0,
			.paramObjNum_V = 4,
			.paramName = "stability_HR"
		},
		.param[7] = {
			.flgSIC	= 0,
			.paramObjNum_V = 20,
			.paramName = "stability_move"
		}
	}
};


int GetMetaInfo(char *outData, float *radarData, unsigned int appNum){
    int i;
    int idxApp = -1;

    for(i=0;i<APP_SIZE;i++){
        if(gAppInfo[i].appNum == appNum){
            idxApp = i;
			if(outData!=NULL){
				sprintf(outData, "%06d%06d", 
					(int)radarData[gAppInfo[i].appSerialNumIdx + PROTOCOL_INFO_MAX_SZ], 
					(int)radarData[gAppInfo[i].appSerialNumIdx+1 + PROTOCOL_INFO_MAX_SZ]
				); 
			}
                        // outData[12]='\0';
			return idxApp;
        }
    }
    if(idxApp == -1){    
		if(outData!=NULL){
			sprintf(outData, "%06d%06d", 
				999999, 
				999999
			);
                        // outData[12]='\0';
		}
    }
    return idxApp;
}


int main(int argc, char *argv[]){
    int param_opt;
    char tmpCmdLine[100];
    int prePid_i=0;
    char* tmpPtr;
    int i, j, port_i;
    clock_t start_t, end_t;
    int len;
    int frame_cnt;
    char gotSerial[15];
    
    int file_access;  
    int temp_4byte = 0, numArg=0;

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
                        gPortNum = atoi(argv[3]);
                        printf("Baudrate: [%d]         Port: [%d]", gBaudRate, gPortNum);
                        if(gBaudRate % 9600 != 0) {
                                printf("Wrong baudrate\n");
                                exit(1);
                        }                        
                        gIdxPayload = 4;
                }
                else if(strcmp(argv[1],"-s")==0){
                        strcpy(SrcSerialNum, argv[2]);
                        printf("Baudrate: [%d]         Serial: [%s]", gBaudRate, argv[2]);
                        flgBySerialMode = 1;
                        gIdxPayload = 3;
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












        if(flgBySerialMode == 1){
                for(port_i=0; port_i<10; port_i++){

                        sprintf(device, "/dev/ttyACM%d", port_i);

                        file_access = access(device, R_OK | W_OK);
                        if(file_access == -1){
                                continue;
                        }
                        else if(file_access == 0){
                                UART_init(&fd, device);
                                if(fd == -1){
                                        printf("%s invalid device\n", device);
                                        exit(1);
                                }
                        }
                        // printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   %s\n", device);

                        start_t = clock();
                        end_t = clock();
                        frame_cnt = 0;
                        appNumber = 0;
                        idxFront=0;
                        preIdxXAKA=0;
                        curIdxXAKA=0;
                        memset(recvQueue, 0, 3000);
                        // memset(dest_Data, 0, 3000);
                        flgSuccess = 0;
                        flgFound = 0;
                        
                        while( end_t-start_t < 50000){
                                len = read(fd, &tmpBuff[0], MAX_STR_LEN);  
                                if(len){  
                                        for (i = 0; i < len; i++){
                                                recvQueue[idxFront++] = tmpBuff[i];
                                                if(idxFront == RECV_BUFF_SIZE) idxFront = 0;
                                                
                                        }

                                        // for (i = 0; i < RECV_BUFF_SIZE; i++){
                                        for (i = 0; i < len; i++){
                                                if(recvQueue[(RECV_BUFF_SIZE + idxFront-i)%RECV_BUFF_SIZE] == 'A'&&recvQueue[(RECV_BUFF_SIZE + idxFront-i-1)%RECV_BUFF_SIZE] == 'K' &&
                                                                recvQueue[(RECV_BUFF_SIZE + idxFront-i-2)%RECV_BUFF_SIZE] == 'A'&&recvQueue[(RECV_BUFF_SIZE + idxFront-i-3)%RECV_BUFF_SIZE] == 'X'){
                                                
                                                        curIdxXAKA = (RECV_BUFF_SIZE + idxFront - i - 3)%RECV_BUFF_SIZE;
                                                        if((curIdxXAKA != preIdxXAKA) && frame_cnt>2){
                                                                for (j = 0; j < RECV_BUFF_SIZE; j++){
                                                                        if(recvQueue[(RECV_BUFF_SIZE + curIdxXAKA-j   - 4)%RECV_BUFF_SIZE] == 'A'
                                                                        && recvQueue[(RECV_BUFF_SIZE + curIdxXAKA-j-1 - 4)%RECV_BUFF_SIZE] == 'K' 
                                                                        && recvQueue[(RECV_BUFF_SIZE + curIdxXAKA-j-2 - 4)%RECV_BUFF_SIZE] == 'A'
                                                                        && recvQueue[(RECV_BUFF_SIZE + curIdxXAKA-j-3 - 4)%RECV_BUFF_SIZE] == 'X')
                                                                        {
                                                                                preIdxXAKA = (RECV_BUFF_SIZE + curIdxXAKA - j - 3 - 4)%RECV_BUFF_SIZE;
                                                                                break;
                                                                        }
                                                                }
                                                                

                                                                temp_4byte = curIdxXAKA - preIdxXAKA;
                                                                if(temp_4byte<0){
                                                                        temp_4byte = (RECV_BUFF_SIZE + curIdxXAKA) - preIdxXAKA;
                                                                } 
                                                                numArg = temp_4byte/4;

                                                                // if(rcvmsg.i[2] != (temp_4byte-4*3)/4 && frame_cnt>3) 
                                                                //         LOG_E("Data Error", "DN: %d         Received N: %d       Received data: %d bytes\n", rcvmsg.i[2], (temp_4byte-4*3)/4, temp_4byte);

                                                                for(loopi=0;loopi<numArg;loopi++){
                                                                        for(loopj=0;loopj<4;loopj++){    
                                                                                rcvmsg.c[loopi * 4 + loopj] = recvQueue[(preIdxXAKA+(loopi * 4 + loopj))%RECV_BUFF_SIZE];
                                                                        }                                  
                                                                        dest_Data[loopi] = rcvmsg.f[loopi];
                                                                }

                                                                if(temp_4byte > 0){
                                                                        appNumber = (unsigned int)rcvmsg.f[3];
                                                                }
                                                                
                                                                // for(loopi=0;loopi<40;loopi++){
                                                                //         printf("%f \n", loopi, dest_Data[loopi], numArg);
                                                                // }

                                                                
                                                                GetMetaInfo(gotSerial, dest_Data, appNumber);
                                                                // printf("%s, %s    %d\n", gotSerial,SrcSerialNum, appNumber);
                                                                if( strcmp(gotSerial, SrcSerialNum) == 0 ){
                                                                // printf("yes yes yes yes yes yes yes yes yes yes yes yes yes yes yes yes \n");
                                                                        flgFound = 1;
                                                                }

                                                                flgSuccess = 1;
                                                        } 
                                                        preIdxXAKA = curIdxXAKA;  
                                                        
                                                        frame_cnt++;
                                                        break;
                                                }
                                        }
                                }

                                if (flgSuccess) break;
                                end_t = clock();
                                // printf("%d\n", end_t-start_t);
                                usleep(5000);
                                
                                        // printf("ssssssssss %d\n",frame_cnt);
                        }
                        close(fd);

                        if(flgFound){
                                flgPort = port_i;
                                break;
                        }
                }
        }
        
        
        if(flgBySerialMode == 1){
                if(flgFound == 0){
                        printf("Can't find radar(%s)!!!!!!!!!!!!!!!!!!!!!!!!!! \n",SrcSerialNum);
                        exit(0);
                }
                sprintf(device, "/dev/ttyACM%d", port_i);
        }
        else
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
                printf("@@@@@@@ %d      %f\n", LengthMsg, sendmsg.f[loopi]);
        }
                printf("LengthMsg = %d\n", LengthMsg);

        for(loopi=0 ; loopi<LengthMsg ; loopi++){
                printf("%2d:%f\n",loopi,sendmsg.f[loopi]);

                for(loopj=0 ; loopj<4 ; loopj++) 
                {
                        printf("  %2d:%d(%02x)\n", loopi*4+loopj, sendmsg.c[loopi*4+loopj], sendmsg.c[loopi*4+loopj]);
                }
        }

		write(fd, &sendmsg.c[0], LengthMsg*4);
		
        close(fd);
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



