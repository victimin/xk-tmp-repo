#ifndef __XK_LED_LIGHTING__H__
#define __INIT_H__

#ifdef __XK_LED_LIGHTING__H__
extern "C"
{
#endif

#define LED_LIGHT_NUM           2
#define MAX_DEV_NUM             20

char LED_SerialNumber[MAX_DEV_NUM][20];
char LED_Status[MAX_DEV_NUM];

#define LEDLIGHT_RED_PIN        26
#define LEDLIGHT_YELLOW_PIN     20
#define LEDLIGHT_GREEN_PIN      21







#ifdef __cplusplus
}
#endif


#endif

