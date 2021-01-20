#include "RadarCommand.h"

static const char *TAG = "SERVER CMD";

Sendmsg_t sendMsgCasting;
TartgetRadar_t targetRadar;

extern char gFlgPause;
float gPayload[128]={0.0, };
unsigned char gPayloadDec[128]={0, };

extern char gTypeData;
extern char gTypeRID;

typedef enum
{
	CMD_ID,
	CMD_HEADER,
	CMD_CMD,
	CMD_PAYLOAD	
} XK_CMD_PROTOCOL_SEQ;


void RadarCommandUART(int fd, Sendmsg_t * arrParam, int numParam){
    int loopi, loopj;
	int idxSOF = 0;
	int idxCMD = 0;
	int idxEOF = 0;
	char cmdTarget = CMD_TO_RADAR;

	// uart_write_bytes(uart_num, &arrParam->c[0], numParam*4);
	write(fd, &arrParam->c[0 + 4], numParam*4);
	
	printf("### RADAR CMD = [");
	for(loopi=0;loopi<numParam+1;loopi++) printf("%f ", arrParam->f[loopi]);
	printf("]\n\r");
}

//	RadarID	CMD			Payload
//	31001	-1
//	31001	1010.1010	0	999	255.255	255.255
//	31001	4040.4040	0	255.255
void CommandParser(XK_HTTPHandle_t * HTTPHandle, JSON_Array *array){
    int loopi, loopj, i;
	int idxSOF = 0;
	int idxCMD = 0;
	int idxEOF = 0;
	
	int numParam = json_array_get_count(array);
	char cmdTarget = CMD_TO_RADAR;
	int cntPayload=0;
	char tmpStr[10];
    char ExCmd[100] = {0, };
    int retval;
	int eventChk = 0;
	float tmp_payload;

	targetRadar.ID = 0;
	targetRadar.port = -1;
	if (array != NULL && numParam > 0) {
		for (loopi = 0; loopi < numParam; loopi++) {
			sendMsgCasting.f[loopi] = json_array_get_number(array, loopi);
			// LOG_I(TAG, "CMD DATA Received: %f", json_array_get_number(array, loopi));
		}
			
		if(sendMsgCasting.f[CMD_HEADER]==RPI_CMD_HEADER){
			targetRadar.ID = sendMsgCasting.f[CMD_ID];
			for (i = 0; i < MAX_DEVICE_NUM; i++){
				if(HTTPHandle->radarID[i] == targetRadar.ID) {
					targetRadar.port = i;
					printf("R-ID: %d,	port: %d \n", targetRadar.ID, targetRadar.port);
					eventChk++;
				}
			}

			cmdTarget = CMD_TO_RPI;
			idxCMD = CMD_CMD;
			for (loopi = CMD_PAYLOAD; loopi < numParam; loopi++) {
				gPayload[cntPayload++] = sendMsgCasting.f[loopi];
				if(sendMsgCasting.f[loopi] == RPI_CMD_FOOTER){
					idxEOF = loopi;	
					break;				
				}
			}
			// if(eventChk>1) {
			// 	LOG_E("CMD", "Multiple radar detected!!");
			// 	return 0;
			// }
			// else if(eventChk==0){
			// 	LOG_E("CMD", "Invalid radar ID!!");
			// 	// return 0;
			// }
		}

		if(cmdTarget == CMD_TO_RPI && idxEOF > 0){
			
			JSON_Value *aConfigVal;	
			JSON_Object *aConfigObject;
			JSON_Object *configObject;

			aConfigVal = json_parse_file(CONFIG_PATH);
    		aConfigObject = json_value_get_object(aConfigVal);   
			configObject = json_object_get_object(aConfigObject, "config");
			
			printf("### GATEWAY CMD = [");
			for(loopi=0;loopi<numParam;loopi++) printf("%f ", sendMsgCasting.f[loopi]);
			printf("]\n\r");

			switch((int)sendMsgCasting.f[idxCMD]){
				// 0:
				case RPI_CMD_CHG_RCV_DATA:
					LOG_I("", "gooooooooooooooooooooooooooood");
				break;	

				// 1:
				//ota 0 1// ota [port] [appnum]
				// case RPI_CMD_OTA_RADAR:
				// 	gFlgPause = 1;
					
				// 	for (i = 0; i < MAX_DEVICE_NUM; i++){
				// 		if(HTTPHandle->radarID[i] == (int)targetRadar.ID) {
				// 			sprintf(ExCmd, "sudo chmod 777 %s", GATEWAY_OTA_PATH);
				// 			retval = system(ExCmd);
				// 			if (retval == 127)
				// 			{
				// 				LOG_E("Rpi-OTA", "can't execute firmware update tool!!");
				// 			}
				// 			else if (retval == -1)
				// 			{
				// 				LOG_E("Rpi-OTA", "fork error!!");
				// 			}
				// 			else
				// 			{
				// 			}
				// 			sprintf(ExCmd, "sudo %s %d %d", GATEWAY_OTA_PATH, i, HTTPHandle->appNum[i]);
				// 			// sprintf(ExCmd, "sudo %s %d %d", GATEWAY_OTA_PATH, i, (int)gPayload[1 + 2]);
				// 			// sprintf(ExCmd, "sudo tools/ota %d %d", (int)gPayload[0 + 2], (int)gPayload[1 + 2]);
				// 			retval = system(ExCmd);
				// 			if (retval == 127)
				// 			{
				// 				LOG_E("Rpi-OTA", "can't execute firmware update tool!!");
				// 			}
				// 			else if (retval == -1)
				// 			{
				// 				LOG_E("Rpi-OTA", "fork error!!");
				// 			}
				// 			else
				// 			{
				// 			}
				// 			break;
				// 		}
				// 	}	
				// 	gFlgPause = 0;
				// break;	

				// 2:
				case RPI_CMD_RID_FORCE_CHANGE:		
					sendMsgCasting.f[1] = 1010.1010;
					sendMsgCasting.f[2] = 0;
					sendMsgCasting.f[3] = 0;
					sendMsgCasting.f[4] = 255.255;
					sendMsgCasting.f[5] = 255.255;
					for (i = 0; i < MAX_DEVICE_NUM; i++){
						if(HTTPHandle->radarID[i] == (int)targetRadar.ID) {
							RadarCommandUART(HTTPHandle->fd[i], &sendMsgCasting, 5);
							break;
						}
					}				
				break;	

				// 3:
				case RPI_CMD_RID_TYPE_CHANGE:
					tmp_payload = gPayload[0];
					printf("%f\n", tmp_payload);
					if(tmp_payload==0) {
						gTypeRID = 'r';
						json_object_set_string(configObject, "rID-type", "r");
						json_serialize_to_file_pretty(aConfigVal, CONFIG_PATH);
					}
					else {
						gTypeRID = 's';
						json_object_set_string(configObject, "rID-type", "s");
						json_serialize_to_file_pretty(aConfigVal, CONFIG_PATH);
					}
				break;	

				// 4:
				case RPI_CMD_DATA_TYPE_CHANGE:
					tmp_payload = gPayload[0];
					printf("%f\n", tmp_payload);
					if(tmp_payload==0){
						gTypeData = 'a';
						json_object_set_string(configObject, "data-type", "a");
						json_serialize_to_file_pretty(aConfigVal, CONFIG_PATH);
					}
					else{
						gTypeData = 'v';
						json_object_set_string(configObject, "data-type", "v");
						json_serialize_to_file_pretty(aConfigVal, CONFIG_PATH);
					}
				break;	

				// 5:
				case RPI_CMD_SYSTEM_SCRIPT:
					// LOG_I("", "It will be updated");
				    system("sudo /boot/XandarKardian/script/xk.sh &");
					delay(1000);

				break;	

				// 6:
				case RPI_CMD_TEAMVEIWER_RESET:
                    // system("sudo teamviewer --daemon restart");
                    system("sudo systemctl restart teamviewerd.service");
					LOG_I("Server CMD", "sudo systemctl restart teamviewerd.service");
					delay(1000);

				break;	

				// 7:
				case RPI_CMD_WIFI_RESET:
					system("sudo ifconfig wlan0 down");
					system("sudo ifconfig wlan0 up");
					LOG_I("Server CMD", "WiFi Reconnecting...");
					delay(1000);

				break;	

				// 8:
				case RPI_CMD_SYSTEM_REBOOT:
				    system("sudo shutdown -r now");
					// delay(1000);

				break;	
				// // 0:
				// case RPI_CMD_CHG_RCV_DATA:
				// break;	

				//Default:
				default:
					LOG_E(TAG, "Invalid command!!!");
				break;	
			}
			
    		json_value_free(aConfigVal);
		}
		else{
			if((int)sendMsgCasting.f[0] == 0) LOG_W("CMD", "Not recommended that the Radar-ID is 0.");
			for (i = 0; i < MAX_DEVICE_NUM; i++){
				if(HTTPHandle->radarID[i] == (int)sendMsgCasting.f[0]) {			
					eventChk++;
				}
			}
			if(eventChk>1) {
				LOG_E("CMD", "Multiple radar detected!!");
				return 0;
			}
			else if(eventChk==0){
				LOG_E("CMD", "Invalid radar ID!!");
				// return 0;
			}
			// Web Command ex) 1 -1
			// Web Command ex) 1 1010.1010 0 999 255.255 255.255
			for (i = 0; i < MAX_DEVICE_NUM; i++){
				if(HTTPHandle->radarID[i] == (int)sendMsgCasting.f[0]) {
					RadarCommandUART(HTTPHandle->fd[i], &sendMsgCasting, json_array_get_count(array)-1);
				}
			}
			LOG_I("CMD", "Total = %d event(s) occurred!!", eventChk);
		}
	}	
}

