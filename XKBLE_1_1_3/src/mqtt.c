#include "mqtt.h"

void Publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 2;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    // printf("Message '%s' with delivery token %d delivered\n", payload, token);
}

int OnMessage(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received operation %s\n", payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// int MqttPublish(char * address, char * topic, char * clientID, char * data) {
int MqttPublish(XK_MQTTinfo_t info, char * data) {
    MQTTClient client;
    char tmp[100];
    FILE *file_access;

    sprintf(tmp, "ssl://%s:%d", info.address, info.mqttPort);
    MQTTClient_create(&client, tmp, "none", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    // MQTTClient_create(&client, "tcp://stream.premisehq.co:8883", "none", MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // printf("MQTTClient_create CLEAR\n");

    // MQTTClient_create(&client, info.address, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    conn_opts.username = info.username;
    conn_opts.password = info.password;
    
    conn_opts.ssl = &ssl_opts;
    conn_opts.ssl->enableServerCertAuth  = 0;
    // conn_opts.ssl->keyStore = "cert.pem";
    // conn_opts.ssl->trustStore = "cert.pem";


    file_access = access(info.cert, R_OK | W_OK);
    if(file_access == -1){
        LOG_E("MqttPublish", "No Certification File!");
    }
    else if(file_access == 0){
        
    }


    conn_opts.ssl->privateKey = info.cert;
    conn_opts.ssl->enabledCipherSuites = "TLSv1.2";

    MQTTClient_setCallbacks(client, NULL, NULL, OnMessage, NULL);

    // printf("MQTTClient_setCallbacks CLEAR\n");

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
		return -1;
    }
    
    // printf("MQTTClient_connect CLEAR\n");
    //create device
    Publish(client, info.topic, data);

    // printf("Publish CLEAR\n");
    //listen for operation
    // MQTTClient_subscribe(client, "s/ds", 0);

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
	return 1;
}


void MakeMqttMsg(char * dest, xk_ble_data_t * src, int idx){
    char tmpMacAddr[20]={0,};

    if(strlen(dest)>0) dest[strlen(dest)] = SEND_SEPARATOR_COMMON;
    if(iBconDate[idx].connectionStatus == CONN_STATUS_DISCONNECTED){
        dest[strlen(dest)] = SEND_CMD_COMMON_ERROR;
        dest[strlen(dest)] = SEND_ERROR_DISCONNECTED + 48;
    }
    else if(iBconDate[idx].connectionStatus == CONN_STATUS_NONE){
        dest[strlen(dest)] = SEND_CMD_COMMON_ERROR;
        dest[strlen(dest)] = SEND_ERROR_NONE + 48;
    }
    else{
        dest[strlen(dest)] = 0 + 48;
        dest[strlen(dest)] = 4 + 48;
        dest[strlen(dest)] = 9 + 48;
        
        delChar( configInfo.radarDeviceList[idx], tmpMacAddr, ':' );
        sprintf(&dest[strlen(dest)], "%s", tmpMacAddr);
        
        dest[strlen(dest)] = (src->pres_cm)%10 + 48;

        dest[strlen(dest)] = (src->cursor)%10 + 48;

        dest[strlen(dest)] = (((src->dwellTime)/3600)/10)%10 + 48;
        dest[strlen(dest)] = ((src->dwellTime)/3600)%10 + 48;
        
        dest[strlen(dest)] = ((((src->dwellTime)%3600)/60)/10)%10 + 48;
        dest[strlen(dest)] = (((src->dwellTime)%3600)/60)%10 + 48;
        
        dest[strlen(dest)] = (((src->dwellTime)%60)/10)%10 + 48;
        dest[strlen(dest)] = ((src->dwellTime)%60)%10 + 48;

    }

    if(strlen(dest) >= MQTT_MSG_SEND_LIMIT_SZ) {    
        memset(dest, 0, MQTT_MSG_SZ);        
        dest[0] = SEND_CMD_COMMON_ERROR;
        dest[1] = SEND_ERROR_OVERFLOW_DATA + 48;
    }
    else if(strlen(dest) == 0){
        memset(dest, 0, MQTT_MSG_SZ);        
        dest[0] = SEND_CMD_COMMON_ERROR;
        dest[1] = SEND_ERROR_EMPTY + 48;
    }
    
    dest[strlen(dest)] = '\0';
                
}
