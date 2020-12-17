#ifndef __XK_THREAD__
#define __XK_THREAD__

#ifdef __cplusplus
extern "C"
{
#endif

#define THREAD_MAX			(20)


#define TEST_MAC		"E6:85:CA:F8:7A:92"

int XkBleThreadInit(int device);
int XkWatchThreadInit(void);



#ifdef __cplusplus
}
#endif
#endif

