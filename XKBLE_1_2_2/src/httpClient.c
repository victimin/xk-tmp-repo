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
#include "parson.h"
#include "RadarCommand.h"

#include <curl/curl.h>
#include <fcntl.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define GET_SIZE 3000
#define USB_DEVICE_STATUS_NODATA       "no-data"
#define USB_DEVICE_STATUS_NORMAL       "normal"
#define USB_DEVICE_STATUS_NOAPPNUM     "invalid-appnum"

#define HTTP_QUERY_SIZE_MAX             (1024 * 10u)

FILE *fpTmpLog;
FILE *fpOutput;

char PATH_temp[200];

char Data_Path[200];
char Basic_Path[200];
char Output_Path[200];

int brktLevel=0;
int objCnt=0;
int groupCnt[100]={0, };

struct sockaddr_in *remote;
struct sockaddr_in server_addr;

extern ParamQ_Handle_t RadarCMD_Q;
extern char send_A_type;
extern char gTypeData;
extern char gTypeRID;
extern int gFlgSysLog;

int listenfd;

extern struct tm gLocalTime;
// extern XK_USBinfo_t infoUSB[USB_DEV_PORT_MAX_SIZE];

SSL_CTX * XK_InitCtx(void);
SSL * XK_ConnectSSL(SSL_CTX *ctx, int sock);
int XK_ConnectToServer(int sock, char *host, int port, int hostType);
int XK_SetSockOption(int sock);
void XK_GetIPAddr(int sock, char * interface, char * out);
int XK_InitSock(int *sock);
void XK_OptionalPrint(XK_HTTPHandle_t * HTTPHandle, int gFlgSysLog, char *query, char *request);
int XK_HTTPSendMsg(int sock, SSL *ssl, char *query);
int XK_HTTPRecvMsg(int sock, SSL *ssl, char *request);
void XK_HTTPParseMsg(XK_HTTPHandle_t * HTTPHandle, char * request, int bodyIndex);
void XK_CloseSock(int sock, SSL *ssl, SSL_CTX *ctx);

char SendRadarData(XK_HTTPHandle_t * HTTPHandle)
{    
    int i, j;
    int res;

    int sock;
    char *get;
    char *ip;

    char query[HTTP_QUERY_SIZE_MAX];
    char request[GET_SIZE];
    int bodyIndex = 0;

    JSON_Array *array;

    char *host = HTTPHandle->info->host;
    char *page = HTTPHandle->info->page;
    int port = HTTPHandle->info->port;

    //for https
    SSL_CTX *ctx;
    SSL *ssl=NULL;
    
    if (port == 443) ctx = XK_InitCtx();
    res=XK_InitSock(&sock);
    if(res==0){
        LOG_E("XK_InitSock","Fail to create socket\n");
        return 0;
    }
    XK_GetIPAddr(sock, "eth0", HTTPHandle->IPAddr_e);
    XK_GetIPAddr(sock, "wlan0", HTTPHandle->IPAddr_w);
    res=XK_SetSockOption(sock);
    if(res==0){
        LOG_E("XK_SetSockOption", "fail to set sockopt()");
        close(sock);
        return 0;
    }
    res=XK_ConnectToServer(sock, host, port, HTTPHandle->info->host_type);
    if(res==0){
        LOG_E("XK_ConnectToServer", "fail to connect socket");
        close(sock);
        return 0;
    }
    if (port == 443){
        ssl = XK_ConnectSSL(ctx, sock);
        if(ssl==0){
            LOG_E("XK_ConnectSSL", "Could not connect");
            close(sock);
            return 0;
        }
    }


#if 1 //POST
    XK_BuildQuery(HTTPHandle, host, page, query);
    res = XK_HTTPSendMsg(sock, ssl, query);
    if(res==-1){
        LOG_E("XK_HTTPSendMsg", "Could not send");
        close(sock);
        return 0;
    }
    bodyIndex = XK_HTTPRecvMsg(sock, ssl, request);   
    if(bodyIndex==-1){
        XK_OptionalPrint(HTTPHandle, gFlgSysLog, query, request);
        LOG_E("XK_HTTPSendMsg", "Could not receive");
        close(sock);
        return 0;
    }     
    XK_HTTPParseMsg(HTTPHandle, request, bodyIndex);
    XK_OptionalPrint(HTTPHandle, gFlgSysLog, query, request);
#endif
    XK_CloseSock(sock, ssl, ctx);

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
    // LOG_I("get_ip", "End point is %s", ip);
    return 1;
}

SSL_CTX * XK_InitCtx(void){
    BIO *outbio = NULL;
    SSL_METHOD *method;
    SSL_CTX *ctx;
    
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
    return ctx;
}

SSL * XK_ConnectSSL(SSL_CTX *ctx, int sock){
    int res;
    SSL *ssl;

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    res = SSL_connect(ssl);
    
    if (res == 0)
    {
        return 0;
    }

    return ssl;
}

