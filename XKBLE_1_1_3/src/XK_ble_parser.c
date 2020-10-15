#include "XK_ble_parser.h"
#include "XK_tools.h"

static void xk_decodingMsg(stIBconData * dest);
static void pos_chg(uint8_t* b);


#define FIND_ACC_MAX_N 100
char MACaddrList[FIND_ACC_MAX_N][6];
int MACaddrListRSSI[FIND_ACC_MAX_N];
char MACaddrCursor = 0;

int ParseBleData(char * data, stIBconData * dest, int len){
    int i;
    // for(i=0;i<len;i++){
    //     printf("%02x ", data[i]);
    // }
    // printf("\n");
    // for(i=0;i<3;i++) printf("%02x ", dest->iBconPrefix.advFlgs[i]);
    // printf("\n");
    // for(i=0;i<2;i++) printf("%02x ", dest->iBconPrefix.advHeader[i]);
    // printf("\n");
    // for(i=0;i<2;i++) printf("%02x ", dest->iBconPrefix.companyID[i]);
    // printf("\n");
    // for(i=0;i<1;i++) printf("%02x ", dest->iBconPrefix.iBconType);
    // printf("\n");
    // for(i=0;i<1;i++) printf("%02x ", dest->iBconPrefix.iBconLen);
    // printf("\n");
    // for(i=0;i<16;i++) printf("%02x ", dest->UUID[i]);
    // printf("\n");
    // for(i=0;i<2;i++) printf("%02x ", dest->majorNum[i]);
    // printf("\n");
    // for(i=0;i<2;i++) printf("%02x ", dest->minorNum[i]);
    // printf("\n");
    memcpy(dest->iBconPrefix.advFlgs, &data[IBCON_PACKET_IDX_ADV_FLG_0], 3);
    memcpy(dest->iBconPrefix.advHeader, &data[IBCON_PACKET_IDX_ADV_HEADER_0], 2);
    memcpy(dest->iBconPrefix.companyID, &data[IBCON_PACKET_IDX_COMP_ID_0], 2);
    memcpy(&dest->iBconPrefix.iBconType, &data[IBCON_PACKET_IDX_IBCON_TYPE], 1);
    memcpy(&dest->iBconPrefix.iBconLen, &data[IBCON_PACKET_IDX_IBCON_LEN], 1);
    memcpy(dest->UUID, &data[IBCON_PACKET_IDX_UUID_0], 16);
    memcpy(dest->majorNum, &data[IBCON_PACKET_IDX_MAJOR_0], 2);
    memcpy(dest->minorNum, &data[IBCON_PACKET_IDX_MINOR_0], 2);

    xk_decodingMsg(dest);

}

void xk_findAccAll(xk_ble_pkt_t* buf){
    xk_ble_pkt_t* xk_data = buf;

    // if(xk_data->event == EVT_LE_META_EVENT && xk_data->subevent == EVT_LE_ADVERTISING_REPORT){
    if(xk_data->event == 0x3E && xk_data->subevent == 0x02){
        char addr[18];
        ba2str(xk_data->MACaddr, addr);
        
        uint8_t pdu_cursor = 0;
        uint8_t cnt_pdu = 0;

        xk_ble_pdu_t* pdu;

        while(pdu_cursor < xk_data->payload_len && cnt_pdu++ < 10){
            pdu = (xk_ble_pdu_t*)(&xk_data->payload[pdu_cursor]);
            pdu_cursor += pdu->len + 1;
            
            if(pdu->type == 0xFF){
                if(pdu->company == 0x8875){
                    int isList = 0;
                    for(int mi=0;mi<MACaddrCursor;mi++){
                        if(memcmp(MACaddrList[mi],xk_data->MACaddr,6)==0){
                            MACaddrListRSSI[mi] = GetComplements(xk_data->payload[xk_data->payload_len]);
                            isList = 1;
                            // GetComplements((char)info->data[info->length]);
                        }
                    }
                    if(!isList){
                        memcpy(MACaddrList[MACaddrCursor],xk_data->MACaddr,6);
                        MACaddrListRSSI[MACaddrCursor] = GetComplements(xk_data->payload[xk_data->payload_len]);
                        MACaddrCursor++;
                    }
                    // printf("company: 0x%04X\n",pdu->company);
                    // printf("%s - len 0x%02X\n", addr, (char)pdu->len);
                    // memcpy(dest->iBconPrefix.advHeader, pdu, 2);
                    // memcpy(dest->iBconPrefix.companyID, &(pdu->company), 2);
                    // memcpy(&dest->iBconPrefix.iBconType, &(pdu->msg_type), 1);
                    // memcpy(&dest->iBconPrefix.iBconLen, &(pdu->msg_len), 1);
                    // memcpy(dest->UUID, pdu->UUID, 16);
                    // memcpy(dest->majorNum, &(pdu->major), 2);
                    // memcpy(dest->minorNum, &(pdu->minor), 2);
                    // dest->connectionStatus = CONN_STATUS_CONNECTED;
                    // dest->cntData++;
                }
            }
            else if(pdu->type == 0x02){
                // memcpy(dest->iBconPrefix.advFlgs, pdu, 3);
            }
        }
    }
}
    // uint8_t ddddd[] = {0x9d,0xa9 ,0x9c ,0x77 ,0xf0 ,0xc7 ,0xab ,0xff ,0x4c ,0xa7 ,0xe6 ,0xf3 ,0x4f ,0xa1 ,0x70 ,0xa6 ,0x96 ,0xe6 ,0xcf ,0x5f};
    // memcpy(d->UUID,ddddd,20);
