/*! Kernel features that can dynamically be turned on/off */

#include <types/basic.h>

#define FEATURE_GET		0
#define FEATURE_SET		1

#define FEATURE_INTERRUPTS	(1 << 0)

#define FEATURE_SUPPORTED	FEATURE_INTERRUPTS

uint sys__feature(uint features, int cmd, int enable);

#define OS_ENABLE(FEATURE)	sys__feature(FEATURE, FEATURE_SET, TRUE)
#define OS_DISABLE(FEATURE)	sys__feature(FEATURE, FEATURE_SET, FALSE)
