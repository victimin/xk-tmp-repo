#ifndef __XK_TOOLS_H__
#define __XK_TOOLS_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
   P_INFO = 0,
   P_WARRING,
   P_ERROR

} XK_PRINT_TYPE;

void XK_SystemPrint(char *str, unsigned char type);
void LOG_E (char *tag, char *str,...);
void LOG_I (char *tag, char *str,...);
void LOG_W (char *tag, char *str,...);
void LOG_N (char *tag, char *str,...);
void LOG_S (char *tag, char *str,...);
void LOG_system (char *type, char *tag, char *msg);
int GetRamInKB(void);
int GetCPUSerial(char *serial_string, char *HW_string);
double get_temp(int use_farenheit);

void XK_GetTeamviewerID(char *id, char *path);
void XK_GetTeamviewerVer(char *ver, char *path);

void XK_ClientAddBody_s(char * data, char * object, char * contents);
void XK_ClientAddBody_i(char * data, char * object, int contents);
void XK_ClientAddBody_f(char * data, char * object, float contents);
void XK_ClientAddBracket(char * data, char * bracket);
void XK_ClientAddIndent(char * data);
void XK_ClientAddBody_g(char * data, char * object);

void XK_HTTP_ClientSetHeader_API(char * data, char * page, char * version);
void XK_HTTP_ClientSetHeader(char * data, char * object, char * contents);
void XK_HTTP_ClientSetHeaderClose(char * data);

void CPU_Tick(long double *a);
long double CPU_Tock(long double *a, long double *b);
void XK_GetMAC(char *mac, char *path);

void XK_PrintInfo_s(char *object, char *content);
void XK_PrintInfo_i(char *object, int content);
void XK_PrintInfo_c(char *object, int content);

int GetProcessNameByPid(int pid, char* name);
   
int GetMetaInfo(char *outData, float *radarData, unsigned int appNum);

#ifdef __cplusplus
}
#endif


#endif

