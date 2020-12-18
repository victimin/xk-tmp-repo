#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h> 
#include <unistd.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>


#define VERSION                     	"0.5"
#define MYPID_PATH                      "/etc/xksdk/pre-pid"
//Edef
#define ESUCCESS                        0
#define ENOPROC                         1
#define ENONAME                         2
#define MAX_PROCESS_NAME                (512)

#define MAX_NUM_USB_DEVICE                  20
#define USB_DEV_PATH                    "/sys/bus/usb/devices/"
#define USB_DEV_BUS_PATH                "/dev/bus/usb"
#define USB_DEV_HUB_PATH                "\:1.0"
#define USB_DEV_DEVNUM_FILE             "devnum"
#define USB_DEV_BUSNUM_FILE             "busnum"
#define USB_DEV_ADAPTER_START           (1u)
#define USB_DEV_ADAPTER_END             (5u)
#define USB_DEV_MAIN_ADAPTER_SIZE       USB_DEV_ADAPTER_END - USB_DEV_ADAPTER_START + 1
#define USB_DEV_PORT_MAX_SIZE           30
#define USB_DEV_HUB_ADAPTER_SIZE        MAX_NUM_USB_DEVICE

char usbName[200][USB_DEV_PORT_MAX_SIZE];
char gPrePid_s[10];

typedef struct
{
        char host[100];
        char page[50];
        int port;
        int send_period;
        char data_type;
        char MAC_e_Addr[20];
        char MAC_w_Addr[20];
        char teamviewerID[11];
        char teamviewerVer[20];
        char xksdkVer[20];
} XK_UsbCmdData_t;

typedef struct
{
        int busnum;
        int devnum;
        int portnum;

        int hubN;

        int hubNum[10];
} XK_USBinfo_t;

typedef struct
{
        XK_USBinfo_t USBinfo;
        int mainHubNum;
        int subHubNum;
} XK_USBinfoBySymlink_t;

XK_USBinfo_t infoUSB[USB_DEV_PORT_MAX_SIZE];
XK_USBinfoBySymlink_t infoUSB_bySymlink[USB_DEV_HUB_ADAPTER_SIZE+1];
XK_UsbCmdData_t gUSB_CMD_Data;


int parseDir(char * path, XK_USBinfo_t * usbInfo)
{
    int i;
    int hubCount=0;
    int cmmCount=0;
    char tmpChar=0;
    int charN = strlen(path);

    for(i=0;i<10;i++) usbInfo->hubNum[i] = 0;

    for(i=0;i<charN;i++){
        if (path[i] == '.'){
            cmmCount++;
            usbInfo->hubNum[hubCount++] = tmpChar-48;
        }
        else if(path[i] == ':'){
            break;
        }
        tmpChar = path[i];
    }
    usbInfo->hubNum[hubCount] = tmpChar-48;
    return hubCount;
}


int readDir(char * path)
{
    DIR *dir;
    int count=0;
    struct dirent *ent;
    dir = opendir (path);
    if (dir != NULL) {  
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            // printf ("%s\n", ent->d_name);
            strcpy(usbName[count], ent->d_name);
            // printf ("%s\n", &name[count]);
            count++;
        }
        closedir (dir);
    } else {
         /* could not open directory */
         perror ("");
        return -1;
    }

    return count;
    
}

int XK_USB_Get_Devnum(int *hubArray, int deep){
        int i;
        char path[200];
        char buffer[10];
        FILE *fp;
        int file_access;

        sprintf(path, USB_DEV_PATH);
        sprintf(&path[strlen(path)], "1-1");
        for(i=1;i<=deep;i++){
                sprintf(&path[strlen(path)], ".%d", hubArray[i]);
        }
        sprintf(&path[strlen(path)], "/%s", USB_DEV_DEVNUM_FILE);

        file_access = access(path, F_OK);
// LOG_W("dev file_access", "[%d] %s", file_access, path);
        if(file_access == -1)
                return -1;
        else if(file_access == 0){
                fp = fopen(path, "r");
        }
        if (fp == NULL)
        {
            printf("%s file open error\n",path);
            fclose(fp);
            // continue;
            return -1;
        }
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);
        return atoi(buffer);
}

