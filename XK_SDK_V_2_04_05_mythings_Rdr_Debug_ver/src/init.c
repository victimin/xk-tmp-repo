#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parson.h"
#include "XK_tools.h"
#include "init.h"

char info_name[20];
char info_version[20];
char info_function[30];
char info_excutable[30];
char info_client[100];

char cfg_Auth[100];
char cfg_AccID[100];
char cfg_device[100];
char cfg_gw_serial[50];
char cfg_host[100];
char cfg_page[100];
char cfg_port[20];
char cfg_mode[20];
char cfg_dataType[20];
char cfg_rIDType[20];

char serial_string[20]={0,};
char hw_string[20]={0,};

char *strlwr(char *str);
JSON_Value * XK_ParseFile(char *file);
JSON_Object * XK_GetObjectFromVal(const JSON_Value *value);
void XK_GetLowObjFromHighObj(const JSON_Object **lowObj, const JSON_Object *highObj, int size);
const JSON_Object *XK_GetLowObjFromHighObj_Sel(const JSON_Object *highObj, char *name);
void XK_GetStringFromObject(char * dest, const JSON_Object *object, char *name);
int XK_GetNumFromObject(const JSON_Object *object, char *name);
int XK_CheckHostType(XK_HTTPHandle_t * XK_HTTPHandle, char *rawHost);

/******************************************** 
 *  SDK config JSON parsing section 
 *  Add JSON Object 
*********************************************/
void InitInfo(XK_HTTPHandle_t * XK_HTTPHandle, XK_InitInfo_t *initInfo){
    JSON_Value *confValFromFile;
    JSON_Object *confLv1Obj;
    JSON_Object *confLv2Obj[XK_LV1_OBJ_SIZE];
        
    JSON_Object *confLv3Obj_endpoint;
    JSON_Object *confLv3Obj_server;

    JSON_Object *infoObject;
    JSON_Object *configObject;
    JSON_Object *endpointObject;
    JSON_Object *serverObject;
    
    char cfg_host_tmp[100];

    int i;
    
    GetCPUSerial(XK_HTTPHandle->info.serial, XK_HTTPHandle->info.hardware); 

    // Parsing value from file
    confValFromFile = XK_ParseFile(CONFIG_PATH);
    //Getting objects from value
    confLv1Obj = XK_GetObjectFromVal(confValFromFile);
    //Getting objects from high level object
    confLv2Obj[XK_LV1_OBJ_INFO] = XK_GetLowObjFromHighObj_Sel(confLv1Obj, "info");
    confLv2Obj[XK_LV1_OBJ_CONFIG] = XK_GetLowObjFromHighObj_Sel(confLv1Obj, "config");
    confLv3Obj_endpoint = XK_GetLowObjFromHighObj_Sel(confLv2Obj[XK_LV1_OBJ_CONFIG], "endpoint");
    confLv3Obj_server = XK_GetLowObjFromHighObj_Sel(confLv2Obj[XK_LV1_OBJ_CONFIG], "server");

    //info
    XK_GetStringFromObject(XK_HTTPHandle->info.name, confLv2Obj[XK_LV1_OBJ_INFO], "name");
    XK_GetStringFromObject(XK_HTTPHandle->info.version, confLv2Obj[XK_LV1_OBJ_INFO], "version");
    XK_GetStringFromObject(XK_HTTPHandle->info.function, confLv2Obj[XK_LV1_OBJ_INFO], "function");
    XK_GetStringFromObject(XK_HTTPHandle->info.client, confLv2Obj[XK_LV1_OBJ_INFO], "client");

    //config    
    XK_GetStringFromObject(XK_HTTPHandle->info.auth, confLv2Obj[XK_LV1_OBJ_CONFIG], "authorization");
    XK_GetStringFromObject(XK_HTTPHandle->info.accID, confLv2Obj[XK_LV1_OBJ_CONFIG], "account-ID");
    XK_GetStringFromObject(XK_HTTPHandle->info.device, confLv2Obj[XK_LV1_OBJ_CONFIG], "device");

    XK_GetStringFromObject(initInfo->mode, confLv2Obj[XK_LV1_OBJ_CONFIG], "mode");
    strlwr(initInfo->mode);

    initInfo->flgSendData = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "send-to-server");
    // XK_HTTPHandle->info.send_if_changed = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "send-if-changed");
    initInfo->send_if_changed = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "send-if-changed");
    initInfo->info_SIC_interval = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "SIC-interval");

    initInfo->info_send_period = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "send-period");
    initInfo->flgSystemLog = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "system-log");
    initInfo->flgDataLog = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "data-log");
    initInfo->flgAutoReboot = XK_GetNumFromObject(confLv2Obj[XK_LV1_OBJ_CONFIG], "auto-reboot");

    XK_GetStringFromObject(&initInfo->typeData, confLv2Obj[XK_LV1_OBJ_CONFIG], "data-type");
    strlwr(&initInfo->typeData);
    XK_GetStringFromObject(&initInfo->typeRID, confLv2Obj[XK_LV1_OBJ_CONFIG], "rID-type");
    strlwr(&initInfo->typeRID);

    XK_HTTPHandle->info.port = XK_GetNumFromObject(confLv3Obj_endpoint, "port");
    XK_GetStringFromObject(cfg_host_tmp, confLv3Obj_endpoint, "host");
    if(initInfo->flgSendData == 1) initInfo->flgSendData = XK_CheckHostType(XK_HTTPHandle, cfg_host_tmp);
    XK_GetStringFromObject(XK_HTTPHandle->info.page, confLv3Obj_endpoint, "page");
    XK_HTTPHandle->info.server_port = XK_GetNumFromObject(confLv3Obj_server, "port");
    
    json_value_free(confValFromFile);
}

