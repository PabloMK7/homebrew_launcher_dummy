#include "clock.h"

#define CPU_TICKS_PER_MSEC (SYSCLOCK_ARM11 / 1000.0)
#define CPU_TICKS_PER_USEC (SYSCLOCK_ARM11 / 1000000.0)


u64     Timer_Restart(void)
{
	return (svcGetSystemTick());
}
u64		getTimeInMsec(u64 timer) {
	return timer / CPU_TICKS_PER_MSEC;
}
bool    Timer_HasTimePassed(float nbmsecToWait, u64 timer)
{
	u64 seconds = (u64)(nbmsecToWait * CPU_TICKS_PER_MSEC);
	u64 current = svcGetSystemTick();
	u64 diff = current - timer;

	return (diff >= seconds);
}