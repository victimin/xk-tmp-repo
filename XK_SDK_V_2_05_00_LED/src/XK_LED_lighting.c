#include "XK_LED_lighting.h"

led_info_t gLEDInfo = {
	.usingAppNum = 30,
	.numOfGroup = MAX_GROUP_NUM,
	.group[0] = {
		.numOfSensor = 2,
		// .medianCap = 3,
		.maxCap = 10,
        .ledRedPin = LEDLIGHT_G0_RED_PIN,
        .ledYellowPin = LEDLIGHT_G0_YELLOW_PIN,
        .ledGreenPin = LEDLIGHT_G0_GREEN_PIN,
		.usingSerial = {
			"100300000237",
			"100307000238"
		}
	}
};



char LED_SerialNumber[20][20]={
	{"123456000310"},
	{"222222222222"},
};


