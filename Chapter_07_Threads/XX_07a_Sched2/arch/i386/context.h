/*! Processor/thread context */

#pragma once

#include <types/basic.h>
#include <arch/context.h>

#define	INIT_EFLAGS	0x0202 /* ring 0 ! */


/*! context manipulation ---------------------------- */

typedef struct _arch_context_t_
{
	int32    edi, esi, ebp, _esp, ebx, edx, ecx, eax;
	uint32   eflags;
	uint32   eip;
}
__attribute__((__packed__)) arch_context_t;

/*! Thread context */
struct _context_t_
{
	arch_context_t  *context; /* thread context is on stack! */

#ifdef USE_SSE
	uint32          sse_mmx_fpu;
#endif
};
