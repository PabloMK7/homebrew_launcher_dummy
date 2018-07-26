#ifndef NEWLIBHEAP_H
#define NEWLIBHEAP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "3ds/types.h"

u8      *getHeapStart(void);
u8      *getHeapEnd(void);
u8      *getHeapLimit(void);
int     getMemUsed(void);
int     getMemFree(void);

#ifdef __cplusplus
}
#endif
#endif
