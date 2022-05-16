/*! System call - call to kernel from threads (via software interrupt) */
#pragma once

#include <kernel/syscall.h>
#include <types/basic.h>

void k_syscall(uint irqn);
