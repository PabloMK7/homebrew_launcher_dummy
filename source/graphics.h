#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "draw.h"
#include "main.h"

#define STACKSIZE 0x1000

void    initUI(void);
void    exitUI(void);
int     updateUI(void);
void	setExitMode();
void handleFadeIn(s64 time, u64 tot);


#define newAppTop(...) newAppInfoEntry(appTop, __VA_ARGS__)
#define removeAppTop() removeAppInfoEntry(appTop)
#define clearTop(update)    clearAppInfo(appTop, update)

#define TRACE() {newAppTop(DEFAULT_COLOR, SMALL, "%s:%d",__FUNCTION__, __LINE__); svcSleepThread(1000000000); updateUI(); svcSleepThread(1000000000);}
#define XTRACE(str, ...) {newAppTop(DEFAULT_COLOR, SMALL, str, __VA_ARGS__); updateUI(); svcSleepThread(500000000);}  

#endif