#include "logging.h"
// #include <direct.h>

int preTimeDay = -1;
 
void Eliminate(char *str, char ch){
    for (; *str != '\0'; str++){
        if (*str == ch){
            strcpy(str, str + 1);
            str--;
        }
    }
}

void LoggingBleData(char * mac, stIBconData *iBconDate){
    FILE *fd_log_file;   
    int file_access;  
	char logPrePath[100]={0,}; 
	char logFullPath[200]={0,}; 
	char logInfoBuff[200]={0,};  
    char month[12][10] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char tmpMac[20] = {0,};
	int i,j;

	strcpy(tmpMac, mac);
	Eliminate(tmpMac, ':');
	
    preTimeDay = gLocalTime.tm_mday;   
	// sprintf(logPrePath, LOG_PATH, tmpMac, LOG_NAME);        
	sprintf(logPrePath, LOG_PATH);
	sprintf(&logPrePath[strlen(logPrePath)], "/%s", tmpMac);	
	int nResult = mkdir( logPrePath );
	sprintf(&logPrePath[strlen(logPrePath)], "/%s", LOG_NAME);

	sprintf(logFullPath, logPrePath, gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday);
	// printf("%s\n", logFullPath);

	file_access = access(logFullPath, R_OK | W_OK);
	if(file_access == -1){
		fd_log_file = fopen(logFullPath, "w");

		fprintf(fd_log_file,"time,");
		for(j=0;j<3;j++) fprintf(fd_log_file,"advFlgs[%d],", j);
		for(j=0;j<2;j++) fprintf(fd_log_file,"advHeader[%d],", j);
		for(j=0;j<2;j++) fprintf(fd_log_file,"companyID[%d],", j);
		for(j=0;j<1;j++) fprintf(fd_log_file,"iBconType[%d],", j);
		for(j=0;j<1;j++) fprintf(fd_log_file,"iBconLen[%d],", j);
		for(j=0;j<16;j++) fprintf(fd_log_file,"UUID[%d],", j);
		for(j=0;j<2;j++) fprintf(fd_log_file,"majorNum[%d],", j);
		for(j=0;j<2;j++) fprintf(fd_log_file,"minorNum[%d],", j);
		fprintf(fd_log_file,"RSSI\n");
	}
	else if(file_access == 0)
		fd_log_file = fopen(logFullPath, "a+");

	if ( fd_log_file == NULL ) {
		LOG_E("LOGGING","log file open error\n"); 
		return -1;       
	}
	
	fprintf(fd_log_file, "%02d:%02d:%02d %s %02d,", gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec, month[gLocalTime.tm_mon], gLocalTime.tm_mday);

	for(j=0;j<3;j++) fprintf(fd_log_file,"%02x,", iBconDate->iBconPrefix.advFlgs[j]);
	for(j=0;j<2;j++) fprintf(fd_log_file,"%02x,", iBconDate->iBconPrefix.advHeader[j]);
	for(j=0;j<2;j++) fprintf(fd_log_file,"%02x,", iBconDate->iBconPrefix.companyID[j]);
	for(j=0;j<1;j++) fprintf(fd_log_file,"%02x,", iBconDate->iBconPrefix.iBconType);
	for(j=0;j<1;j++) fprintf(fd_log_file,"%02x,", iBconDate->iBconPrefix.iBconLen);
	for(j=0;j<16;j++) fprintf(fd_log_file,"%02x,", iBconDate->UUID[j]);
	for(j=0;j<2;j++) fprintf(fd_log_file,"%02x,", iBconDate->majorNum[j]);
	for(j=0;j<2;j++) fprintf(fd_log_file,"%02x,", iBconDate->minorNum[j]);
	fprintf(fd_log_file,"%d\n", iBconDate->RSSI);
	fclose(fd_log_file);

}