/******************************************** 
 *  SDK config JSON parsing section 
 *  Add JSON Object 
*********************************************/
void InitParamSize(void){

  JSON_Value *paramVal;  
  JSON_Object *appNumObj;
  JSON_Array *array[20];
	int arraySize[20];
  int loopi, loopj;
  
  paramVal = json_parse_file(PARAM_PATH);
  appNumObj = json_value_get_object(paramVal);
  
  array[1] = json_object_get_array(appNumObj, "1");
  array[2] = json_object_get_array(appNumObj, "2");
  array[3] = json_object_get_array(appNumObj, "3");
  array[4] = json_object_get_array(appNumObj, "4");
  array[5] = json_object_get_array(appNumObj, "5");
  array[6] = json_object_get_array(appNumObj, "6");
  array[7] = json_object_get_array(appNumObj, "7");
  array[8] = json_object_get_array(appNumObj, "8");
  array[9] = json_object_get_array(appNumObj, "9");
  array[10] = json_object_get_array(appNumObj, "10");

  for(loopi=1;loopi<10;loopi++) arraySize[loopi] = json_array_get_count(array[loopi]);

  for(loopi=1;loopi<10;loopi++){
    for(loopj=0;loopj<arraySize[loopi];loopj++){
      HTTP_send_switch[loopi][loopj] = json_array_get_number(array[loopi], loopj);
    }
    HTTP_send_switch[loopi][loopj] = -1;
  }
  
  json_value_free(paramVal);
}

/* XandarKardian */
char *strlwr(char *str){
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}


JSON_Value * XK_ParseFile(char *file){
    JSON_Value *tmp;
    tmp = json_parse_file(file);
    if(tmp == NULL) 
    {
        LOG_E("XK_ParseFile","Can't parse file");        
        XK_Exit();
    }
    return tmp;
}

JSON_Object * XK_GetObjectFromVal(const JSON_Value *value){
    JSON_Object *tmp;
    tmp = json_value_get_object(value);
    if(tmp == NULL) 
    {
        LOG_E("XK_GetObjectFromVal","Can't get object");        
        XK_Exit();
    }
    return tmp;
}

void XK_GetLowObjFromHighObj(const JSON_Object **lowObj, const JSON_Object *highObj, int size){
    JSON_Object *tmp[size];
    int i;

    for(i=0;i<size;i++){
        tmp[i] = json_object_get_object(highObj, highObj->names[i]);
        if(tmp[i] == NULL) 
        {
            LOG_E("XK_GetLv2ObjFromLv1Obj","Can't get object: %d", i);        
            XK_Exit();
        }
    }   
    for(i=0;i<size;i++) lowObj[i] = tmp[i];
}

const JSON_Object *XK_GetLowObjFromHighObj_Sel(const JSON_Object *highObj, char *name){
    JSON_Object *tmp;
    int i;

    tmp = json_object_get_object(highObj, name);
    if(tmp == NULL) 
    {
        LOG_E("XK_GetLowObjFromHighObj_Sel","Can't get object: %s", name);        
        XK_Exit();
    }
    return tmp;
}

void XK_GetStringFromObject(char * dest, const JSON_Object *object, char *name){
    char *tmp = json_object_get_string(object, name); 
    if(tmp==NULL) 
    {
        LOG_E("XK_GetStringFromObject","Can't get string: %s", name);        
        XK_Exit();
    }
    strcpy(dest, tmp);
}

int XK_GetNumFromObject(const JSON_Object *object, char *name){
    int tmp = json_object_get_number(object, name);

    if(tmp==-99) 
    {
        LOG_E("XK_GetNumFromObject","Can't get number: %s", name);        
        XK_Exit();
    }
    return tmp;
}

int XK_CheckHostType(XK_HTTPHandle_t * XK_HTTPHandle, char *rawHost){
    int cntChkDot = 0;
    int cntChkChar = 0;
    int idxPage = 0;
    int flgSendData = 0;    
    char flgIPDomainMode=0;
    int i;
    
    flgSendData=1;
    for(i=0;i<strlen(rawHost);i++){
      if(rawHost[i] == '.') cntChkDot++;
      if(rawHost[i] == '/' && cntChkDot){
          idxPage = i;
          break;
      }
      if( (rawHost[i]>='a' && rawHost[i]<='z') || (rawHost[i]>='A' && rawHost[i]<='Z')) {
        cntChkChar++;
        // flgIPDomainMode++;
      }
    }
    if(cntChkChar && cntChkDot) flgIPDomainMode = 1;

    if(flgIPDomainMode){
      LOG_I("server-address","Domain address type: %s", rawHost);
      if(strstr( rawHost, "http://") != NULL){
        sprintf(XK_HTTPHandle->info.host, &rawHost[7], idxPage - 7);
      }
      else if(strstr( rawHost, "https://") != NULL){
        sprintf(XK_HTTPHandle->info.host, &rawHost[8], idxPage - 8);
        XK_HTTPHandle->info.port = 443;
      }
      else{
        sprintf(XK_HTTPHandle->info.host, rawHost, idxPage);
      }
      XK_HTTPHandle->info.host_type = XK_HOST_TYPE_DOMAIN;
    }
    else if(cntChkDot >= 3){      
      sprintf(XK_HTTPHandle->info.host, rawHost, idxPage);
      LOG_I("server-address","IP address type: %s", XK_HTTPHandle->info.host);
      XK_HTTPHandle->info.host_type = XK_HOST_TYPE_IPADDR;
    }
    else{
      LOG_I("server-address","No send type");
      flgSendData=0;
      XK_HTTPHandle->info.host_type = XK_HOST_TYPE_NONE;
    }
    return flgSendData;
}

