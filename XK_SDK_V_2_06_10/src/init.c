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
    JSON_Object *confMainObj;
    JSON_Object *confInfoObj;
    JSON_Object *confConfObj;
        
    JSON_Object *confConfHttpObj;
    JSON_Object *confConfMqttObj;

    JSON_Object *confConfHttpCertObj;
    JSON_Object *confConfHttpEndpObj;
    JSON_Object *confConfHttpServerObj;

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
    confMainObj = XK_GetObjectFromVal(confValFromFile);
    //Getting objects from high level object
    confInfoObj = XK_GetLowObjFromHighObj_Sel(confMainObj, "info");
    confConfObj = XK_GetLowObjFromHighObj_Sel(confMainObj, "config");

    confConfHttpObj = XK_GetLowObjFromHighObj_Sel(confConfObj, "http");
    confConfMqttObj = XK_GetLowObjFromHighObj_Sel(confConfObj, "mqtt");

    confConfHttpCertObj = XK_GetLowObjFromHighObj_Sel(confConfHttpObj, "certification");
    confConfHttpEndpObj = XK_GetLowObjFromHighObj_Sel(confConfHttpObj, "endpoint");
    confConfHttpServerObj = XK_GetLowObjFromHighObj_Sel(confConfHttpObj, "server");

    //info
    XK_GetStringFromObject(XK_HTTPHandle->info.name, confInfoObj, "name");
    XK_GetStringFromObject(XK_HTTPHandle->info.version, confInfoObj, "version");
    XK_GetStringFromObject(XK_HTTPHandle->info.function, confInfoObj, "function");
    XK_GetStringFromObject(XK_HTTPHandle->info.client, confInfoObj, "client");

    //config
    initInfo->flgSystemLog = XK_GetNumFromObject(confConfObj, "system-log");
    initInfo->flgDataLog = XK_GetNumFromObject(confConfObj, "data-log");
    initInfo->flgAutoReboot = XK_GetNumFromObject(confConfObj, "auto-reboot");
    XK_GetStringFromObject(XK_HTTPHandle->info.device, confConfObj, "device");

    ////config-http
    initInfo->flgSendData = XK_GetNumFromObject(confConfHttpObj, "http-onoff");
    initInfo->send_if_changed = XK_GetNumFromObject(confConfHttpObj, "send-if-changed");
    initInfo->info_SIC_interval = XK_GetNumFromObject(confConfHttpObj, "SIC-interval");    
    XK_GetStringFromObject(initInfo->mode, confConfHttpObj, "mode");
    strlwr(initInfo->mode);
    initInfo->info_send_period = XK_GetNumFromObject(confConfHttpObj, "send-period");
    XK_GetStringFromObject(&initInfo->typeRID, confConfHttpObj, "rID-type");
    strlwr(&initInfo->typeRID);
    XK_GetStringFromObject(&initInfo->typeData, confConfHttpObj, "data-type");
    strlwr(&initInfo->typeData);

    //////config-http-certification
    XK_GetStringFromObject(XK_HTTPHandle->info.auth, confConfHttpCertObj, "authorization");
    XK_GetStringFromObject(XK_HTTPHandle->info.accID, confConfHttpCertObj, "account-ID");

    //////config-http-endpoint
    XK_HTTPHandle->info.port = XK_GetNumFromObject(confConfHttpEndpObj, "port");
    XK_GetStringFromObject(cfg_host_tmp, confConfHttpEndpObj, "host");
    if(initInfo->flgSendData == 1) initInfo->flgSendData = XK_CheckHostType(XK_HTTPHandle, cfg_host_tmp);
    XK_GetStringFromObject(XK_HTTPHandle->info.page, confConfHttpEndpObj, "page");

    //////config-http-server
    XK_HTTPHandle->info.server_port = XK_GetNumFromObject(confConfHttpServerObj, "port");

    ////config-http
    initInfo->mqtt_onoff = XK_GetNumFromObject(confConfMqttObj, "mqtt-onoff");
    XK_GetStringFromObject(XK_HTTPHandle->MqttInfo.address, confConfMqttObj, "address");
    XK_GetStringFromObject(XK_HTTPHandle->MqttInfo.topic, confConfMqttObj, "topic");
    XK_GetStringFromObject(XK_HTTPHandle->MqttInfo.username, confConfMqttObj, "username");
    XK_GetStringFromObject(XK_HTTPHandle->MqttInfo.password, confConfMqttObj, "password");
    XK_GetStringFromObject(XK_HTTPHandle->MqttInfo.cert, confConfMqttObj, "cert-key");
    XK_HTTPHandle->MqttInfo.mqttPort = XK_GetNumFromObject(confConfMqttObj, "port");
    XK_HTTPHandle->MqttInfo.frequency = XK_GetNumFromObject(confConfMqttObj, "frequency");

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
    if(strlen(tmp)==1){
      *dest = tmp[0];
    }
    else{
      strcpy(dest, tmp);
    }
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

