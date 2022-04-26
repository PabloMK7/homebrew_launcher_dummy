#include "main.h"
#include "draw.h"
#include <time.h>
#include "clock.h"



int main(void)
{
    //u32         kernelVersion;
	gfxInitDefault();
	romfsInit();
	drawInit();
	initUI();
	bool loop = true;
	s64 timenow = (s64)getTimeInMsec(Timer_Restart());
	s64 timefinish = timenow + 500;
	s64 dif;
	while (aptMainLoop() && loop)
	{
		timenow = (s64)getTimeInMsec(Timer_Restart());
		if (timefinish - timenow > 0)
			dif = timefinish - timenow;
		else
			dif = 0;
		
		handleFadeIn(dif, 500);
		

		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) {
			loop = false;
			setExitMode();
		}; // break in order to return to hbmenu
		updateUI();
	}
	svcSleepThread(1000000000);
	drawExit();
	exitUI();
    romfsExit();
    gfxExit();
    return (0);
}
