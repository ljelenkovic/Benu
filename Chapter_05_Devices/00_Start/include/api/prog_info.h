/*! Program info */
#pragma once

#include <types/basic.h>

/* "programs" */
int hello_world();
int timer();
int segm_fault();

#define	hello_world_PROG_HELP	"Print 'Hello world'."
#define	timer_PROG_HELP		"Timer interface demonstration: "	\
				"periodic timer activations."
#define	segm_fault_PROG_HELP	"Generate segmentation fault."
