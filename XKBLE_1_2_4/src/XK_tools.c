#include <stdio.h>
#include <string.h>
#include <time.h> 
#include <stdarg.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "XK_tools.h"
#include "XK_CommonDefineSet.h"

#define BUFFER_SIZE 		(1024 * 20u)
extern char gFlgSysLog;
int preTimeDaySys;

void XK_SystemPrint(char *str, unsigned char type){
	switch (type){
		case P_INFO:
			printf("# XK system : [INFO]%s", str);
		break;
		case P_WARRING:
			printf("# XK system : [WARRING]%s", str);
		break;
		case P_ERROR:
			printf("# XK system : [ERROR]%s", str);
		break;
		default:
			printf("XK PRINT ERROR!!!!!!");
		break;
	}    
}

// extern struct tm gLocalTime;

char tmp[1024*20]={0,};
char month[12][10] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
// FILE *sysLog;
struct   stat  file_info;
fpos_t gCurPos;
time_t timer;
struct tm *t;

void LOG_E (char *tag, char *str,...) {
	static char buffer[1024 * 20];
	va_list vl;
	va_start( vl, str );
	vsnprintf( buffer, 1024 * 20, str, vl );
	va_end( vl );
	sprintf(tmp, ANSI_COLOR_RED);
	sprintf(&tmp[strlen(tmp)], "[ERROR] ");
	sprintf(&tmp[strlen(tmp)], tag);
	sprintf(&tmp[strlen(tmp)], ": ");
	sprintf(&tmp[strlen(tmp)], buffer);
	sprintf(&tmp[strlen(tmp)], ANSI_COLOR_RESET);
	printf("%s %d %02d:%02d:%02d XAKA System: ",
		month[gLocalTime.tm_mon], gLocalTime.tm_mday,
		gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);
	printf("%s\n", tmp);
	if(gFlgSysLog) LOG_system ("ERROR", tag, buffer);
}

void LOG_I (char *tag, char *str,...) {	
	static char buffer[1024 * 20];
	va_list vl;
	va_start( vl, str );
	vsnprintf( buffer, 1024 * 20, str, vl );
	va_end( vl );
	sprintf(tmp, ANSI_COLOR_GREEN);
	sprintf(&tmp[strlen(tmp)], "[INFO ] ");
	sprintf(&tmp[strlen(tmp)], tag);
	sprintf(&tmp[strlen(tmp)], ": ");
	sprintf(&tmp[strlen(tmp)], buffer);
	sprintf(&tmp[strlen(tmp)], ANSI_COLOR_RESET);
	printf("%s %d %02d:%02d:%02d XAKA System: ",
		month[gLocalTime.tm_mon], gLocalTime.tm_mday,
		gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);
	printf("%s\n", tmp);	
	if(gFlgSysLog) LOG_system ("INFO ", tag, buffer);
}

void LOG_W (char *tag, char *str,...) {
	static char buffer[1024 * 20];
	va_list vl;
	va_start( vl, str );
	vsnprintf( buffer, 1024 * 20, str, vl );
	va_end( vl );
	sprintf(tmp, ANSI_COLOR_YELLOW);
	sprintf(&tmp[strlen(tmp)], "[WARN ] ");
	sprintf(&tmp[strlen(tmp)], tag);
	sprintf(&tmp[strlen(tmp)], ": ");
	sprintf(&tmp[strlen(tmp)], buffer);
	sprintf(&tmp[strlen(tmp)], ANSI_COLOR_RESET);
	  printf("%s %d %02d:%02d:%02d XAKA System: ",
         month[gLocalTime.tm_mon], gLocalTime.tm_mday,
         gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);
	printf("%s\n", tmp);
	if(gFlgSysLog) LOG_system ("WARN ", tag, buffer);
}

void LOG_S (char *tag, char *str,...) {
	static char buffer[1024 * 20];
	va_list vl;
	va_start( vl, str );
	vsnprintf( buffer, 1024 * 20, str, vl );
	va_end( vl );
	sprintf(tmp, ANSI_COLOR_CYAN);
	sprintf(&tmp[strlen(tmp)], "[API  ] ");
	sprintf(&tmp[strlen(tmp)], tag);
	sprintf(&tmp[strlen(tmp)], ": ");
	sprintf(&tmp[strlen(tmp)], buffer);
	sprintf(&tmp[strlen(tmp)], ANSI_COLOR_RESET);
	  printf("%s %d %02d:%02d:%02d XAKA System: ",
         month[gLocalTime.tm_mon], gLocalTime.tm_mday,
         gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);
	printf("%s\n", tmp);
	if(gFlgSysLog) LOG_system ("API  ", tag, buffer);
}

