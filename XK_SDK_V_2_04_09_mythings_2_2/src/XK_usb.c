#include "XK_usb.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>

// #include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <error.h>


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
            LOG_E("XK_USB_Get_Devnum", "%s file open error",path);
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
            LOG_E("XK_USB_Get_Busnum", "%s file open error",path);
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


