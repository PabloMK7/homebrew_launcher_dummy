#include <3ds/types.h>
#include <malloc.h>    // for mallinfo() 
#include <unistd.h>    // for sbrk() 

extern u8 *fake_heap_end;   // current heap start 
extern u8 *fake_heap_start;   // current heap end 

u8  *getHeapStart(void)
{
    return fake_heap_start;
}

u8  *getHeapEnd(void)
{
    return (u8*)sbrk(0);
}

u8  *getHeapLimit(void)
{
    return fake_heap_end;
}

// returns the amount of used memory in bytes 
int getMemUsed(void)
{ 
    struct mallinfo mi = mallinfo();
    return mi.uordblks;
}

// returns the amount of free memory in bytes 
int getMemFree(void)
{ 
    struct mallinfo mi = mallinfo();
    return mi.fordblks + (getHeapLimit() - getHeapEnd());
}
