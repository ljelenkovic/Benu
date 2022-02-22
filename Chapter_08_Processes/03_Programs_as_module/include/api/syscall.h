/*! Syscall - call to kernel function through software interrupt */
#pragma once

#include <types/basic.h>
#include <kernel/syscall.h> /* for syscall IDs */

extern int syscall(uint id, ...) __attribute__((noinline));

static inline uint sys_feature(uint features, int cmd, int enable)
{
	return syscall(SYSFEATURE, features, cmd, enable);
}

#define OS_ENABLE(FEATURE)	sys_feature(FEATURE, FEATURE_SET, TRUE)
#define OS_DISABLE(FEATURE)	sys_feature(FEATURE, FEATURE_SET, FALSE)
