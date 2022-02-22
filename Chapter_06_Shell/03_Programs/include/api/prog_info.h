/*! Program info */
#pragma once

#include <types/basic.h>

typedef struct _prog_info_t_
{
	/* defined in compile time */
	void   *init;		/* process initialization function */
	void   *entry;		/* starting user function */
	void   *param;		/* parameter to starting function */

	/* (re)defined in run time */
	void   *heap;
	size_t  heap_size;

	void   *mpool;
}
prog_info_t;

void prog_init(void *args);

/* "programs" */
int hello_world(char *args[]);
int timer(char *args[]);
int keyboard(char *args[]);
int shell(char *argv[]);
int arguments(char *argv[]);
int segm_fault(char *argv[]);
int run_all(char *argv[]);

#define	hello_world_PROG_HELP	"Print 'Hello world'."
#define	timer_PROG_HELP		"Timer interface demonstration: "	\
				"periodic timer activations."
#define	keyboard_PROG_HELP	"Print ASCII code for each keystroke. "	\
				"Press '.' to end."
#define	shell_PROG_HELP		"Simple command shell"
#define	arguments_PROG_HELP	"Print given command line arguments."
#define	segm_fault_PROG_HELP	"Generate segmentation fault."
#define	run_all_PROG_HELP	"Start selected programs."



#define PROGRAMS_FOR_SHELL					\
{								\
{hello_world,	"hello",	hello_world_PROG_HELP},		\
{timer, 	"timer",	timer_PROG_HELP},		\
{keyboard,	"keyboard",	keyboard_PROG_HELP},		\
{shell,		"shell",	shell_PROG_HELP},		\
{arguments,	"args",		arguments_PROG_HELP},		\
{segm_fault,	"segm_fault", 	segm_fault_PROG_HELP},		\
{run_all,	"run_all", 	run_all_PROG_HELP},		\
{NULL, 		NULL, 		NULL}				\
}