int XK_ConnectToServer(int sock, char *host, int port, int hostType){
    struct sockaddr_in server_addr;
    int res;
    char ip_addr[15];

#if ONOFF_SUB_EPOINT
    if (hostType == XK_HOST_TYPE_DOMAIN)
#else
    // if (*flg1time == 0 && hostType == XK_HOST_TYPE_DOMAIN)
    if (hostType == XK_HOST_TYPE_DOMAIN)
#endif
    {
        res = get_ip(host, ip_addr);
        // *flg1time = 1;
        if (res == 0)
        {
            LOG_E("HTTP", "Can't get ip address");
            // *flg1time = 0;
            return 0;
        }
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(hostType == XK_HOST_TYPE_DOMAIN) server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    else server_addr.sin_addr.s_addr = inet_addr(host);


    unsigned long waitGETTime = clock();

    // while (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    // {            
    //     if(clock() - waitGETTime > (5 * CLOCKS_PER_SEC)){
    //         LOG_E("HTTP", "Can't connect");
    //         return 0;
    //     }
    // }   
    // return 1;

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG_E("HTTP", "Can't connect");
        return 0;
    }
    return 1;

}

int XK_SetSockOption(int sock){
    struct timeval optVal = {5, 000000};
    struct timeval optSndVal = {2, 000000};
    
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        LOG_E("HTTP", "fail set receive sockopt()");
        return 0;
    }
    else
    {
    }

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &optSndVal, sizeof(optSndVal)) != 0)
    {
        LOG_E("HTTP", "fail set send sockopt()");
        return 0;
    }
    else
    {
    }
    return 1;
}

void XK_GetIPAddr(int sock, char * interface, char * out){
    struct ifreq ifr;
    char localIP[50];

    memcpy(ifr.ifr_name, interface, IFNAMSIZ-1); ioctl(sock, SIOCGIFADDR, &ifr);
    strcpy(out, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

int XK_InitSock(int *sock){
    *sock = socket(PF_INET, SOCK_STREAM, 0); //try AF_INET
    // *sock = socket(AF_INET, SOCK_STREAM, 0); //try AF_INET
    if (-1 == sock)
    {
        return 0;
    }
    return 1;
}

void XK_OptionalPrint(XK_HTTPHandle_t * HTTPHandle, int gFlgSysLog, char *query, char *request){
    if (gFlgSysLog && HTTPHandle->flgParamSizeErr == 1)
    {
        LOG_E("system", "\n->\n<<START>>\n%s\n<<END>>\n\n\n", query);
    }

    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_SEND))
    {
        printf("\n\r\n\rQuery is:\n<<START>>\n%s\n<<END>>\n", query);
    }

    if (HTTPHandle->optDisp & (0x1 << HTTP_DISP_OPT_RECV))
    {
        printf("\n\r%s\n\r", request);
    }
}

void XK_CloseSock(int sock, SSL *ssl, SSL_CTX *ctx){
    close(sock);

    if (ssl != NULL)
    {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
}

int XK_HTTPSendMsg(int sock, SSL *ssl, char *query){
    //Send the query to the server
    unsigned int sent = 0;
    int res;

    while (sent < strlen(query))
    {
        // if (port == 443)
        if (ssl != NULL)
        {
            res = SSL_write(ssl, query + sent, strlen(query) - sent);
        }
        else
        {
            res = send(sock, query + sent, strlen(query) - sent, 0);
        }
        if (res == -1)
        {
            LOG_E("HTTP", "Can't send query");
            return -1;
        }
        else
        {
            // printf("Succeed to send\n");
        }
        sent += res;
    }
    return 1;
}

   
int XK_HTTPRecvMsg(int sock, SSL *ssl, char *request){
    // clock_t waitTime = clock();
    int i;
    int recvCnt = 0;
    int bodyIndex = 0;
    int srchStartIdx = 0;
    int accumRecvCnt = 0;
    int cntBrktIn = 0;
    int cntBrktOut = 0;
    unsigned long waitGETTime = clock();
    unsigned char flgTimeOut = 0;

    bzero(request, GET_SIZE);

    while (clock() - waitGETTime < (WAIT_GET_TIME_OUT * CLOCKS_PER_SEC))
    {
        // if (port == 443)
        if (ssl != NULL)
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
            // close(sock);
            return -1;
        }

        
        char * pTMP = strstr(request, "\"ok\"");
        if(pTMP == NULL) return 0;
    

    
        accumRecvCnt += recvCnt;        
        if(accumRecvCnt >= GET_SIZE){
            LOG_E("XK_HTTPRecvMsg", "Not enough resquest buffer");
            // close(sock);
            return -1;
        }
        
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
        // close(sock);
        return -1;
    }
    return bodyIndex;
}

void XK_HTTPParseMsg(XK_HTTPHandle_t * HTTPHandle, char * request, int bodyIndex){
    JSON_Value *serverGetVal;
    JSON_Object *serverGetObject;
    JSON_Array *array;

    serverGetVal = json_parse_string(&request[bodyIndex]);
    serverGetObject = json_value_get_object(serverGetVal);

    array = json_object_get_array(serverGetObject, "param");
    CommandParser(HTTPHandle, array);

    json_value_free(serverGetVal);
}

