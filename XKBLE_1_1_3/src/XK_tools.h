#ifndef __XK_TOOLS_H__
#define __XK_TOOLS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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
int GetComplements(char data);
void XK_ClientAddBody_iBcon(char * data, char * object, int devNum);
void delChar( char *buf, char *dest, char ch );
   

#ifdef __cplusplus
}
#endif


#endif

