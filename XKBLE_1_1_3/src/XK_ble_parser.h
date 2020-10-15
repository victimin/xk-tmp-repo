#ifndef __XK_BLE_PARSER__
#define __XK_BLE_PARSER__

#ifdef __cplusplus
extern "C"
{
#endif

#include <curses.h>

#define ACCNR_BLE_HEADER			0x020104

typedef enum
{
	CONN_STATUS_DISCONNECTED = -1,
	CONN_STATUS_NONE = 0,
	CONN_STATUS_CONNECTED = 1
	
} CONN_STATUS;

typedef struct {
	char advFlgs[3];
	char advHeader[2];
	char companyID[2];
	char iBconType;
	char iBconLen;
} stIBconPrefix;

typedef struct {
	int cntData;
	int connectionStatus;
	stIBconPrefix iBconPrefix;
	char UUID[16];
	char majorNum[2];
	char minorNum[2];
	char txPower;
	int RSSI;
} stIBconData;

typedef enum {
	IBCON_PACKET_IDX_ADV_FLG_0,
	IBCON_PACKET_IDX_ADV_FLG_1,
	IBCON_PACKET_IDX_ADV_FLG_2,
	IBCON_PACKET_IDX_ADV_HEADER_0,
	IBCON_PACKET_IDX_ADV_HEADER_1,
	IBCON_PACKET_IDX_COMP_ID_0,
	IBCON_PACKET_IDX_COMP_ID_1,
	IBCON_PACKET_IDX_IBCON_TYPE,
	IBCON_PACKET_IDX_IBCON_LEN,
	IBCON_PACKET_IDX_UUID_0,
	IBCON_PACKET_IDX_UUID_1,
	IBCON_PACKET_IDX_UUID_2,
	IBCON_PACKET_IDX_UUID_3,
	IBCON_PACKET_IDX_UUID_4,
	IBCON_PACKET_IDX_UUID_5,
	IBCON_PACKET_IDX_UUID_6,
	IBCON_PACKET_IDX_UUID_7,
	IBCON_PACKET_IDX_UUID_8,
	IBCON_PACKET_IDX_UUID_9,
	IBCON_PACKET_IDX_UUID_10,
	IBCON_PACKET_IDX_UUID_11,
	IBCON_PACKET_IDX_UUID_12,
	IBCON_PACKET_IDX_UUID_13,
	IBCON_PACKET_IDX_UUID_14,
	IBCON_PACKET_IDX_UUID_15,
	IBCON_PACKET_IDX_MAJOR_0,
	IBCON_PACKET_IDX_MAJOR_1,
	IBCON_PACKET_IDX_MINOR_0,
	IBCON_PACKET_IDX_MINOR_1,
	IBCON_PACKET_IDX_RSSI,
} IBCON_PACKET_IDX;

#define XK_BLEPKT_LEN_MAX 	(40)
#define XK_BLEPKT_UUID_N 	(16)

typedef struct {
	uint8_t ptype;
	uint8_t event;
	uint8_t plen;
	uint8_t subevent;
	uint8_t num_reports;
	uint8_t dummy[2]; // no needed

	uint8_t MACaddr[6];
	uint8_t payload_len;
	uint8_t payload[XK_BLEPKT_LEN_MAX];
} xk_ble_pkt_t;

typedef struct {
	uint8_t len;
	uint8_t type;
	union {
		struct {
			uint16_t company;
			uint8_t msg_type;
			uint8_t msg_len;
			uint8_t UUID[XK_BLEPKT_UUID_N];
			uint16_t major;
			uint16_t minor;
			uint8_t txPwr;
			uint8_t RSSI;
		};
		uint8_t data[4+XK_BLEPKT_UUID_N+4+1];
	};
} xk_ble_pdu_t;

typedef struct {
	char ver_fw[4];
	float btry;
	float Pres_scr_min;
	float Pres_scr_max;
	float Pres_scr_avr;

	unsigned int dwellTime;

	char pres;
	char pres_cm;
	char cursor;
} xk_ble_data_t;

int ParseBleData(char * data, stIBconData * dest, int len);
void xk_parseMsg(stIBconData * dest,xk_ble_data_t* data);
void xk_parsePrint(xk_ble_data_t* data);
void xk_findAccAll(xk_ble_pkt_t* buf);
void xk_printDetectMAClistAll(void);

#ifdef __cplusplus
}
#endif
#endif