void LOG_N (char *tag, char *str,...) {
	static char buffer[1024 * 20];
	va_list vl;
	va_start( vl, str );
	vsnprintf( buffer, 1024 * 20, str, vl );
	va_end( vl );
	sprintf(tmp, "[INFO-] ");
	sprintf(&tmp[strlen(tmp)], tag);
	sprintf(&tmp[strlen(tmp)], ": ");
	sprintf(&tmp[strlen(tmp)], buffer);
	  printf("%s %d %02d:%02d:%02d XAKA System: ",
         month[gLocalTime.tm_mon], gLocalTime.tm_mday,
         gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);
	printf("%s\n", tmp);
	if(gFlgSysLog) LOG_system ("INFO-", tag, buffer);
}

void LOG_D (char *tag, char *str,...) {
	static char buffer[1024 * 20];
	va_list vl;
	va_start( vl, str );
	vsnprintf( buffer, 1024 * 20, str, vl );
	va_end( vl );
	sprintf(tmp, ANSI_COLOR_MAGENTA);
	sprintf(&tmp[strlen(tmp)], "[DEBUG] ");
	sprintf(&tmp[strlen(tmp)], tag);
	sprintf(&tmp[strlen(tmp)], ": ");
	sprintf(&tmp[strlen(tmp)], buffer);
	sprintf(&tmp[strlen(tmp)], ANSI_COLOR_RESET);
	  printf("%s %d %02d:%02d:%02d XAKA System: ",
         month[gLocalTime.tm_mon], gLocalTime.tm_mday,
         gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);
	printf("%s\n", tmp);
	if(gFlgSysLog) LOG_system ("DEBUG", tag, buffer);
}

void LOG_system (char *type, char *tag, char *msg) {
	char _tmp[1024*20];
	char logFullPath[200];  
    FILE *sysLog;
	sprintf(logFullPath, SYS_LOG_PATH, gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday);
	if ( 0 > stat( logFullPath, &file_info)){
		sysLog = fopen(logFullPath, "w");
	}
	else{
		sysLog = fopen(logFullPath, "a+");
	}
	sprintf(&_tmp[0], "%s %d %02d:%02d:%02d ",
		month[gLocalTime.tm_mon], gLocalTime.tm_mday,
		gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec);	
	sprintf(&_tmp[strlen(_tmp)], "[%s] ", type);
	sprintf(&_tmp[strlen(_tmp)], tag);
	sprintf(&_tmp[strlen(_tmp)], ": ");
	sprintf(&_tmp[strlen(_tmp)], msg);
	fprintf(sysLog, "%s\n", _tmp);
	
	fsetpos(sysLog, &gCurPos);
	fclose(sysLog);
	 
	if(gLocalTime.tm_mday != preTimeDaySys) {
		int retval;
		retval = system("find /var/log/xk/xkble/sys/*.xkl -maxdepth 1 -mtime +60 -delete");
		preTimeDaySys = gLocalTime.tm_mday;
	}
}

int GetRamInKB(void)
{
    FILE *meminfo = fopen("/proc/meminfo", "r");
    // if(meminfo == NULL)
        // ... // handle error

    char line[256];
    while(fgets(line, sizeof(line), meminfo))
    {
        int ram;
        if(sscanf(line, "MemFree: %d kB", &ram) == 1)
        {
            fclose(meminfo);
            return ram;
        }
    }

    // If we got here, then we couldn't find the proper line in the meminfo file:
    // do something appropriate like return an error code, throw an exception, etc.
    fclose(meminfo);
    return -1;
}

int GetCPUSerial(char *serial_string, char *HW_string)
{
	FILE *f = fopen("/proc/cpuinfo", "r");
	if (!f) {
		return 1;
	}
	char line[256]; 
	int serial;
	while (fgets(line, 256, f)) {
		if (strncmp(line, "Serial", 6) == 0) {
			serial = atoi(strcpy(serial_string, strchr(line, ':') + 2));
			serial_string[strlen(serial_string)-1] = '\0';
		}
		if (strncmp(line, "Hardware", 8) == 0) {
			serial = atoi(strcpy(HW_string, strchr(line, ':') + 2));
			HW_string[strlen(HW_string)-1] = '\0';
		}
	}

	return fclose(f);
}

double get_temp(int use_farenheit)
{
  char data[BUFFER_SIZE];
  FILE *finput;
  size_t bytes_read;
  double temp = -1000;
  finput = fopen("/sys/class/thermal/thermal_zone0/temp","r");
  if (finput != NULL) {
    memset(data,0,BUFFER_SIZE);
    fread(data,BUFFER_SIZE,1,finput);
    temp = atoi(data);
    temp /= 1000;
    if (use_farenheit) {
      temp = temp * 9 / 5 + 32;
    }
    fclose(finput);
  }
  return temp;
}

void CPU_Tick(long double *a){  
    FILE *fp;  
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    fclose(fp);
}
long double CPU_Tock(long double *a, long double *b){        
    FILE *fp;
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    fclose(fp);
    return ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
}

