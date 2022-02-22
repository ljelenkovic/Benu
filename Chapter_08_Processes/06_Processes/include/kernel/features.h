/*! Kernel features that can dynamically be turned on/off */

#include <types/basic.h>

#define FEATURE_GET		0
#define FEATURE_SET		1

#define FEATURE_INTERRUPTS	(1 << 0)
#define FEATURE_TIMERS		(1 << 1)
#define FEATURE_SCHEDULER	(1 << 2)
#define FEATURE_SCHED_RR	(1 << 3)

#define FEATURE_SUPPORTED	(FEATURE_INTERRUPTS | FEATURE_TIMERS   | \
				  FEATURE_SCHEDULER  | FEATURE_SCHED_RR   )
#ifdef _KERNEL_

uint k_feature(uint features, int cmd, int enable);

#define K_ENABLE(FEATURE)	k_feature(FEATURE, FEATURE_SET, TRUE)
#define K_DISABLE(FEATURE)	k_feature(FEATURE, FEATURE_SET, FALSE)

int sys__feature(void *p);

#endif /* _KERNEL_ */