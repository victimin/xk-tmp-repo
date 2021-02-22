#include <stdio.h>
#include <string.h>

int main(void)
{
	system("sudo rm -r /forcefsck");
	system("sudo reboot");

    return 0;
}
