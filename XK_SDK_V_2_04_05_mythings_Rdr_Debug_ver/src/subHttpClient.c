#include "parson.h"
#include "RadarCommand.h"
#include "XK_CommonDefineSet.h"
#include "subHttpClient.h"
#include "Plugin_falldetection.h"
#include "Plugin_inoutCounting.h"

#define INCLUDE_ALL_PARA 0

extern char send_A_type;
int addJsonMultiRadarData(XK_HTTPHandle_t *HTTPHandle, char* msgData){

    int i, j;
    // char bracket_fall[] = ",\r\n\t\"fall%d\":{\r\n\t\t\"func\":\"wmFall\"";
    char bracket_a_sub[] = ",\r\n\t\t\"r%d\":{";
    char TEMPSTR_info_sub[] = "\r\n\t\t\t\"radarID\":\"%d\",\r\n\t\t\t\"application\":\"%d\",\r\n\t\t\t\"serial\":\"%06d%06d\",\r\n\t\t\t\"serial_user\":\"%06d%06d\",\r\n\t\t\t\"port\":\"%d\"";
    char bracket_d_sub[] = "\r\n\t\t}";
    char bracket_d[] = "\r\n\t}";

    char TEMPSTR_v_Type_sub[] = "\r\n\t\t\"v%d\":\"%0.2f\"";
    char TEMPSTR_sub[] = "\r\n\t\t\"%s\":\"%0.2f\"";
    unsigned char dataLength = 0;
    
    unsigned int appNum[MAX_NUM_USB_DEVICE];

    int NumofApp[RADAR_APP_MAX_SZ] = {0};

    for (i = 0; i < MAX_NUM_USB_DEVICE; i++)
    {
        appNum[i] = i;
    #if USE_LEGACY_APPNUM
        if (HTTPHandle->USBStatus[i] == 0 && (HTTPHandle->appNum[i] == FallDetection_WallMount || HTTPHandle->appNum[i] == 8))
    #else
        if (HTTPHandle->USBStatus[i] == 0 && HTTPHandle->appNum[i] == FallDetection_WallMount)
    #endif
        {
            NumofApp[RADAR_APP_WMFALL] = getNumofSet_fall();
        }
    #if USE_LEGACY_APPNUM
        if(HTTPHandle->USBStatus[i] == 0 && (HTTPHandle->appNum[i] == IN_OUT || HTTPHandle->appNum[i] == 11))
    #else
        if(HTTPHandle->USBStatus[i] == 0 && HTTPHandle->appNum[i] == IN_OUT)
    #endif
        {
            NumofApp[RADAR_APP_INOUT] = getNumofSet_Inout();
        }
        if(0/*HTTPHandle->USBStatus[i] == 0 && HTTPHandle->appNum[i] == RADAR_APP_INOUT*/){
            // Maybe this will be filled by the flag or Number of other application.
        }
    }
    
    for (i = 0; i < RADAR_APP_MAX_SZ; i++)
    {
        if(NumofApp[i]>0 && i == RADAR_APP_WMFALL)
        {
            //This section is Fall Only.
            // char bracket_fall[] = ",\r\n\t\"fall%d\":{\r\n\t\t\"func\":\"wmFall\"";
            char bracket_fall[] = ",\r\n\t\"%06d%06d\":{\r\n\t\t\"func\":\"wmFall\",\r\n\t\t\"application\":\"50\"";
            for(int noa=0;noa<NumofApp[i];noa++){
                // sprintf(&msgData[strlen(msgData)], bracket_fall, noa);
                // sprintf(&msgData[strlen(msgData)], ",\r\n\t\t\"serial\":\"%06d%06d\"", (int)getRadarInfo_fall(GET_INFO_F_SERIAL1,0,noa), (int)getRadarInfo_fall(GET_INFO_F_SERIAL2,0,noa)); // DH
                int SelR = (int)getRadarInfo_fall(GET_INFO_F_SELR,0,noa);
                sprintf(&msgData[strlen(msgData)], bracket_fall, (int)getRadarInfo_fall(GET_INFO_F_SERIAL1,SelR,noa), (int)getRadarInfo_fall(GET_INFO_F_SERIAL2,SelR,noa));
                for(int rri=0;rri<NUM_RADAR_FALL;rri++){
                    sprintf(&msgData[strlen(msgData)], bracket_a_sub, rri);
                    // sprintf(&msgData[strlen(msgData)], bracket_a_sub, (int)getRadarInfo_fall(GET_INFO_F_ID,rri,noa));
                    sprintf(&msgData[strlen(msgData)], TEMPSTR_info_sub, 
                                                        (int)getRadarInfo_fall(GET_INFO_F_ID,rri,noa), 
                                                        appNum[i], 
                                                        (int)getRadarInfo_fall(GET_INFO_F_SERIAL1,rri,noa), 
                                                        (int)getRadarInfo_fall(GET_INFO_F_SERIAL2,rri,noa), 
                                                        (int)getRadarInfo_fall(GET_INFO_F_SERIAL_USER1,rri,noa), 
                                                        (int)getRadarInfo_fall(GET_INFO_F_SERIAL_USER2,rri,noa), 
                                                        (int)getRadarInfo_fall(GET_INFO_F_PORT,rri,noa)
                                                        );
                    if(!getRadarInfo_fall(GET_INFO_F_NODATA,rri,noa)) {
#if INCLUDE_ALL_PARA
                        int port_i = (int)getRadarInfo_fall(GET_INFO_F_PORT,rri,noa);
                        for (j = PROTOCOL_INFO_MAX_SZ; j < HTTPHandle->rcvDataSize[port_i]; j++){
                            if (HTTP_send_switch[j]){
                                sprintf(&msgData[strlen(msgData)], ",");
                                if (send_A_type == ON){
                                    sprintf(&msgData[strlen(msgData)], TEMPSTR_v_Type_sub, j - PROTOCOL_INFO_MAX_SZ, HTTPHandle->rcvdRadarData[port_i][j]);
                                }
                                else{
                                    sprintf(&msgData[strlen(msgData)], TEMPSTR_sub, appParamName[appNum[port_i]][j - PROTOCOL_INFO_MAX_SZ], HTTPHandle->rcvdRadarData[port_i][j]);
                                }
                            }
                        }
#endif
                    }
                    else{
                        sprintf(&msgData[strlen(msgData)], ",\r\n\t\t\t\"USB%d\":\"nodata\"",(int)getRadarInfo_fall(GET_INFO_F_PORT,rri,noa));
                    }
                    sprintf(&msgData[strlen(msgData)], bracket_d_sub);
                }

                if (!getNoData_fall(noa))
                {
                    makeJsonFallResult(&msgData[strlen(msgData)],noa);
                }
                else
                {
                    sprintf(&msgData[strlen(msgData)], ",\r\n\t\t\"error\":\"nodata FALL %d\"",noa);
                }

                sprintf(&msgData[strlen(msgData)], bracket_d);
                HTTPHandle->rcvDataSize[i] = 0;
            }
        }
        if(NumofApp[i]>0 && i == RADAR_APP_INOUT)
        {
            //This section is INOUT Only.
            char bracket_inout[] = ",\r\n\t\"inout%d\":{\r\n\t\t\"func\":\"inout\"";
            for(int noa=0;noa<NumofApp[i];noa++){
                sprintf(&msgData[strlen(msgData)], bracket_inout, noa);
                for(int rri=0;rri<NUM_RADAR_INOUT;rri++){
                    // sprintf(&msgData[strlen(msgData)], bracket_a_sub, rri);
                    sprintf(&msgData[strlen(msgData)], bracket_a_sub, (int)getRadarInfo_Inout(GET_INFO_IO_ID,rri,noa));
                    sprintf(&msgData[strlen(msgData)], TEMPSTR_info_sub, 
                                                        (int)getRadarInfo_Inout(GET_INFO_IO_ID,rri,noa), 
                                                        appNum[i], 
                                                        (int)getRadarInfo_Inout(GET_INFO_IO_SERIAL1,rri,noa), 
                                                        (int)getRadarInfo_Inout(GET_INFO_IO_SERIAL2,rri,noa), 
                                                        (int)getRadarInfo_Inout(GET_INFO_IO_PORT,rri,noa)
                                                        );
                    if(!getRadarInfo_Inout(GET_INFO_IO_NODATA,rri,noa)) {
#if INCLUDE_ALL_PARA
                        int port_i = (int)getRadarInfo_Inout(GET_INFO_IO_PORT,rri,noa);
                        for (j = PROTOCOL_INFO_MAX_SZ; j < HTTPHandle->rcvDataSize[port_i]; j++){
                            if (HTTP_send_switch[j]){
                                sprintf(&msgData[strlen(msgData)], ",");
                                if (send_A_type == ON){
                                    sprintf(&msgData[strlen(msgData)], TEMPSTR_v_Type_sub, j - PROTOCOL_INFO_MAX_SZ, HTTPHandle->rcvdRadarData[port_i][j]);
                                }
                                else{
                                    sprintf(&msgData[strlen(msgData)], TEMPSTR_sub, appParamName[appNum[port_i]][j - PROTOCOL_INFO_MAX_SZ], HTTPHandle->rcvdRadarData[port_i][j]);
                                }
                            }
                        }
#endif
                    }
                    else{
                        sprintf(&msgData[strlen(msgData)], ",\r\n\t\t\t\"USB%d\":\"nodata\"",(int)getRadarInfo_Inout(GET_INFO_IO_PORT,rri,noa));
                    }
                    sprintf(&msgData[strlen(msgData)], bracket_d_sub);
                }

                if (!getNoData_Inout(noa)){
                    makeJsonInoutResult(&msgData[strlen(msgData)],noa);
                } else{
                    sprintf(&msgData[strlen(msgData)], ",\r\n\t\t\"error\":\"nodata INOUT %d\"",noa);
                }

                sprintf(&msgData[strlen(msgData)], bracket_d);
                HTTPHandle->rcvDataSize[i] = 0;
            }
        }
    }

    msgData[strlen(msgData)] = '\0';
    
    return 0;
}

