#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h> 
#include <unistd.h>

#define VERSION                     	"1.0"
#define MYPID_PATH                      "/etc/xksdk/pre-pid"
//Edef
#define ESUCCESS                        0
#define ENOPROC                         1
#define ENONAME                         2
#define MAX_PROCESS_NAME                (512)

char gPrePid_s[10];

int WritePID(void){
    FILE *fMyPid = fopen(MYPID_PATH, "w");   //MYPID_PATH
    int myPid_i;
    char myPid_s[10];
    myPid_i = getpid();
    printf("My PID = %d\n", myPid_i);
    sprintf(myPid_s, "%d\n", myPid_i);
    fwrite(myPid_s, 1, strlen(myPid_s), fMyPid);
    fclose(fMyPid);
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

int main()
{
	int i;
    char tmpCmdLine[100];
    int prePid_i=0;
    char preProcessName[MAX_PROCESS_NAME];
    char* tmpPtr;

    FILE *fMyPid = fopen(MYPID_PATH, "r");
    if(fMyPid == NULL) 
    {
        fMyPid = fopen(MYPID_PATH, "r");
        printf("Can't read pid file.\n");        
        return 0;
    }

	fgets(gPrePid_s, sizeof(gPrePid_s), fMyPid);
	fclose(fMyPid);

	sprintf(tmpCmdLine, "sudo kill -2 %s", gPrePid_s);
	prePid_i = atoi(gPrePid_s);
	GetProcessNameByPid(prePid_i, preProcessName);
	tmpPtr = strstr(preProcessName, "xksdk");

	if(tmpPtr != 0){
		system(tmpCmdLine);
		printf("PID(%d) is killed\n", prePid_i);
	}
	else{
		printf("No existing process\n");
	}
	
	sleep(1);

	sprintf(tmpCmdLine, "sudo xksdk");
	system(tmpCmdLine);

	return 0;
}



