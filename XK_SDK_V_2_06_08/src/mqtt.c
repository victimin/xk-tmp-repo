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
        return -5;
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

