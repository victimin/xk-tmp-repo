#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>

#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

#include <signal.h>

#include "XK_CommonDefineSet.h"
#include "httpClient.h"


/*********************************************** 
 *  iBeacon Data
************************************************/
int cntRecognizedDev = 0;

/*********************************************** 
 *  system flags
************************************************/
char gFlgSysLog = ON;
char gFlgForce = OFF;
char gFlgFirst = 1;
char gFlgDataLog = 1;
char gFlgAutoReboot = 1;
char gDebugMode = OFF;
char gInfoMode = OFF;
char gTypeData;
char gTypeRID;
char gPrePid_s[10];

char *help = 
"================ How to execute XKBLE ================\n"
"   -default:   display default \n"
"   -h:         display help \n"
"   -f:         force run \n"
"   -s:         display raw send data \n"
"   -r:         display raw receive data \n"
"   -w:         display raw data \n"
"   -d:         display debug messege \n"
"   -R:         display free heap memory size (free memory) \n"
"   -u:         display CPU usage \n"
"   -t:         send a-type message \n"
"   -v:         check XKBLE, API version \n"
"=======================================================\n";

int gRunning;

void OptionParser(int argc, char *argv[]);
int IsRunning();
void  INThandler(int sig);
void XK_Exit(void);
void PrintInfo(void);


int main (int argc, char *argv[])
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    pinMode(GPIO_UART_RST, OUTPUT);     // Set regular LED as output
    digitalWrite(GPIO_UART_RST, HIGH); // Turn LED ON

    XkWatchThreadInit();
	int i;
	int ret, status;
	int device;
	le_set_scan_enable_cp scan_cp;
    delay(5);

    if(argc > 1){
        if(strcmp(argv[1],"-v")==0){        
            printf("- XKBLE version: %s\n Copyright(c) 2020. XandarKardian. All rights reserved.\n", XKBLE_VERSION);
            sem_close(gRunning);
            sem_unlink(XK_PROCESS_ID);
            return 0;
        }
        else if(strcmp(argv[1],"-h")==0){
            printf("%s\n", help);
            return 0;
        }
    } 
    sprintf(httpHandle.bootedTime, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:00", gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday, gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec , (gLocalTime.tm_gmtoff/(60*60) > 0)? '+':'-', (gLocalTime.tm_gmtoff/(60*60) > 0)? gLocalTime.tm_gmtoff/(60*60):(gLocalTime.tm_gmtoff/(60*60))*(-1));

    printf("%s\n", logo);

    printf("Registering PID......\n");
    OptionParser(argc, argv);

    if(gFlgForce == ON){
        sem_close(gRunning);
        sem_unlink(XK_PROCESS_ID);
    }
    signal(SIGINT, INThandler);
    IsRunning();
    WritePID();

    printf("running...\n");

	InitInfo(&configInfo);
	    
    configInfo.isNewWorld = 0;
    char *ptr = strstr(configInfo.host, "internalapi");
    if (ptr != NULL) configInfo.isNewWorld = 1;
    PrintInfo();

    httpHandle.info = &configInfo;

	// Get HCI device.
	device = GetHciDevice();
	// Set BLE scan parameters.	
	SetBleScanParams(device);
	// Set BLE events report mask.
	SetBleEventsReportMask(device);
	// Enable scanning.
	EnableScanning(device, &scan_cp);
	// Get Results.
	GetResults(device);

	printf("Scanning....\n");

	// uint8_t buf[HCI_MAX_EVENT_SIZE];
	// evt_le_meta_event * meta_event;
	// le_advertising_info * info;
	// int len;
	// int flgFound=0;
    // int bytes_read, client;
	
	XkBleThreadInit(device);
    pause();

	while ( 1 ) {
		// len = read(device, buf, sizeof(buf));
		// if ( len >= HCI_EVENT_HDR_SIZE ) {
		// 	meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
		// 	if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT ) {
		// 		uint8_t reports_count = meta_event->data[0];
		// 		void * offset = meta_event->data + 1;
		// 		while ( reports_count-- ) {
		// 			info = (le_advertising_info *)offset;
		// 			char addr[18];
		// 			ba2str(&(info->bdaddr), addr);
		// 			// printf("%s - RSSI %d\n", addr, (char)info->data[info->length]);
                    
		// 			if( strcmp(TEST_MAC, addr)==0 ){
		// 				iBconDate->RSSI=GetComplements((char)info->data[info->length]);

		// 				printf("%d\n", iBconDate->RSSI);

        //                 printf("%s\n", addr);
        //                 printf("info->length = %d\n", info->length);

		// 				ParseBleData(info->data, iBconDate, info->length);

        //                 // for(i=0;i<info->length;i++){
        //                 //     printf("%02x ", (char)info->data[i]);
        //                 // }
        //                 printf("\n");
		// 			}
		// 			offset = info->data + info->length + 2;
		// 		}
		// 	}
		// }
		delay(5000);
	}

	// Disable scanning.
	DisableScanning(device, &scan_cp);

	return 0;
}