void XK_BuildQuery(XK_HTTPHandle_t *HTTPHandle, char *host, char *page, char *query)
{
    char body[HTTP_QUERY_SIZE_MAX];
    char *getpage = page;
    int len;
    char tmp[10];
    char tpl[1024];
    char serial_number[50];
    char basicAuth[100];
    sprintf(serial_number, "%s/%s", HTTPHandle->info->hardware, HTTPHandle->info->serial);
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
    sprintf(basicAuth, "Basic %s",HTTPHandle->info->auth);
    XK_HTTP_ClientSetHeader(tpl, "Authorization", basicAuth);
    XK_HTTP_ClientSetHeader(tpl, "accountId", HTTPHandle->info->accID);
    XK_HTTP_ClientSetHeader(tpl, "x-access-serial", serial_number);
    len = LengthofBody;
    sprintf(tmp, "%d", len);
    XK_HTTP_ClientSetHeader(tpl, "Content-Length", tmp);
    XK_HTTP_ClientSetHeaderClose(tpl);

    query[len] = 0;
    strcat(query, tpl);
    strcat(query, body);
}

void build_get_resp_query(XK_HTTPHandle_t *HTTPHandle, char * page, char * query)
{
    char *getpage = page;
    int len;
    char *tpl = HTTP_SERVER_HEADER;
    char body[HTTP_QUERY_SIZE_MAX];
    
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
    char tmp[100];
    char curDate[100];
    unsigned char dataLength = 0;
    int idxApp=-1;
    char TEMPLETE[HTTP_QUERY_SIZE_MAX] = {0};

    sprintf(curDate, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:00", gLocalTime.tm_year + 1900, gLocalTime.tm_mon + 1, gLocalTime.tm_mday, gLocalTime.tm_hour, gLocalTime.tm_min, gLocalTime.tm_sec , (gLocalTime.tm_gmtoff/(60*60) > 0)? '+':'-', (gLocalTime.tm_gmtoff/(60*60) > 0)? gLocalTime.tm_gmtoff/(60*60):(gLocalTime.tm_gmtoff/(60*60))*(-1));
    
    brktLevel = 0;
    objCnt = 0;
    memset(groupCnt, 0, 100*4);
    TEMPLETE[0] = 0;
    
    XK_ClientAddBracket(TEMPLETE, "{");
    XK_ClientAddBody_s(TEMPLETE, "client", HTTPHandle->info->client);
    XK_ClientAddBody_s(TEMPLETE, "device", HTTPHandle->info->device);
    XK_ClientAddBody_s(TEMPLETE, "function", HTTPHandle->info->function);
    XK_ClientAddBody_s(TEMPLETE, "start-date", HTTPHandle->bootedTime);
    XK_ClientAddBody_s(TEMPLETE, "time", curDate);
    XK_ClientAddBody_s(TEMPLETE, "eth0", HTTPHandle->IPAddr_e);
    XK_ClientAddBody_s(TEMPLETE, "wlan0", HTTPHandle->IPAddr_w);
    XK_ClientAddBody_i(TEMPLETE, "free-mem", HTTPHandle->memHeap);
    XK_ClientAddBody_i(TEMPLETE, "cpu-usage", HTTPHandle->cpuUsage);
    XK_ClientAddBody_f(TEMPLETE, "cpu-temp", HTTPHandle->piTemperature);
    XK_ClientAddBody_s(TEMPLETE, "eth0-MAC", HTTPHandle->info->MAC_e_Addr);
    XK_ClientAddBody_s(TEMPLETE, "wlan0-MAC", HTTPHandle->info->MAC_w_Addr);
    XK_ClientAddBody_s(TEMPLETE, "version", XKBLE_VERSION);
    {
        for (i = 0; i < configInfo.radarDeviceNum; i++){

            XK_ClientAddBody_g(TEMPLETE, configInfo.radarDeviceList[i]);
            {
                XK_ClientAddBody_i(TEMPLETE, "RSSI", iBconDate[i].RSSI);
                if(iBconDate[i].connectionStatus == CONN_STATUS_NONE)
                    XK_ClientAddBody_s(TEMPLETE, "data", "none");            
                else if(iBconDate[i].connectionStatus == CONN_STATUS_DISCONNECTED)
                    XK_ClientAddBody_s(TEMPLETE, "data", "disconnected");            
                else
                    XK_ClientAddBody_iBcon(TEMPLETE, "data", i);
            }
            XK_ClientAddBracket(TEMPLETE, "}"); 
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
    JSON_Value *serverGetVal;
    JSON_Object *serverGetObject;
    char *page = HTTPHandle->info->page;

    int tmpres;
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

    int bodyIndex = 0;
    int srchStartIdx = 0;
    int accumRecvCnt = 0;
    int cntBrktIn = 0;
    int cntBrktOut = 0;
    unsigned char flgTimeOut = 0;
    int i;
    char query[HTTP_QUERY_SIZE_MAX];
    
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
                    build_get_resp_query(HTTPHandle, page, query);
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
