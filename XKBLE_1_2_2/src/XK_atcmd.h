#ifndef __XK_ATCMD_H__
#define __XK_ATCMD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MYTHINGS_MSG_SZ                     200
#define MYTHINGS_MSG_SEND_LIMIT_SZ          110
#define MYTHINGS_MSG_MAX_NUM_OF_DEV         4

#define MYTHINGS_SEND_PERIOD                120000  //msec

#define BARETECH_USB_DEV        "/dev/ttyACM%d"
#define BARETECH_USB_BAUD       (9600)
#define BARETECH_USB_TIMEOUT    (1)

#define AT_CMD_UNI              "AT-U=%d%c%s%c\r"
#define AT_CMD_RST              "AT-RST\r"
#define AT_CMD_FACTORY_RST      "ATZ\r"
#define AT_CMD_INFO             "ATI\r"
#define AT_EOF                  0x1A
#define AT_TAB                  0x09
#define AT_MSG_MAX_SZ           (20)
#define AT_DEV                  BARETECH_USB_DEV
#define AT_DEV_EC25             "/dev/ttyUSB3"


void XK_ATcmdInit(int *fd);
void XK_ATcmdGetDevId(char *dev);
void XK_ATcmdWriteSimple(int fd, char *cmd);
int XK_ATcmdRead(int fd, int numOfCrlf);
void XK_ATcmdMakeUniMsg(char *destMsg, char *srcMsg);
void XK_ATcmdSendUniMsg(int fd, float id, float hr, float br);
void XK_ATcmdClose(int fd);
int SetInterfaceAttribs(int fd, short *usbStatus, int speed, int parity, int waitTime);




#ifdef __cplusplus
}
#endif


#endif

