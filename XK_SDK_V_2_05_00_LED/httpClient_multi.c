#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <dirent.h>
#include <error.h>
#include <unistd.h>
#include <time.h>
#include "httpClient.h"
#include "XK_CommonDefineSet.h"
#include <curl/curl.h>
#include <fcntl.h>
#include "parson.h"
#include "RadarCommand.h"

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "XK_usb.h"
#include "subHttpClient.h"

#define GET_SIZE 3000
#define USB_DEVICE_STATUS_NODATA       "no-data"
#define USB_DEVICE_STATUS_NORMAL       "normal"
#define USB_DEVICE_STATUS_NOAPPNUM     "invalid-appnum"

// FILE *fpTmpLog;
// FILE *fpOutput;

// char PATH_temp[200];

// char Data_Path[200];
// char Basic_Path[200];
// char Output_Path[200];
// char flgFirstTime = 0;



// char comStr[100] = "B2B";
// char roomStr[100] = "606";
// char devID[100] = "RASP606";


int test_cnt = 0;
int brktLevel=0;
int objCnt=0;
int groupCnt[100]={0, };

struct sockaddr_in *remote;
// struct sockaddr_in server_addr;

JSON_Value *serverGetVal;
JSON_Object *serverGetObject;

extern ParamQ_Handle_t RadarCMD_Q;
extern char send_A_type;
extern char gTypeData;
extern char gTypeRID;
int a = 0;
char flgOneTime = 0;
extern int gFlgSysLog;
int frame_cnt22 = 0;
int listenfd;

extern struct tm gLocalTime;
// extern XK_USBinfoBySymlink_t infoUSB_bySymlink[USB_DEV_HUB_ADAPTER_SIZE+1];
extern XK_USBinfo_t infoUSB[USB_DEV_PORT_MAX_SIZE];

void GetSerialNumber(char *outData, float *radarData, unsigned int appNum);
int GetMetaInfo(char *outData, float *radarData, unsigned int appNum);

char SendRadarData(XK_HTTPHandle_t *HTTPHandle)
{
    char ip_addr[15];
    char query[1024 * 20]={0,};
    char body[1024 * 20]={0,};
    char TEMPLETE[1024 * 20] = {0};
    char request[GET_SIZE];

    char *host;
    char *page;
    int port;
    int sock;

    char *get;
    int tmpres;
    int i, j;

    char res;
    char *ip;
    char buf[BUFSIZ + 1];

    int recvCnt = 0;
    int bodyIndex = 0;
    int srchStartIdx = 0;
    int accumRecvCnt = 0;
    int cntBrktIn = 0;
    int cntBrktOut = 0;
    clock_t waitTime = clock();
    unsigned long waitGETTime = 0;
    unsigned char flgTimeOut = 0;
    int cntParam = 0;
    JSON_Array *array;
    
    struct ifreq ifr_e;
    char localIP_e[50];
    struct ifreq ifr_w;
    char localIP_w[50];

    host = HTTPHandle->info.host;
    page = HTTPHandle->info.page;
    port = HTTPHandle->info.port;

    BIO *outbio = NULL;
    SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;

    if (port == 443)
    {
        OpenSSL_add_all_algorithms();
        ERR_load_BIO_strings();
        ERR_load_crypto_strings();
        SSL_load_error_strings();

        outbio = BIO_new(BIO_s_file());
        outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

        if (SSL_library_init() < 0)
        {
            BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");
        }

        method = SSLv23_client_method();
        ctx = SSL_CTX_new(method);
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    }

    struct sockaddr_in server_addr;
    sock = socket(PF_INET, SOCK_STREAM, 0); //try AF_INET
    if (-1 == sock)
    {
        printf("Fail to socket create\n");
        return 0;
    }

    memcpy(ifr_e.ifr_name, "eth0", IFNAMSIZ-1);ioctl(sock, SIOCGIFADDR, &ifr_e);
    strcpy(localIP_e, inet_ntoa(((struct sockaddr_in *)&ifr_e.ifr_addr)->sin_addr));
    HTTPHandle->IPAddr_e = localIP_e;

    memcpy(ifr_w.ifr_name, "wlan0", IFNAMSIZ-1);ioctl(sock, SIOCGIFADDR, &ifr_w);
    strcpy(localIP_w, inet_ntoa(((struct sockaddr_in *)&ifr_w.ifr_addr)->sin_addr));
    HTTPHandle->IPAddr_w = localIP_w;

    struct timeval optVal = {5, 000000};
    struct timeval optSndVal = {2, 000000};
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        LOG_E("HTTP", "fail set receive sockopt()");
        close(sock);
        return 0;
    }
    else
    {
    }

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &optSndVal, sizeof(optSndVal)) != 0)
    {
        LOG_E("HTTP", "fail set send sockopt()");
        close(sock);
        return 0;
    }
    else
    {
    }


