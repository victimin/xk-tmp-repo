#include "XK_LED_lighting.h"

led_info_t gLEDInfo = {
	.group[0] = {
		.maxCap = 7,
        .ledRedPin = LEDLIGHT_G0_RED_PIN,
        .ledYellowPin = LEDLIGHT_G0_YELLOW_PIN,
        .ledGreenPin = LEDLIGHT_G0_GREEN_PIN,
        .ledBuzzerPin = LEDLIGHT_G0_YELLOW_PIN,
		.usingSerial = {
			"100218001030",
			"100306000219",
			"100302000246",
			"100302000220",
		}
	}
};