int XK_USB_Get_Busnum(int *hubArray, int deep){
        int i;
        char path[200];
        char buffer[10];
        FILE *fp;
        int file_access;

        sprintf(path, USB_DEV_PATH);
        sprintf(&path[strlen(path)], "1-1");
        for(i=1;i<=deep;i++){
                sprintf(&path[strlen(path)], ".%d", hubArray[i]);
        }
        sprintf(&path[strlen(path)], "/%s", USB_DEV_BUSNUM_FILE);

        file_access = access(path, F_OK);
// LOG_W("bus file_access", "[%d] %s", file_access, path);
        if(file_access == -1)
                return -1;
        else if(file_access == 0){
                fp = fopen(path, "r");
        }
        if (fp == NULL)
        {
            printf("%s file open error\n",path);
            fclose(fp);
            // continue;
            return -1;
        }
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);
        return atoi(buffer);
}


int XK_USB_Get_Portnum(char * path){
        // char path[200];
        char varPath[200];
        char finalPath[200];
        char buffer[10];
        FILE *fp;
        int file_access;
        int i;

        sprintf(varPath, USB_DEV_PATH);
        sprintf(&varPath[strlen(varPath)], "%s/ttyUSB", path);

        for(i=0;i<50;i++){                
                sprintf(finalPath, "%s%d", varPath, i);
        
                file_access = access(finalPath, F_OK);
// LOG_W("port file_access", "[%d] %s", file_access, varPath);
                if(file_access == -1){
                        continue;
                }
                else if(file_access == 0){
                        return i;
                }
        }
        return 200;
}



int XK_USB_Scan_All(void){
        int i, j;
        int cntConnected=0;
        int fileN;
        int portN = 200;

        printf("XK-USB Device scanning....\n");
        
        fileN = readDir("/sys/bus/usb/devices/");

        for(i=0;i<fileN;i++){
            // printf ("[%d]%s\n", i, &usbName[i]);
            portN = XK_USB_Get_Portnum(usbName[i]);
            infoUSB[portN].portnum = portN;
            infoUSB[portN].hubN = parseDir(usbName[i], &infoUSB[portN]);
            if(portN != 200){
                infoUSB[portN].busnum = XK_USB_Get_Busnum(infoUSB[portN].hubNum, infoUSB[portN].hubN);
                infoUSB[portN].devnum = XK_USB_Get_Devnum(infoUSB[portN].hubNum, infoUSB[portN].hubN);
            }
        }
        return cntConnected;
}


int XK_USB_Reset(int port){
        char path[200];
        int fd;
        int rc;
        // printf("USB cable problem found!\n");

        sprintf(path, USB_DEV_BUS_PATH);
        sprintf(&path[strlen(path)], "/%03d", infoUSB[port].busnum);
        sprintf(&path[strlen(path)], "/%03d", infoUSB[port].devnum);
        // sprintf(&path[strlen(path)], "/%03d", infoUSB_bySymlink[port].USBinfo.busnum);
        // sprintf(&path[strlen(path)], "/%03d", infoUSB_bySymlink[port].USBinfo.devnum);
        printf("%s\n",path);

        fd = open(path, O_WRONLY);
        if (fd < 0) {
                printf("Error opening output file\n");
                return -1;
        }        
        usleep(1000);

        rc = ioctl(fd, USBDEVFS_RESET, 0);
        if (rc < 0) {
                printf("Error in ioctl\n");
                close(fd);
                return -1;
        }
        usleep(1000);
        printf("/dev/ttyUSB%d reset successful\n", port);

        close(fd);
        return 1;  
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
	
	if(argc > 1){
			if(strcmp(argv[1],"-v")==0){        
					printf("- usbreset version: %s\nCopyright(c) 2020. XandarKardian. All rights reserved.\n", VERSION);
					return 0;
			}
	}
	char device[100]="/dev/ttyUSB";

    int file_access;  

	int i;
	XK_USB_Scan_All();
	sleep(1);
	for(i=0;i<10;i++){
		sprintf(device, "/dev/ttyUSB%d", i);

		file_access = access(device, R_OK | W_OK);
		if(file_access == -1){
		}
		else if(file_access == 0){
			XK_USB_Reset(i);
		}
	}
	XK_USB_Scan_All();

	return 0;
}