#if ONOFF_SUB_EPOINT
    if (HTTPHandle->info.host_type == XK_HOST_TYPE_DOMAIN)
#else
    if (flgOneTime == 0 && HTTPHandle->info.host_type == XK_HOST_TYPE_DOMAIN)
#endif
    {
        res = get_ip(host, ip_addr);
        flgOneTime = 1;
        if (res == 0)
        {
            LOG_E("HTTP", "Can't get ip address");
            close(sock);

            flgOneTime = 0;
            return 0;
        }
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(HTTPHandle->info.host_type == XK_HOST_TYPE_DOMAIN) server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    else server_addr.sin_addr.s_addr = inet_addr(host);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG_E("HTTP", "Can't connect");
        close(sock);
        return 0;
    }

    if (port == 443)
    {
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock);
        res = SSL_connect(ssl);
        if (res == 0)
        {
            LOG_E("HTTPS", "Could not connect");
            close(sock);
            return 0;
        }
    }

#if 1 //POST
    build_get_query(HTTPHandle, host, page, query, body);

    if (gFlgSysLog && HTTPHandle->flgParamSizeErr == 1)
    {
        LOG_E("system", "\n->\n<<START>>\n%s\n<<END>>\n\n\n", query);
    }

    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_SEND))
    {
        printf("\n\r\n\rQuery is:\n<<START>>\n%s\n<<END>>\n", query);
    }

    //Send the query to the server
    unsigned int sent = 0;
    while (sent < strlen(query))
    {

        if (port == 443)
        {
            tmpres = SSL_write(ssl, query + sent, strlen(query) - sent);
        }
        else
        {
            tmpres = send(sock, query + sent, strlen(query) - sent, 0);
        }
        if (tmpres == -1)
        {
            LOG_E("HTTP", "Can't send query");
            close(sock);
            return 0;
        }
        else
        {
            // printf("Succeed to send\n");
        }
        sent += tmpres;
    }

    recvCnt = 0;
    accumRecvCnt = 0;
    cntBrktIn = 0;
    cntBrktOut = 0;
    flgTimeOut = 0;
    waitGETTime = clock();
    bzero(request, GET_SIZE);

    while (clock() - waitGETTime < (WAIT_GET_TIME_OUT * CLOCKS_PER_SEC))
    {
        if (port == 443)
        {
            recvCnt = SSL_read(ssl, &request[accumRecvCnt], GET_SIZE - 1);
        }
        else
        {
            recvCnt = recv(sock, &request[accumRecvCnt], GET_SIZE - 1, MSG_NOSIGNAL);
        }
        if (recvCnt == -1)
        {
            LOG_E("HTTP", "Receive Error ");
            close(sock);
            return 0;
        }

        accumRecvCnt += recvCnt;
        for (i = srchStartIdx; i < accumRecvCnt; i++)
        {
            if (request[i] == '\n' && request[i + 1] == '\r')
            {
                bodyIndex = i + 2;
            }
            if (request[i] == '{')
                cntBrktIn++;
            if (request[i] == '}')
                cntBrktOut++;
        }
        if (cntBrktIn == cntBrktOut && cntBrktIn != 0)
        {
            flgTimeOut = 1;
            break;
        }
        srchStartIdx = accumRecvCnt;
    }

    if (flgTimeOut)
    {
        for (i = 0; i < accumRecvCnt; i++)
        {
            if (request[i] == '{')
            {
                bodyIndex = i;
                break;
            }
        }
    }
    else
    {
        LOG_E("HTTP Receive", "Time out...");
        close(sock);
        return 0;
    }

    serverGetVal = json_parse_string(&request[bodyIndex]);
    serverGetObject = json_value_get_object(serverGetVal);

    cntParam = 0;

    array = json_object_get_array(serverGetObject, "param");
    CommandParser(HTTPHandle, array);

    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_RECV))
    {
        printf("\n\r%s\n\r", request);
    }


    json_value_free(serverGetVal);
#endif
    close(sock);

    if (port == 443)
    {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    return 1;
}

