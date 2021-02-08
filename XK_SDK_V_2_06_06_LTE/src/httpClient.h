#ifndef __UTTPCLIENT_H__
#define __UTTPCLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <error.h>
#include <unistd.h>
#include <time.h>
#include "RadarCommand.h"


#define SERVER_HOST         "show.xandartech.com"
#define SERVER_PAGE         "sensor"
#define SERVER_GETPAGE      "sensor/gettest"

#define CONTENTTYPE         "application/json"
#define CACHE               "no-cache"
#define Connection          "keep-alive"
#define DATA_FORMAT         ".xkd"

#define ROOM_NO 101
#define HOTEL_NAME          "direct"
#define COMPANY             "TEST"
#define ROOM                "101"

typedef enum
{
    SEND_MSG_TYPE_NORMAL,
    SEND_MSG_TYPE_GCP
} SEND_MSG_TYPE;

int create_tcp_socket();
char get_ip(char *host, char *ip);
// void build_get_query(XK_HTTPHandle_t * HTTPHandle, char *host, char *page);
void XK_BuildQuery(XK_HTTPHandle_t *HTTPHandle, char *host, char *page, char *query);
void GetData(XK_HTTPHandle_t * HTTPHandle, char *output);
void usage();
int HTTPConfigInit(void);
char SendRadarData(XK_HTTPHandle_t * HTTPHandle);

void XK_SockInit(void);

void startServer(char *port);
int respond(XK_HTTPHandle_t *HTTPHandle, int clients);
void build_get_resp_query(XK_HTTPHandle_t *HTTPHandle, char * page, char *query);

char isNewWorld;

#ifdef __cplusplus
}
#endif

#endif