extern int brktLevel;
extern int objCnt;
extern int groupCnt[100];

void XK_ClientAddBody_g(char * data, char * object){
    if(objCnt || groupCnt[brktLevel]) sprintf(&data[strlen(data)], ",");
    XK_ClientAddIndent(data);
    sprintf(&data[strlen(data)], "\"%s\": {", object);
    brktLevel++;
    objCnt=0;
}

void XK_ClientAddBody_s(char * data, char * object, char * contents){
    if(objCnt) sprintf(&data[strlen(data)], ",");
    XK_ClientAddIndent(data);
    sprintf(&data[strlen(data)], "\"%s\": ", object);
    sprintf(&data[strlen(data)], "\"%s\"", contents);
    objCnt++;
}

void XK_ClientAddBody_iBcon(char * data, char * object, int devNum){
	int j;
    if(objCnt) sprintf(&data[strlen(data)], ",");
    XK_ClientAddIndent(data);
    sprintf(&data[strlen(data)], "\"%s\": ", object);
    sprintf(&data[strlen(data)], "\"");
    	
	for(j=0;j<3;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].iBconPrefix.advFlgs[j]);
	for(j=0;j<2;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].iBconPrefix.advHeader[j]);
	for(j=0;j<2;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].iBconPrefix.companyID[j]);
	for(j=0;j<1;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].iBconPrefix.iBconType);
	for(j=0;j<1;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].iBconPrefix.iBconLen);
	for(j=0;j<16;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].UUID[j]);
	for(j=0;j<2;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].majorNum[j]);
	for(j=0;j<2;j++) sprintf(&data[strlen(data)], "%02x", iBconDate[devNum].minorNum[j]);

    sprintf(&data[strlen(data)], "\"");
    objCnt++;
}

void XK_ClientAddBody_i(char * data, char * object, int contents){
    if(objCnt) sprintf(&data[strlen(data)], ",");
    XK_ClientAddIndent(data);
    sprintf(&data[strlen(data)], "\"%s\": ", object);
    sprintf(&data[strlen(data)], "%d", contents);
    objCnt++;
}

void XK_ClientAddBody_f(char * data, char * object, float contents){
    if(objCnt) sprintf(&data[strlen(data)], ",");
    XK_ClientAddIndent(data);
    sprintf(&data[strlen(data)], "\"%s\": ", object);
    sprintf(&data[strlen(data)], "%0.2f", contents);
    objCnt++;
}

void XK_ClientAddBracket(char * data, char * bracket){
    if(bracket[0] == '{'){        
        if(groupCnt[brktLevel]) sprintf(&data[strlen(data)], ",");
        XK_ClientAddIndent(data);
        sprintf(&data[strlen(data)], bracket);
        brktLevel++;
        objCnt=0;
    }
    else if(bracket[0] == '}'){
        brktLevel--;
        groupCnt[brktLevel]++;        
        XK_ClientAddIndent(data);
        sprintf(&data[strlen(data)], bracket);
    }
}

void XK_ClientAddIndent(char * data){
    int i;
    sprintf(&data[strlen(data)], "\n\r");
    for(i=0;i<brktLevel;i++){
        sprintf(&data[strlen(data)], "\t");
    }
}

void XK_HTTP_ClientSetHeader_POST(char * data, char * page, char * version){
    sprintf(&data[0], "POST ");
    sprintf(&data[strlen(data)], "/%s ", page);
    sprintf(&data[strlen(data)], "%s\r\n", version);
}

void XK_HTTP_ClientSetHeader(char * data, char * object, char * contents){
    sprintf(&data[strlen(data)], "%s: ", object);
    sprintf(&data[strlen(data)], "%s\r\n", contents);
}

void XK_HTTP_ClientSetHeaderClose(char * data){
    sprintf(&data[strlen(data)], "\r\n");
}

void XK_GetTeamviewerID(char *id, char *path){
	FILE *fd;
	char tmp[30000];
	fd = fopen(path, "r");
	fread(tmp, 30000, 1, fd);
	if(strstr(tmp, "ClientID = ") != NULL){
		memcpy(id, strstr(tmp, "ClientID = ")+11, 10);
	}
	fclose(fd);
}
void XK_GetTeamviewerVer(char *ver, char *path){
	FILE *fd;
	char tmp[30000];
	fd = fopen(path, "r");
	fread(tmp, 30000, 1, fd);
	if(strstr(tmp, " Version = ") != NULL){
		memcpy(ver, strstr(tmp, " Version = ")+12, 11);
	}
	fclose(fd);
}