char get_ip(char *host, char *ip)
{
    struct hostent *hent;
    int iplen = 15; //XXX.XXX.XXX.XXX
    memset(ip, 0, iplen + 1);

    if ((hent = gethostbyname(host)) == NULL)
    {
        LOG_E("HTTP", "Can't get IP");
        return 0;
    }
    if (inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
    {
        LOG_E("HTTP", "Can't resolve host");
        return 0;
    }
    LOG_I("get_ip", "End point is %s", ip);
    return 1;
}

void build_get_query(XK_HTTPHandle_t *HTTPHandle, char *host, char *page, char *query, char *body)
{
    char *getpage = page;
    int len;
    char tmp[10];
    char tpl[1024];
    char serial_number[50];
    char basicAuth[100];
    sprintf(serial_number, "%s/%s", HTTPHandle->info.hardware, HTTPHandle->info.serial);
    query[0] = 0;    
    if (getpage[0] == '/')
    {
        getpage = getpage + 1;
        LOG_E("get page", "Removing leading \"/\", converting %s to %s\n", page, getpage);
    }
    GetData(HTTPHandle, body);
    int LengthofBody = strlen(body);

    XK_HTTP_ClientSetHeader_POST(tpl, getpage, "HTTP/1.1");
    XK_HTTP_ClientSetHeader(tpl, "Host", host);
    XK_HTTP_ClientSetHeader(tpl, "Connection", Connection);
    XK_HTTP_ClientSetHeader(tpl, "Content-Type", CONTENTTYPE);
    sprintf(basicAuth, "Basic %s",HTTPHandle->info.auth);
    XK_HTTP_ClientSetHeader(tpl, "Authorization", basicAuth);
    XK_HTTP_ClientSetHeader(tpl, "accountId", HTTPHandle->info.accID);
    XK_HTTP_ClientSetHeader(tpl, "x-access-serial", serial_number);
    len = LengthofBody;
    sprintf(tmp, "%d", len);
    XK_HTTP_ClientSetHeader(tpl, "Content-Length", tmp);
    XK_HTTP_ClientSetHeaderClose(tpl);

    query[len] = 0;
    strcat(query, tpl);
    strcat(query, body);
}

void build_get_resp_query(XK_HTTPHandle_t *HTTPHandle, char *query, char *body)
{
    char *getpage = HTTPHandle->info.page;
    int len;
    char *tpl = HTTP_SERVER_HEADER;
    
    GetData(HTTPHandle, body);
    int LengthofBody = strlen(body);
    len = LengthofBody + strlen(tpl) + strlen(CONTENTTYPE) - 3;
    // len = LengthofBody + strlen(host) + strlen(getpage) + strlen(Connection) + strlen(tpl) + strlen(CONTENTTYPE) - 6;
    query[len] = 0;
    sprintf(query, tpl, CONTENTTYPE, LengthofBody, body);
    // return query;
}

void GetData(XK_HTTPHandle_t *HTTPHandle, char *output)
{
    int i, j, k;
    int loopj, loopi;
    char tmp[50];
    char curDate[100];
    unsigned char dataLength = 0;
    int switchCmpTmp[200]={0, };
    int idxApp=-1;
    
    sprintf(curDate, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:00", gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday, gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec , (gLocalTime.tm_gmtoff/(60*60) > 0)? '+':'-', (gLocalTime.tm_gmtoff/(60*60) > 0)? gLocalTime.tm_gmtoff/(60*60):(gLocalTime.tm_gmtoff/(60*60))*(-1));
    
    brktLevel = 0;
    objCnt = 0;
    memset(groupCnt, 0, 100*4);
    TEMPLETE[0] = 0;
    
    XK_ClientAddBracket(TEMPLETE, "{");
    XK_ClientAddBody_s(TEMPLETE, "client", HTTPHandle->info.client);
    XK_ClientAddBody_s(TEMPLETE, "device", HTTPHandle->info.device);
    XK_ClientAddBody_s(TEMPLETE, "function", HTTPHandle->info.function);
    XK_ClientAddBody_s(TEMPLETE, "start-date", HTTPHandle->bootedTime);
    XK_ClientAddBody_s(TEMPLETE, "time", curDate);
    XK_ClientAddBody_s(TEMPLETE, "eth0", HTTPHandle->IPAddr_e);
    XK_ClientAddBody_s(TEMPLETE, "wlan0", HTTPHandle->IPAddr_w);
    XK_ClientAddBody_i(TEMPLETE, "free-mem", HTTPHandle->memHeap);
    XK_ClientAddBody_i(TEMPLETE, "cpu-usage", HTTPHandle->cpuUsage);
    XK_ClientAddBody_f(TEMPLETE, "cpu-temp", HTTPHandle->piTemperature);
    XK_ClientAddBody_s(TEMPLETE, "eth0-MAC", HTTPHandle->MAC_e_Addr);
    XK_ClientAddBody_s(TEMPLETE, "wlan0-MAC", HTTPHandle->MAC_w_Addr);
    XK_ClientAddBody_s(TEMPLETE, "version", SDK_VERSION);
    {
        unsigned int appNum[MAX_NUM_USB_DEVICE];
        for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
#if(FALL_ENABLE || INOUT_ENABLE)
            if (HTTPHandle->USBStatus[i] == 0 
                && (
    #if USE_LEGACY_APPNUM
                    HTTPHandle->appNum[i] != 8/*RADAR_APP_WMFALL*/ &&
                    HTTPHandle->appNum[i] != 11 /*RADAR_APP_INOUT*/ &&
    #endif
                    HTTPHandle->appNum[i] != FallDetection_WallMount &&
                    HTTPHandle->appNum[i] != IN_OUT
                )
            ){
#else
            if (HTTPHandle->USBStatus[i] == 0){
#endif
                appNum[i] = HTTPHandle->appNum[i];
                
                idxApp = GetMetaInfo(HTTPHandle->serialNum, HTTPHandle->rcvdRadarData[i], appNum[i]);

                if (HTTPHandle->rcvDataSize[i] > 0 && HTTPHandle->cntNodata[i] == 0 && idxApp != -1){
                    if(gTypeRID=='r'){
                        sprintf(tmp, "r%d", HTTPHandle->radarID[i]);
                        XK_ClientAddBody_g(TEMPLETE, tmp);
                        // GetSerialNumber(HTTPHandle->serialNum, HTTPHandle->rcvdRadarData[i], appNum[i]);
                    }
                    else
                    {
                        // GetSerialNumber(HTTPHandle->serialNum, HTTPHandle->rcvdRadarData[i], appNum[i]);
                        XK_ClientAddBody_g(TEMPLETE, HTTPHandle->serialNum);
                    }
                    LOG_D("Debug", "%d       %d", HTTPHandle->rcvDataSize[i], HTTPHandle->cntNodata[i]);

                    XK_ClientAddBody_i(TEMPLETE, "port", i);
                    XK_ClientAddBody_s(TEMPLETE, "status", USB_DEVICE_STATUS_NORMAL);
                    XK_ClientAddBody_i(TEMPLETE, "application", gAppInfo[idxApp].appNum);
                    // XK_ClientAddBody_s(TEMPLETE, "application", gAppInfo[idxApp].appName);
                
                    if(gTypeData=='m'){
                        {
                            LOG_D("Debug", "%d       %d", HTTPHandle->rcvDataSize[i], HTTPHandle->cntNodata[i]);

                            for (j = 0; j < gAppInfo[idxApp].appParamSZ; j++){
                                XK_ClientAddBody_f(
                                    TEMPLETE, 
                                    gAppInfo[idxApp].param[j].paramName, 
                                    HTTPHandle->rcvdRadarData[i][gAppInfo[idxApp].param[j].paramObjNum_V]);  
                            }
                        }
                    }
                    else{
                        XK_ClientAddBody_i(TEMPLETE, "radarID", HTTPHandle->radarID[i]);
                        XK_ClientAddBody_s(TEMPLETE, "serial", HTTPHandle->serialNum);

                        sprintf(tmp, "%d-%d@1-1", infoUSB[i].busnum, infoUSB[i].devnum);
                        for (j = 1; j <= infoUSB[i].hubN; j++){
                            sprintf(&tmp[strlen(tmp)], ".%d", infoUSB[i].hubNum[j]);
                        }
                        XK_ClientAddBody_s(TEMPLETE, "usb-info", tmp);
                        
                        for (j = 0; j < 200; j++){
                            if(appNum[i]>=10){
                                if(HTTP_send_switch[appNum[i]/10][j] == -1) break;
                                else switchCmpTmp[HTTP_send_switch[appNum[i]/10][j]] = 1;
                            }
                            else{   //do del!!
                                if(HTTP_send_switch[appNum[i]][j] == -1) break;
                                else switchCmpTmp[HTTP_send_switch[appNum[i]][j]] = 1;
                            }
                        }
                        for (j = PROTOCOL_INFO_MAX_SZ; j < HTTPHandle->rcvDataSize[i]; j++){
                            if (switchCmpTmp[j - PROTOCOL_INFO_MAX_SZ] ){
                                if(gTypeData=='a'){
                                    sprintf(tmp, "a%d", j - 1);
                                    XK_ClientAddBody_f(TEMPLETE, tmp, HTTPHandle->rcvdRadarData[i][j]);  
                                }
                                else {
                                    sprintf(tmp, "v%d", j - PROTOCOL_INFO_MAX_SZ);
                                    XK_ClientAddBody_f(TEMPLETE, tmp, HTTPHandle->rcvdRadarData[i][j]);  
                                }
                            }
                        }


                    }
                }
                else{
                    if(gTypeRID=='r'){
                        sprintf(tmp, "r%d", 999999999999);
                        XK_ClientAddBody_g(TEMPLETE, tmp);
                    }
                    else
                    {
                        XK_ClientAddBody_g(TEMPLETE, "999999999999");
                    }
                    XK_ClientAddBody_i(TEMPLETE, "port", i);
                    
                    if(idxApp == -1)
                        XK_ClientAddBody_s(TEMPLETE, "status", USB_DEVICE_STATUS_NOAPPNUM);
                    else    
                        XK_ClientAddBody_s(TEMPLETE, "status", USB_DEVICE_STATUS_NODATA);


                    sprintf(tmp, "%d-%d@1-1", infoUSB[i].busnum, infoUSB[i].devnum);
                    for (j = 1; j <= infoUSB[i].hubN; j++){
                        sprintf(&tmp[strlen(tmp)], ".%d", infoUSB[i].hubNum[j]);
                    }
                    XK_ClientAddBody_s(TEMPLETE, "usb-info", tmp);
                }

                XK_ClientAddBracket(TEMPLETE, "}"); 
                HTTPHandle->rcvDataSize[i] = 0;
            }
        } 
    }
#if ENABLE_SUB_HTTP
    addJsonMultiRadarData(HTTPHandle,TEMPLETE);
#endif
    XK_ClientAddBracket(TEMPLETE, "}");
    TEMPLETE[strlen(TEMPLETE)] = '\0';
    sprintf(output, TEMPLETE);
}

//start server
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;
	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
}

extern char *ROOT;
//client connection
int respond(XK_HTTPHandle_t *HTTPHandle, int clients)
{
    int tmpres;
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

    char query[1024 * 20]={0,};
    char body[1024 * 20]={0,};

    int bodyIndex = 0;
    int srchStartIdx = 0;
    int accumRecvCnt = 0;
    int cntBrktIn = 0;
    int cntBrktOut = 0;
    unsigned char flgTimeOut = 0;
    int i;
    
    JSON_Array *array;

	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(clients, mesg, 99999, 0);

	if (rcvd<0){    // receive error
		LOG_E("respond","recv() error");
        close(clients);
        return 0;
    }
	else if (rcvd==0){    // receive socket closed
		LOG_E("respond","Client disconnected upexpectedly.\n");        
        close(clients);
        return 0;
    }
	else    // message received
	{
        if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_RECV)) printf("Received massage:\n%s\n\n", mesg);

		reqline[0] = strtok (mesg, " \t\n");
        /*API-GET
        * item-get
        */
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{            
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
            LOG_S("GET ", "%s", reqline[1]);
            
            //[GET]: item-get
            if ( strncmp( reqline[1], PAGE_ITEM_GET, sizeof(PAGE_ITEM_GET))==0){
                if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
                {
                    write(clients, "HTTP/1.1 400 Bad Request\n", 25);
                }
                else
                {
                    build_get_resp_query(HTTPHandle, query, body);
                    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_SEND)) printf("\n\r\n\rQuery is:\n<<START>>\n%s\n<<END>>\n", query);

                    //Send the query to the server
                    unsigned int sent = 0;
                    while (sent < strlen(query))
                    {
                        tmpres = send(clients, query + sent, strlen(query) - sent, 0);
                        
                        if (tmpres == -1)
                        {
                            LOG_E("HTTP", "Can't send query");
                            close(clients);
                            return 0;
                        }
                        else
                        {

                        }
                        sent += tmpres;
                    }
                }
            }
		}
        
        /*API-POST
        * radar-cmd
        */
        else if ( strncmp(reqline[0], "POST\0", 5)==0 ){
            
            reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
            LOG_S("POST", "%s", reqline[1]);
            
            //[POST]: radar-cmd
            if ( strncmp( reqline[1], PAGE_RADAR_CMD, sizeof(PAGE_RADAR_CMD))==0){
                if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
                {
                    write(clients, "HTTP/1.1 400 Bad Request\n", 25);
                }
                else
                {
                    for (i = 0; i < rcvd; i++)
                    {
                        if (mesg[i] == '{')
                        {
                            bodyIndex = i;
                            break;
                        }
                    }
                    if(bodyIndex==0){
                        LOG_E("server", "No body");
                        return 0;
                    }
                    
                    serverGetVal = json_parse_string(&mesg[bodyIndex]);
                    serverGetObject = json_value_get_object(serverGetVal);

                    array = json_object_get_array(serverGetObject, "param");
                    CommandParser(HTTPHandle, array);
                    json_value_free(serverGetVal);
                    send(clients, "HTTP/1.1 200 OK\n\n", 17, 0);
                }
            }
        }        
        close(clients);
	}
}


