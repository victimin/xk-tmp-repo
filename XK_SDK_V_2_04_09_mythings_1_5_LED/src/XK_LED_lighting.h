#ifndef __XK_LED_LIGHTING__H__
#define __INIT_H__

#ifdef __XK_LED_LIGHTING__H__
extern "C"
{
#endif

#define LED_LIGHT_NUM           1
#define LED_COLOR_NUM           3
#define MAX_DEV_NUM             20
#define MAX_GROUP_NUM           2

#define LIGHT_RED_THR           3
#define LIGHT_YELLOW_THR        2
#define LIGHT_GREEN_THR         1

char LED_SerialNumber[MAX_DEV_NUM][20];
char LED_Status[MAX_GROUP_NUM][3];

typedef enum
{
    COLOR_IDX_RED = 0,
    COLOR_IDX_YELLOW,
    COLOR_IDX_GREEN,
} COLOR_IDX;

typedef enum
{
    LED_ON = 0,
    LED_OFF = 1,
} LED_ONOFF;

#define LEDLIGHT_G0_RED_PIN        16
#define LEDLIGHT_G0_YELLOW_PIN     20
#define LEDLIGHT_G0_GREEN_PIN      21

#define LEDLIGHT_G1_RED_PIN        6
#define LEDLIGHT_G1_YELLOW_PIN     13
#define LEDLIGHT_G1_GREEN_PIN      19

#define LEDLIGHT_N0_PIN            5
#define LEDLIGHT_N1_PIN            26


typedef struct {
    int numOfSensor;   
    int medianCap;
    int maxCap;
    int ledRedPin;
    int ledYellowPin;
    int ledGreenPin;
    char usingSerial[10][20];
} led_serial_info_t;

typedef struct {
    int usingAppNum;
    int numOfGroup;
    led_serial_info_t group[MAX_GROUP_NUM];
} led_info_t;

led_info_t gLEDInfo;


#ifdef __cplusplus
}
#endif


#endif

