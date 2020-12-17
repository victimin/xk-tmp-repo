#include "XK_atcmd.h"
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include <termios.h> /*termio.h for serial IO api*/
#include <errno.h>
#include <signal.h>

char gAcmDevice[30]={0,};

void XK_ATcmdInit(int *fd){
	// read(fd, NULL, 512);
	// XK_ATcmdGetDevId(gAcmDevice);

	// XK_ATcmdGetDevId_ForEC25(gAcmDevice);
	// LOG_I("Get Device ID", "%s", gAcmDevice);
	
	LOG_I("Get Device ID", "%s", "/dev/ttyAMA0");
	memcpy(gAcmDevice, "/dev/ttyAMA0", strlen("/dev/ttyAMA0"));

    if( access(gAcmDevice, R_OK | W_OK ) == 0 ){
        *fd = open(gAcmDevice, O_RDWR | O_NOCTTY);
        if (0 > *fd)
        {
            LOG_E("Open AT-CMD Device", "Can't access");
        }
        else{
            SetInterfaceAttribs(*fd, NULL, BARETECH_USB_BAUD, 0, BARETECH_USB_TIMEOUT); 
        }
    }
    else{
        LOG_E("Access AT-CMD Device", "Can't access");
    }
}

void XK_ATcmdGetDevId_ForEC25(char *dev){
	sprintf(dev, AT_DEV_EC25);
	if(access(dev, R_OK | W_OK) == 0){
		// break;
	}
}

void XK_ATcmdGetDevId(char *dev){
	int i;
	for(i=0;i<20;i++){
		sprintf(dev, AT_DEV, i);
		if(access(dev, R_OK | W_OK) == 0){
			break;
		}
	}
}

void XK_ATcmdWriteSimple(int fd, char *cmd){
	// printf("Sent %s\n",cmd);
	write(fd, cmd, strlen(cmd));
}

int XK_ATcmdRead(int fd, int numOfCrlf){
	char recvbuff[1000]={0,};
	int res=0;
	// unsigned long waitGETTime = clock();
	int cnt_s=0;
	int cnt_c=0;
	int idx_s=0;
	int i;
	
    struct timespec sTimeOutNanoTimeVal;
    int XKtimeCheckSend = 0;
    int XKtimeCheckSend_pre = 0;
    int timeDiff = 0;
    
    clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeVal);    
    XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeVal);
    XKtimeCheckSend_pre=XKtimeCheckSend;

    while (timeDiff < 600 && timeDiff > -600)  /// timeout until 60 sec
    {
		cnt_s = read(fd, &recvbuff[idx_s], 512);
		// if(cnt_s) XKtimeCheckSend_pre = XKtimeCheckSend;//waitGETTime = clock();

		for(i=idx_s;i<cnt_s+idx_s;i++){
			if(recvbuff[i] == '\r' || recvbuff[i] == '\n') cnt_c++;
		}
		idx_s += cnt_s;

        clock_gettime(CLOCK_REALTIME, &sTimeOutNanoTimeVal);
        XKtimeCheckSend = XK_GetTime(sTimeOutNanoTimeVal);
        timeDiff = (10000 + XKtimeCheckSend - XKtimeCheckSend_pre) % 10000;
        // LOG_I("", "%d", timeDiff);

        // if(strstr(recvbuff, "/r/n0/r/n")!=NULL) break;
		
        if(cnt_c >= numOfCrlf){
			res = 1;
			break;
		}
	}
	if (res==0) LOG_E("XK_ATcmdRead", "receive error");

    // printf("Received!!!!!!!!!!!!!!\n");
	for(i=0;i<idx_s;i++) printf("%c", recvbuff[i]);
	printf("%d sec     res = %d\n\n", timeDiff/10, res);

	// printf("%s\n", recvbuff);

	return res;
}

void XK_ATcmdMakeUniMsg(char *destMsg, char *srcMsg){
	sprintf(destMsg, AT_CMD_UNI, strlen(srcMsg)/2, AT_TAB, srcMsg, AT_EOF);
}

void XK_ATcmdSendUniMsg(int fd, float id, float hr, float br){
	char tmpCmdBuff[200]={0,};
	char tmpSrcBuff[200]={0,};
	char tmpSrcAsciiBuff[200]={0,};
	int i;

	sprintf(tmpSrcBuff, "%03d,%3.2f,%3.2f", (int)id, hr, br);
	for(i=0;i<strlen(tmpSrcBuff);i++){
		sprintf(&tmpSrcAsciiBuff[i*2], "%02d", tmpSrcBuff[i]);
	}

	XK_ATcmdMakeUniMsg(tmpCmdBuff, tmpSrcAsciiBuff);
	XK_ATcmdWriteSimple(fd, tmpCmdBuff);
}

void XK_ATcmdClose(int fd){
	close(fd);
}



int SetInterfaceAttribs(int fd, short *usbStatus, int speed, int parity, int waitTime)
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
} /*SetInterfaceAttribs*/