void GetSerialNumber(char *outData, float *radarData, unsigned int appNum){
    switch(appNum){
        case Zone_PeopleCounting:
            sprintf(outData, "%06d%06d", (int)radarData[ZONE_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[ZONE_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        case PERS:
            sprintf(outData, "%06d%06d", (int)radarData[PERS_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[PERS_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        // case RADAR_APP_FALL:
        //     sprintf(outData, "%06d%06d", (int)radarData[FALL_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[FALL_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        // break;
        case Presence_Detection:
            sprintf(outData, "%06d%06d", (int)radarData[PRESENCE_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[PRESENCE_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        // case RADAR_APP_OSR:
        //     sprintf(outData, "%06d%06d", (int)radarData[OSR_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[OSR_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        // break;
        case Foot_Traffic:
            sprintf(outData, "%06d%06d", (int)radarData[FOOT_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[FOOT_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        // case RADAR_APP_WMFALL:
        case FallDetection_WallMount:
            sprintf(outData, "%06d%06d", (int)radarData[WMFALL_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[WMFALL_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        case IN_OUT:
            sprintf(outData, "%06d%06d", (int)radarData[INOUT_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[INOUT_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        case RHRBR:
            sprintf(outData, "%06d%06d", (int)radarData[RHRBR_IDX_SERIAL_NUMBER1 + PROTOCOL_INFO_MAX_SZ], (int)radarData[RHRBR_IDX_SERIAL_NUMBER2 + PROTOCOL_INFO_MAX_SZ]); 
        break;
        case OTHER_DEVICE:
        break;
        default:
            sprintf(outData, "%06d%06d", 999999, 999999); 
            // LOG_E("GetSerialNumber","Invalid App Number!!!!!!!");
        break;
    }
}

int GetMetaInfo(char *outData, float *radarData, unsigned int appNum){
    int i;
    int idxApp = -1;
    for(i=0;i<APP_SIZE;i++){
        if(gAppInfo[i].appNum == appNum){
            idxApp = i;
            sprintf(outData, "%06d%06d", 
                (int)radarData[gAppInfo[i].appSerialNumIdx + PROTOCOL_INFO_MAX_SZ], 
                (int)radarData[gAppInfo[i].appSerialNumIdx+1 + PROTOCOL_INFO_MAX_SZ]
            ); 
        }
    }
    if(idxApp == -1){    
        sprintf(outData, "%06d%06d", 
            999999, 
            999999
        );
    }
    return idxApp;
}





char SendRadarData_To_GCP(XK_HTTPHandle_t *HTTPHandle, int portNum)
{
    int sock2;
    char query2[1024 * 20]={0,};
    char body2[1024 * 20]={0,};
    char TEMPLETE2[1024 * 20] = {0};

    char *get;
    int tmpres;
    int i, j;

    char res;
    char *ip;
    char buf[BUFSIZ + 1];

    int recvCnt = 0;
    int bodyIndex = 0;
    int srchStartIdx = 0;
    int accumRecvCnt = 0;
    int cntBrktIn = 0;
    int cntBrktOut = 0;
    clock_t waitTime = clock();
    unsigned long waitGETTime = 0;
    unsigned char flgTimeOut = 0;
    int cntParam = 0;
    JSON_Array *array;
    char request[GET_SIZE];
    
    char ip_addr2[15];
    
    struct ifreq ifr_e;
    char localIP_e[50];
    struct ifreq ifr_w;
    char localIP_w[50];
    int flgOneTime_=0;
    

    char *host2;
    char *page2;
    int port2;

    host2 = HTTPHandle->info.host;
    page2 = HTTPHandle->info.page;
    port2 = HTTPHandle->info.port;

    BIO *outbio = NULL;
    SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;

    if (port2 == 443)
    {
        OpenSSL_add_all_algorithms();
        ERR_load_BIO_strings();
        ERR_load_crypto_strings();
        SSL_load_error_strings();

        outbio = BIO_new(BIO_s_file());
        outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

        if (SSL_library_init() < 0)
        {
            BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");
        }

        method = SSLv23_client_method();
        ctx = SSL_CTX_new(method);
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    }

    struct sockaddr_in server_addr;
    sock2 = socket(PF_INET, SOCK_STREAM, 0); //try AF_INET
    if (-1 == sock2)
    {
        printf("Fail to socket create\n");
        return 0;
    }

    memcpy(ifr_e.ifr_name, "eth0", IFNAMSIZ-1);ioctl(sock2, SIOCGIFADDR, &ifr_e);
    strcpy(localIP_e, inet_ntoa(((struct sockaddr_in *)&ifr_e.ifr_addr)->sin_addr));
    HTTPHandle->IPAddr_e = localIP_e;

    memcpy(ifr_w.ifr_name, "wlan0", IFNAMSIZ-1);ioctl(sock2, SIOCGIFADDR, &ifr_w);
    strcpy(localIP_w, inet_ntoa(((struct sockaddr_in *)&ifr_w.ifr_addr)->sin_addr));
    HTTPHandle->IPAddr_w = localIP_w;

    struct timeval optVal = {5, 000000};
    struct timeval optSndVal = {2, 000000};
    if (setsockopt(sock2, SOL_SOCKET, SO_RCVTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        LOG_E("HTTP", "fail set receive sockopt()");
        close(sock2);
        return 0;
    }
    else
    {
    }

    if (setsockopt(sock2, SOL_SOCKET, SO_SNDTIMEO, &optSndVal, sizeof(optSndVal)) != 0)
    {
        LOG_E("HTTP", "fail set send sockopt()");
        close(sock2);
        return 0;
    }
    else
    {
    }


#if ONOFF_SUB_EPOINT
    if (HTTPHandle->info.host_type == XK_HOST_TYPE_DOMAIN)
#else
    if (flgOneTime_ == 0)
#endif
    {
        res = get_ip(host2, ip_addr2);
        flgOneTime_ = 1;
        if (res == 0)
        {
            LOG_E("HTTP", "Can't get ip address");
            close(sock2);

            flgOneTime_ = 0;
            return 0;
        }
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port2);
    if(HTTPHandle->info.host_type == XK_HOST_TYPE_DOMAIN) server_addr.sin_addr.s_addr = inet_addr(ip_addr2);
    else server_addr.sin_addr.s_addr = inet_addr(host2);

    if (connect(sock2, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG_E("HTTP", "Can't connect");
        close(sock2);
        return 0;
    }

    if (port2 == 443)
    {
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock2);
        res = SSL_connect(ssl);
        if (res == 0)
        {
            LOG_E("HTTPS", "Could not connect");
            close(sock2);
            return 0;
        }
    }

#if 1 //POST
    // build_get_query(HTTPHandle, host2, page2);
    char *getpage = page2;
    int len;
    char tmp[10];
    char tpl[1024];
    char serial_number[50];
    char basicAuth[100];
    sprintf(serial_number, "%s/%s", HTTPHandle->info.hardware, HTTPHandle->info.serial);
    query2[0] = 0;    
    if (getpage[0] == '/')
    {
        getpage = getpage + 1;
        LOG_E("get page2", "Removing leading \"/\", converting %s to %s\n", page2, getpage);
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetData(HTTPHandle, body2);
    int k;
    int loopj, loopi;
    char tmp3[50];
    char curDate[100];
    unsigned char dataLength = 0;
    int switchCmpTmp[200]={0, };
    int idxApp=-1;
    
    brktLevel = 0;
    objCnt = 0;
    memset(groupCnt, 0, 100*4);
    TEMPLETE2[0] = 0;
    
    XK_ClientAddBracket(TEMPLETE2, "{");
    {
        unsigned int appNum[MAX_NUM_USB_DEVICE];
        // for (i = 0; i < MAX_NUM_USB_DEVICE; i++){
        if (HTTPHandle->USBStatus[portNum] == 0){
            appNum[portNum] = HTTPHandle->appNum[portNum];
            idxApp = GetMetaInfo(HTTPHandle->serialNum, HTTPHandle->rcvdRadarData[portNum], appNum[portNum]);

            if (HTTPHandle->rcvDataSize[portNum] > 0 && HTTPHandle->cntNodata[portNum] == 0 && idxApp != -1){
                
                XK_ClientAddBody_s(TEMPLETE2, "serial", HTTPHandle->serialNum);
                XK_ClientAddBody_i(TEMPLETE2, "port", i);
                XK_ClientAddBody_s(TEMPLETE2, "status", USB_DEVICE_STATUS_NORMAL);
                XK_ClientAddBody_i(TEMPLETE2, "application", gAppInfo[idxApp].appNum);
                if(gTypeData=='m'){
                    {
                        LOG_D("Debug", "%d       %d", HTTPHandle->rcvDataSize[portNum], HTTPHandle->cntNodata[portNum]);

                        for (j = 0; j < gAppInfo[idxApp].appParamSZ; j++){
                            XK_ClientAddBody_f(
                                TEMPLETE2, 
                                gAppInfo[idxApp].param[j].paramName, 
                                HTTPHandle->rcvdRadarData[portNum][gAppInfo[idxApp].param[j].paramObjNum_V]);  
                        }
                    }
                }
            }
            else{
                if(gTypeRID=='r'){
                    sprintf(tmp3, "r%d", 999999999999);
                    XK_ClientAddBody_g(TEMPLETE2, tmp3);
                }
                else
                {
                    XK_ClientAddBody_g(TEMPLETE2, "999999999999");
                }
                XK_ClientAddBody_i(TEMPLETE2, "port2", i);
                
                if(idxApp == -1)
                    XK_ClientAddBody_s(TEMPLETE2, "status", USB_DEVICE_STATUS_NOAPPNUM);
                else
                    XK_ClientAddBody_s(TEMPLETE2, "status", USB_DEVICE_STATUS_NODATA);

                sprintf(tmp3, "%d-%d@1-1", infoUSB[portNum].busnum, infoUSB[portNum].devnum);
                for (j = 1; j <= infoUSB[portNum].hubN; j++){
                    sprintf(&tmp3[strlen(tmp3)], ".%d", infoUSB[portNum].hubNum[j]);
                }
                XK_ClientAddBody_s(TEMPLETE2, "usb-info", tmp3);
            }

            XK_ClientAddBracket(TEMPLETE2, "}"); 
            HTTPHandle->rcvDataSize[portNum] = 0;
        }
        // } 
    }
// #if ENABLE_SUB_HTTP
//     addJsonMultiRadarData(HTTPHandle,TEMPLETE2);
// #endif
    // XK_ClientAddBracket(TEMPLETE2, "}");
    TEMPLETE2[strlen(TEMPLETE2)] = '\0';
    sprintf(body2, TEMPLETE2);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    int LengthofBody = strlen(body2);

    XK_HTTP_ClientSetHeader_POST(tpl, getpage, "HTTP/1.1");
    XK_HTTP_ClientSetHeader(tpl, "host", host2);
    XK_HTTP_ClientSetHeader(tpl, "Connection", Connection);
    XK_HTTP_ClientSetHeader(tpl, "Content-Type", CONTENTTYPE);
    // sprintf(basicAuth, "Basic %s",HTTPHandle->info.auth);
    XK_HTTP_ClientSetHeader(tpl, "Authorization", "Bearer");
    // XK_HTTP_ClientSetHeader(tpl, "accountId", HTTPHandle->info.accID);
    // XK_HTTP_ClientSetHeader(tpl, "x-access-serial", serial_number);
    len = LengthofBody;
    sprintf(tmp, "%d", len);
    XK_HTTP_ClientSetHeader(tpl, "Content-Length", tmp);
    XK_HTTP_ClientSetHeaderClose(tpl);

    query2[len] = 0;
    strcat(query2, tpl);
    strcat(query2, body2);


    if (gFlgSysLog && HTTPHandle->flgParamSizeErr == 1)
    {
        LOG_E("system", "\n->\n<<START>>\n%s\n<<END>>\n\n\n", query2);
    }

    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_SEND))
    {
        printf("\n\r\n\rquery2 is:\n<<START>>\n%s\n<<END>>\n", query2);
    }

    //Send the query2 to the server
    unsigned int sent = 0;
    while (sent < strlen(query2))
    {

        if (port2 == 443)
        {
            tmpres = SSL_write(ssl, query2 + sent, strlen(query2) - sent);
        }
        else
        {
            tmpres = send(sock2, query2 + sent, strlen(query2) - sent, 0);
        }
        if (tmpres == -1)
        {
            LOG_E("HTTP", "Can't send query2");
            close(sock2);
            return 0;
        }
        else
        {
            // printf("Succeed to send\n");
        }
        sent += tmpres;
    }

    recvCnt = 0;
    accumRecvCnt = 0;
    cntBrktIn = 0;
    cntBrktOut = 0;
    flgTimeOut = 0;
    waitGETTime = clock();
    bzero(request, GET_SIZE);

    // while (clock() - waitGETTime < (WAIT_GET_TIME_OUT * CLOCKS_PER_SEC))
    // {
    //     if (port2 == 443)
    //     {
    //         recvCnt = SSL_read(ssl, &request[accumRecvCnt], GET_SIZE - 1);
    //     }
    //     else
    //     {
    //         recvCnt = recv(sock2, &request[accumRecvCnt], GET_SIZE - 1, MSG_NOSIGNAL);
    //     }
    //     if (recvCnt == -1)
    //     {
    //         LOG_E("HTTP", "Receive Error ");
    //         close(sock2);
    //         return 0;
    //     }

    //     accumRecvCnt += recvCnt;
    //     for (i = srchStartIdx; i < accumRecvCnt; i++)
    //     {
    //         if (request[portNum] == '\n' && request[i + 1] == '\r')
    //         {
    //             bodyIndex = i + 2;
    //         }
    //         if (request[portNum] == '{')
    //             cntBrktIn++;
    //         if (request[portNum] == '}')
    //             cntBrktOut++;
    //     }
    //     if (cntBrktIn == cntBrktOut && cntBrktIn != 0)
    //     {
    //         flgTimeOut = 1;
    //         break;
    //     }
    //     srchStartIdx = accumRecvCnt;
    // }

    // if (flgTimeOut)
    // {
    //     for (i = 0; i < accumRecvCnt; i++)
    //     {
    //         if (request[portNum] == '{')
    //         {
    //             bodyIndex = i;
    //             break;
    //         }
    //     }
    // }
    // else
    // {
    //     LOG_E("HTTP Receive", "Time out...");
    //     close(sock2);
    //     return 0;
    // }

    // serverGetVal = json_parse_string(&request[bodyIndex]);
    // serverGetObject = json_value_get_object(serverGetVal);

    // cntParam = 0;

    // array = json_object_get_array(serverGetObject, "param");
    // CommandParser(HTTPHandle, array);

    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_RECV))
    {
        printf("\n\r%s\n\r", request);
    }

    // json_value_free(serverGetVal);
#endif
    close(sock2);

    if (port2 == 443)
    {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    return 1;
}
