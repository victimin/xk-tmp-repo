#include<stdio.h>
#include<wiringPi.h>

#define HEARTBEATS_OUT_PIN      4

int main()
{
	int i;
	if(wiringPiSetup()==-1) return -1;
	
	pinMode(HEARTBEATS_OUT_PIN,OUTPUT);
	while(1)
	{
		digitalWrite(HEARTBEATS_OUT_PIN,1);
		delay(500);
		digitalWrite(HEARTBEATS_OUT_PIN,0);
		delay(500);
	}
	return 0;
}