void XK_GetImageVersion(char *version){
	FILE *fd;
	char tmp[10]={0,};    
    int file_access; 

    file_access = access(IMAGE_VERSION_FILE, R_OK | W_OK);
    if(file_access == -1){
        strcpy(version, "none");
    }
    else if(file_access == 0){
        fd = fopen(IMAGE_VERSION_FILE, "r");
        fread(tmp, 7, 1, fd);
        memcpy(version, tmp, 8);
        fclose(fd);
    }
}

void XK_GetMAC(char *mac, char *path){
	FILE *fd;
	fd = fopen(path, "r");
	if(fd!=NULL){
		fread(mac, 20, 1, fd);
		mac[strlen(mac)-1]=0;
	}
}

void XK_PrintInfo_s(char *object, char *content){
	printf("%s: %s\n", object, content);
}

void XK_PrintInfo_i(char *object, int content){
	printf("%s: %d\n", object, content);
}

void XK_PrintInfo_c(char *object, int content){
	printf("%s: %c\n", object, content);
}

int GetProcessNameByPid(int pid, char* name) { 
	int result = ESUCCESS; 
	
	sprintf(name, "/proc/%d/cmdline", pid); 
	FILE* fp = fopen(name,"r"); 
	if(!fp){ 
		result = ENOPROC; 
	} 
	else { 
		size_t size; 
		size = fread(name, sizeof(char), MAX_PROCESS_NAME , fp); 
		if(size>0){ 
			name[size] ='\0'; 
		} 
		else { 
			result = ENONAME; 
		} 
		
		fclose(fp); 
	} 
	return result ; 
}

int WritePID(void){
    FILE *fMyPid = fopen(MYPID_PATH, "w");   //MYPID_PATH
    int myPid_i;
    char myPid_s[10];
    myPid_i = getpid();
    LOG_I("Init", "My PID = %d\n", myPid_i);
    sprintf(myPid_s, "%d\n", myPid_i);
    fwrite(myPid_s, 1, strlen(myPid_s), fMyPid);
    fclose(fMyPid);
}

int GetComplements(char data){
	int tmp;
	tmp = (char)~data;
	tmp++;
	return tmp*-1;
}

void delChar( char *buf, char *dest, char ch ){
    while ( *buf )
    {
        if ( *buf == ch )
        {
            buf++;
            continue;
        }
        *dest++ = *buf++;
    }
}

void MacConverterS2F(float *mac_f, char * mac_s){
    char tmp[10]={0,};
    int i;

    // printf("%s   \n", mac_s);
    for(i=0;i<6;i++){
        memcpy(tmp, &mac_s[i*3], 2);
        tmp[2]='\0';
        *(mac_f+i) = (float)strtol(tmp, NULL, 16);
        // printf("%s   %f\n", tmp, *(mac_f+i));
    }
    // printf("\n");

}

int SetHostapd(char *mac, char *ssid, char *pass){
    int i;
    char tmpPath[50]={0,};
    char tmpPass[32]={0,};
    char tmpSsid[32]={0,};
    char hostapdConfig[500]={0,};
    char hostapdConfigTmp[500] = "ctrl_interface=/var/run/hostapd\n\
ctrl_interface_group=0\n\
interface=ap0\n\
driver=nl80211\n\
ssid=%s\n\
hw_mode=g\n\
channel=11\n\
wmm_enabled=0\n\
macaddr_acl=0\n\
auth_algs=1\n\
ignore_broadcast_ssid=0\n\
wpa=2\n\
wpa_passphrase=%s\n\
wpa_key_mgmt=WPA-PSK\n\
wpa_pairwise=TKIP CCMP\n\
rsn_pairwise=CCMP\n\
";
    char cpyCmdTemplate[200]="sudo cp %s %s";
    char cpyCmd[200]={0,};

    sprintf(tmpPath, "/boot/XandarKardian/%s", HOSTAPD_CONF_FILE);
    FILE *fHostapd = fopen(tmpPath, "w");

    // printf("%s\n", hostapdConfigTmp);
    srand( time(NULL) );
    for(i=0;i<12;i++){
        sprintf(&tmpPass[strlen(tmpPass)],"%01d",rand()%10);
    }
    strcpy(tmpSsid, AP_SSID);
    for(i=9;i<=16;i++){
        if(i==11 || i==14);
        else sprintf(&tmpSsid[strlen(tmpSsid)],"%c",mac[i]);
    }
    sprintf(hostapdConfig, hostapdConfigTmp, tmpSsid, tmpPass);
    // printf("%s\n", hostapdConfig);
    strcpy(ssid, tmpSsid);
    strcpy(pass, tmpPass);

    fwrite(hostapdConfig, 1, strlen(hostapdConfig), fHostapd);
    fclose(fHostapd);

    sprintf(cpyCmd, cpyCmdTemplate, tmpPath, HOSTAPD_CONF_PATH_FILE);
    printf("%s\n", cpyCmd);
    system(cpyCmd);
}

