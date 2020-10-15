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
void XK_GetStringFromArray(char * dest, const JSON_Array *array, int idx);
// int XK_CheckHostType(cfgInfo_t * cfgInfo, char *rawHost);
int XK_CheckHostType(XK_ConfigInfo_t * cfgInfo, char *rawHost);

/******************************************** 
 *  SDK config JSON parsing section 
 *  Add JSON Object 
*********************************************/
void InitInfo(XK_ConfigInfo_t * cfgInfo){
    JSON_Value *confValFromFile;
    JSON_Object *confMainObj;
    JSON_Object *confInfoObj;
    JSON_Object *confConfObj;
        
    JSON_Object *confConfHttpObj;
    JSON_Object *confConfMqttObj;
    JSON_Object *confConfMytsObj;

    JSON_Object *confConfHttpCertObj;
    JSON_Object *confConfHttpEndpObj;
    JSON_Object *confConfHttpServerObj;

    JSON_Object *infoObject;
    JSON_Object *configObject;
    JSON_Object *endpointObject;
    JSON_Object *serverObject;

    JSON_Array *radarDeviceList;
    
    char cfg_host_tmp[100];

    int i;
    
    GetCPUSerial(cfgInfo->cpuSerial, cfgInfo->cpuHardware); 

    XK_GetMAC(cfgInfo->MAC_e_Addr, MAC_E_PATH);
    XK_GetMAC(cfgInfo->MAC_w_Addr, MAC_W_PATH);

    // Parsing value from file
    confValFromFile = XK_ParseFile(CONFIG_PATH);
    //Getting objects from value
    confMainObj = XK_GetObjectFromVal(confValFromFile);
    //Getting objects from high level object
    confInfoObj = XK_GetLowObjFromHighObj_Sel(confMainObj, "info");
    confConfObj = XK_GetLowObjFromHighObj_Sel(confMainObj, "config");

    confConfHttpObj = XK_GetLowObjFromHighObj_Sel(confConfObj, "http");
    confConfMqttObj = XK_GetLowObjFromHighObj_Sel(confConfObj, "mqtt");
    confConfMytsObj = XK_GetLowObjFromHighObj_Sel(confConfObj, "mythings");

    confConfHttpCertObj = XK_GetLowObjFromHighObj_Sel(confConfHttpObj, "certification");
    confConfHttpEndpObj = XK_GetLowObjFromHighObj_Sel(confConfHttpObj, "endpoint");
    confConfHttpServerObj = XK_GetLowObjFromHighObj_Sel(confConfHttpObj, "server");

    //info
    XK_GetStringFromObject(cfgInfo->name, confInfoObj, "name");
    XK_GetStringFromObject(cfgInfo->version, confInfoObj, "version");
    XK_GetStringFromObject(cfgInfo->function, confInfoObj, "function");
    XK_GetStringFromObject(cfgInfo->client, confInfoObj, "client");

    //config
    cfgInfo->flgSystemLog = XK_GetNumFromObject(confConfObj, "system-log");
    cfgInfo->flgDataLog = XK_GetNumFromObject(confConfObj, "data-log");
    XK_GetStringFromObject(cfgInfo->device, confConfObj, "device");

    ////config-http
    cfgInfo->http_onoff = XK_GetNumFromObject(confConfHttpObj, "http-onoff");
    cfgInfo->send_if_changed = XK_GetNumFromObject(confConfHttpObj, "send-if-changed");
    cfgInfo->info_SIC_interval = XK_GetNumFromObject(confConfHttpObj, "SIC-interval");    
    XK_GetStringFromObject(cfgInfo->mode, confConfHttpObj, "mode");
    strlwr(cfgInfo->mode);
    cfgInfo->info_send_period = XK_GetNumFromObject(confConfHttpObj, "send-period");
    XK_GetStringFromObject(&cfgInfo->typeRID, confConfHttpObj, "rID-type");
    strlwr(&cfgInfo->typeRID);
    XK_GetStringFromObject(&cfgInfo->typeData, confConfHttpObj, "data-type");
    strlwr(&cfgInfo->typeData);

    //////config-http-certification
    XK_GetStringFromObject(cfgInfo->auth, confConfHttpCertObj, "authorization");
    XK_GetStringFromObject(cfgInfo->accID, confConfHttpCertObj, "account-ID");

    //////config-http-endpoint
    cfgInfo->port = XK_GetNumFromObject(confConfHttpEndpObj, "port");
    XK_GetStringFromObject(cfg_host_tmp, confConfHttpEndpObj, "host");
    if(cfgInfo->http_onoff == 1) cfgInfo->http_onoff = XK_CheckHostType(cfgInfo, cfg_host_tmp);
    XK_GetStringFromObject(cfgInfo->page, confConfHttpEndpObj, "page");

    //////config-http-server
    cfgInfo->server_port = XK_GetNumFromObject(confConfHttpServerObj, "port");
    
    ////config-http
    cfgInfo->mqtt_onoff = XK_GetNumFromObject(confConfMqttObj, "mqtt-onoff");
    XK_GetStringFromObject(cfgInfo->MqttInfo.address, confConfMqttObj, "address");
    XK_GetStringFromObject(cfgInfo->MqttInfo.topic, confConfMqttObj, "topic");
    XK_GetStringFromObject(cfgInfo->MqttInfo.username, confConfMqttObj, "username");
    XK_GetStringFromObject(cfgInfo->MqttInfo.password, confConfMqttObj, "password");
    XK_GetStringFromObject(cfgInfo->MqttInfo.cert, confConfMqttObj, "cert-key");
    cfgInfo->MqttInfo.mqttPort = XK_GetNumFromObject(confConfMqttObj, "port");
    cfgInfo->MqttInfo.frequency = XK_GetNumFromObject(confConfMqttObj, "frequency");

    cfgInfo->behr_myts_onoff = XK_GetNumFromObject(confConfMytsObj, "behr-myts-onoff");
    
    radarDeviceList = json_object_get_array(confMainObj, "device");

    cfgInfo->radarDeviceNum = json_array_get_count(radarDeviceList);
    printf("Configured device are(is) %d\n", cfgInfo->radarDeviceNum);
    for(i=0 ; i<cfgInfo->radarDeviceNum ; i++){
      XK_GetStringFromArray(cfgInfo->radarDeviceList[i], radarDeviceList, i);
    }

    json_value_free(confValFromFile);
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

void XK_GetStringFromArray(char * dest, const JSON_Array *array, int idx){
    char *tmp = json_array_get_string(array, idx); 
    if(tmp==NULL) 
    {
        LOG_E("XK_GetStringFromArray","Can't get string: %d", idx);        
        XK_Exit();
    }
    strcpy(dest, tmp);
}

int XK_CheckHostType(XK_ConfigInfo_t * cfgInfo, char *rawHost){
    int cntChkDot = 0;
    int cntChkChar = 0;
    int idxPage = 0;
    int http_onoff = 0;    
    char flgIPDomainMode=0;
    int i;
    
    http_onoff=1;
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
      // LOG_I("server-address","Domain address type: %s", rawHost);
      if(strstr( rawHost, "http://") != NULL){
        sprintf(cfgInfo->host, &rawHost[7], idxPage - 7);
      }
      else if(strstr( rawHost, "https://") != NULL){
        sprintf(cfgInfo->host, &rawHost[8], idxPage - 8);
        cfgInfo->port = 443;
      }
      else{
        sprintf(cfgInfo->host, rawHost, idxPage);
      }
      cfgInfo->host_type = XK_HOST_TYPE_DOMAIN;
    }
    else if(cntChkDot >= 3){      
      sprintf(cfgInfo->host, rawHost, idxPage);
      LOG_I("server-address","IP address type: %s", cfgInfo->host);
      cfgInfo->host_type = XK_HOST_TYPE_IPADDR;
    }
    else{
      LOG_I("server-address","No send type");
      http_onoff=0;
      cfgInfo->host_type = XK_HOST_TYPE_NONE;
    }
    return http_onoff;
}