#define XK_UUID_LENGTH (16)
static void xk_decodingMsg(stIBconData * dest)
{
    xk_ble_pdu_t* d = (xk_ble_pdu_t*)(dest->iBconPrefix.advHeader);

    if(d->company == 0x8875){
        // printf("It is OK.\n");
        pos_chg(&d->major);
        pos_chg(&d->minor);
        
	    uint8_t refCode[2] = {0x88,0x75};
	    uint16_t inc_bit = 0b1001000100001001;
	    uint16_t pos_bit = (d->minor) ^ (inc_bit);
        
	    uint8_t rn[2];
        memcpy(rn,&(d->major),2);

        rn[0] ^= refCode[0];
        rn[1] ^= refCode[1];
        
        for(int p=0;p<XK_UUID_LENGTH;p++){
            d->UUID[XK_UUID_LENGTH-p-1] ^= rn[pos_bit%2];
            d->UUID[XK_UUID_LENGTH-p-1] ^= (uint8_t)inc_bit;
            pos_bit = pos_bit>>1;
            inc_bit = inc_bit>>1;
        }
    }
}

void xk_parseMsg(stIBconData * dest,xk_ble_data_t* data)
{
    xk_ble_pdu_t* d = (xk_ble_pdu_t*)(dest->iBconPrefix.advHeader);

    char ver = d->UUID[15];
    char ver_idx = ver>>6;

    data->ver_fw[ver_idx] = ver & 0b00111111;

    data->pres = d->UUID[0]>>7;
    data->pres_cm = (d->UUID[0]>>6)%2;                      // Preence Status
    data->btry = (float)(d->UUID[0] & 0b00111111)/10.0;
    data->cursor = d->UUID[14];                             // Event Counter
    data->Pres_scr_min = (float)(d->UUID[1])/10;
    data->Pres_scr_max = (float)(d->UUID[2])/10;
    data->Pres_scr_avr = (float)(d->UUID[3])/10;
    data->dwellTime = *(unsigned int*)(&(d->UUID[4]));      // Dwell Time (seconds)
}

void xk_parsePrint(xk_ble_data_t* data)
{
    printf("ver: v%d_%d_%d_p%d\t",data->ver_fw[0],data->ver_fw[1],data->ver_fw[2],data->ver_fw[3]);
    printf("pres: %d/%d\t",data->pres,data->pres_cm);
    printf("btry: %4.0f\t",data->btry);
    printf("cursor: %04d\t",data->cursor);
    printf("Pres_scr: %6.1f / ",data->Pres_scr_min);
    printf("%6.1f / ",data->Pres_scr_max);
    printf("%6.1f / ",data->Pres_scr_avr);
    printf("%07d\n",data->dwellTime);
}

void xk_printDetectMAClistAll(void){
    printf("Found MAC List\n");
    for(int mi=0;mi<MACaddrCursor;mi++){
        printf("%5d: %02X:%02X:%02X:%02X:%02X:%02X  %d dB\n",mi,MACaddrList[mi][5],MACaddrList[mi][4],MACaddrList[mi][3],MACaddrList[mi][2],MACaddrList[mi][1],MACaddrList[mi][0],MACaddrListRSSI[mi]);
    }
    printf("\n\n");
}

static void pos_chg(uint8_t* b)
{
    uint8_t* tmp = b;
    uint8_t ctmp = tmp[0];
    tmp[0] = tmp[1];
    tmp[1] = ctmp;
}