/*********************************************** 
 *  Application option parsing section
 *  Add app option 
************************************************/
void OptionParser(int argc, char *argv[]){
    
    int param_opt; 
    FILE *fMyPid = fopen(MYPID_PATH, "r");
    if(fMyPid == NULL) 
    {
        fMyPid = fopen(MYPID_PATH, "r");
        LOG_E("OptionParser","Can't read pid file, please command 'make install'");        
        XK_Exit();
    }

    char tmpCmdLine[100];
    int prePid_i=0;
    char preProcessName[MAX_PROCESS_NAME];
    char* tmpPtr;

    while( -1 !=( param_opt = getopt( argc, argv, "dfsri")))
    {
        switch( param_opt)
        {
            case  'd'   :
                gDebugMode = ON;
                LOG_D("option","Debugging mode!!");
                // pritnOpt = MAIN_OPT_DEBUG_V;
                // printf("* What number of USB do you want see?? : ");
                // scanf("%d", &debugDispUSBnum);
                // LOG_I("option","Debug mode - /dev/ttyUSB%d is selected!!", debugDispUSBnum);
            break;
            
            case  's'   :  
                httpHandle.optDisp |= (0x1<<HTTP_DISP_OPT_SEND);
                LOG_I("option","Sending data will be displayed!!");
            break;

            case  'r'   :  
                httpHandle.optDisp |= (0x1<<HTTP_DISP_OPT_RECV);
                LOG_I("option","Received data will be displayed!!");
            break;
            
            case  'i'   :  
                gInfoMode = ON;
                LOG_I("option","Received data will be displayed!!");
            break;
            
            case  'f'   : 
                gFlgForce = ON;
                LOG_I("option","Force Run");
                
                fgets(gPrePid_s, sizeof(gPrePid_s), fMyPid);
                fclose(fMyPid);

                sprintf(tmpCmdLine, "sudo kill -2 %s", gPrePid_s);
                prePid_i = atoi(gPrePid_s);
                GetProcessNameByPid(prePid_i, preProcessName);
                tmpPtr = strstr(preProcessName, "xkble");
                // LOG_E("", "%s       %d", preProcessName,tmpPtr);
                if(tmpPtr != 0){
                    system(tmpCmdLine);
                    LOG_I("Force Run", "PID(%d) is killed", prePid_i);
                }
                else{
                    LOG_I("Force Run", "No existing process");
                }
            break;
            

            default :   
            break;
        }
    }
}

int IsRunning()
{
    int ret = FALSE;
    gRunning = sem_open(XK_PROCESS_ID, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 1);

    if(gRunning == SEM_FAILED)
    {
        if(errno == EEXIST)
        {            
            LOG_E("Process","'xkble' Process is already running.");
            printf("  - Enter command to exit.\n  - $ sudo killall -2 xkble\n  - $ xkble -f\n");
            exit(-1);
        }
    }

    return ret;
}

void  INThandler(int sig)
{
    char  c;
    LOG_I("Process","Interrupt signal is detected! 'xkble' process terminated!!");
    sem_close(gRunning);
    sem_unlink(XK_PROCESS_ID);
    exit(-1);
}

void XK_Exit(void)
{
    char  c;
    LOG_I("SDK","Interrupt signal is detected! 'xkble' process terminated!!");
    sem_close(gRunning);
    sem_unlink(XK_PROCESS_ID);
    exit(-1);
}

void PrintInfo(void)
{
    int i;
    XK_PrintInfo_s("system-logging",    (configInfo.flgSystemLog==1)? "ON":"OFF");
    XK_PrintInfo_s("data-logging",      (configInfo.flgDataLog==1)? "ON":"OFF");
    XK_PrintInfo_s("device",            configInfo.device);

    XK_PrintInfo_s("HTTP?",             (configInfo.http_onoff==1)? "ON":"OFF");
    XK_PrintInfo_s("function",          configInfo.function);
    XK_PrintInfo_s("mode",              configInfo.mode);
    XK_PrintInfo_i("send-period(ms)",   (int)configInfo.info_send_period);
    XK_PrintInfo_s("client",            configInfo.client);
    XK_PrintInfo_c("data-type",         configInfo.typeData);
    XK_PrintInfo_c("rID-type",          configInfo.typeRID);
    
    XK_PrintInfo_s("authorization", configInfo.auth);
    XK_PrintInfo_s("account-ID", configInfo.accID);
    
    XK_PrintInfo_s("host", configInfo.host);
    XK_PrintInfo_s("page", configInfo.page);
    XK_PrintInfo_i("port", configInfo.port);
    XK_PrintInfo_s("is it new world?", (configInfo.isNewWorld==1)? "YES":"NO");

    XK_PrintInfo_s("MQTT?", (configInfo.mqtt_onoff==1)? "ON":"OFF");
    
    XK_PrintInfo_s("address", configInfo.MqttInfo.address);
    XK_PrintInfo_s("topic", configInfo.MqttInfo.topic);
    XK_PrintInfo_s("username", configInfo.MqttInfo.username);
    XK_PrintInfo_s("password", configInfo.MqttInfo.password);
    // XK_PrintInfo_s("cert-key", configInfo.MqttInfo.cert);
    XK_PrintInfo_i("port", configInfo.MqttInfo.mqttPort);
    XK_PrintInfo_i("frequency", configInfo.MqttInfo.frequency);
    
    XK_PrintInfo_i("server-port", configInfo.server_port);
    
    XK_PrintInfo_s("MYTHINGS?", (configInfo.behr_myts_onoff==1)? "ON":"OFF");

    // XK_PrintInfo_s("client",        configInfo.client);
    // XK_PrintInfo_s("mode",          configInfo.mode);
    // XK_PrintInfo_s("host",          configInfo.host);
    // XK_PrintInfo_s("page",          configInfo.page);
    // XK_PrintInfo_i("port",          configInfo.port);
    // XK_PrintInfo_i("server-port",   configInfo.server_port);
    // XK_PrintInfo_i("send-period(ms)",   (int)configInfo.info_send_period);
    // XK_PrintInfo_s("is it new world?",  (configInfo.isNewWorld==1)? "YES":"NO");
    // XK_PrintInfo_c("data-type",     configInfo.typeData);
    // XK_PrintInfo_c("rID-type",      configInfo.typeRID);
    printf("device list\n");
    for(i=0 ; i<configInfo.radarDeviceNum ; i++){
      printf("   [%s]\n",configInfo.radarDeviceList[i]);
    }
}
