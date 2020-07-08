#include "XK_LED_lighting.h"

led_info_t gLEDInfo = {
	.usingAppNum = 30,
	.numOfGroup = MAX_GROUP_NUM,
	.group[0] = {
		.numOfSensor = 2,
		// .medianCap = 3,
		.maxCap = 7,
        .ledRedPin = LEDLIGHT_G0_RED_PIN,
        .ledYellowPin = LEDLIGHT_G0_YELLOW_PIN,
        .ledGreenPin = LEDLIGHT_G0_GREEN_PIN,
		.usingSerial = {
			"123456789001",
			"123456789002"
		}
	},
	.group[1] = {
		.numOfSensor = 1,
		// .medianCap = 3,
		.maxCap = 5,
        .ledRedPin = LEDLIGHT_G1_RED_PIN,
        .ledYellowPin = LEDLIGHT_G1_YELLOW_PIN,
        .ledGreenPin = LEDLIGHT_G1_GREEN_PIN,
		.usingSerial = {
			"123456000310"
		}
	}
};



char LED_SerialNumber[20][20]={
	{"123456000310"},
	{"222222222222"},
};


