/*! Memory management */
#define _K_MEMORY_C_

#include "memory.h"

#include "kprint.h"
#include "thread.h"
#include <kernel/errno.h>
#include <arch/processor.h>
#include <arch/interrupt.h>
#include <lib/string.h>
#include <lib/list.h>
#include <types/bits.h>

/*! Dynamic memory allocator for kernel */
MEM_ALLOC_T *k_mpool = NULL;

/*! Memory segments */
static mseg_t *mseg = NULL;

/*! List of programs loaded as modules */
list_t progs;
#define PNAME "prog_name="

/*! Initial memory layout created in arch layer */
void k_memory_init ()
{
	int i, j;
	kprog_t *prog;
	char *name;

	mseg = arch_memory_init ();

	/* initialize dynamic memory allocation subsystem */
	for ( i = 0; mseg[i].type != MS_END && !k_mpool; i++ )
	{
		if ( mseg[i].type == MS_KHEAP )
		{
			k_mpool = k_mem_init ( mseg[i].start, mseg[i].size );
			break;
		}
	}

	ASSERT ( k_mpool );

	list_init ( &progs );

	/* look into each segment marked as module, add programs to 'progs' */
	for ( i = 0; mseg[i].type != MS_END; i++ )
	{
		if ( mseg[i].type != MS_MODULE )
			continue;

		/*
		 * Is this segment a program?
		 * Programs have 'prog_name' in command line
		 */
		name = strstr ( mseg[i].name, PNAME );
		if ( name )
		{
			prog = kmalloc ( sizeof (kprog_t) );

			name += strlen ( PNAME );
			for(j = 0; j < MAX_PROG_NAME_LEN && name[j] != ' '; j++)
				prog->prog_name[j] = name[j];
			prog->prog_name[j] = 0;

			prog->pi = (void *) mseg[i].start;

			prog->m = &mseg[i];

			prog->started = FALSE;

			list_append ( &progs, prog, &prog->list );
		}
	}
}

inline void *k_mem_init ( void *segment, size_t size )
{
	return K_MEM_INIT ( segment, size );
}
inline void *kmalloc ( size_t size )
{
	return KMALLOC ( size );
}
inline int kfree ( void *chunk )
{
	return KFREE ( chunk );
}

inline void *k_process_start_adr ( void *proc )
{
	return ( (kprocess_t *) proc )->m.start;
}

inline size_t k_process_size ( void *proc )
{
	return ( (kprocess_t *) proc )->m.size;
}

/*! kernel <--> user address translation (using segmentation) */
inline void *k_u2k_adr ( void *uadr, kprocess_t *proc )
{
	ASSERT ( (aint) uadr < proc->m.size );

	return uadr + (aint) proc->m.start;
}
inline void *k_k2u_adr ( void *kadr, kprocess_t *proc )
{
	ASSERT ( (aint) kadr >= (aint) proc->m.start &&
		 (aint) kadr < (aint) proc->m.start + proc->m.size );

	return kadr - (aint) proc->m.start;
}

/*! Allocate space for kernel object and for process descriptor of that object*/
void *kmalloc_kobject ( kprocess_t *proc, size_t obj_size )
{
	kobject_t *kobj;

	ASSERT ( proc );

	kobj = kmalloc ( sizeof (kobject_t) );
	ASSERT ( kobj );

	kobj->kobject = NULL;
	kobj->flags = 0;
	kobj->ptr = NULL;

	if ( obj_size )
	{
		kobj->kobject = kmalloc ( obj_size );
		ASSERT ( kobj->kobject );
	}

	list_append ( &proc->kobjects, kobj, &kobj->list );

	return kobj;
}
void *kfree_kobject ( kprocess_t *proc, kobject_t *kobj )
{
	ASSERT ( proc && kobj );

#ifndef DEBUG
	list_remove ( &proc->kobjects, 0, &kobj->list );
#else /* DEBUG */
	ASSERT ( list_find_and_remove ( &proc->kobjects, &kobj->list ) );
#endif

	if ( kobj->kobject )
		kfree ( kobj->kobject );

	kfree ( kobj );

	return EXIT_SUCCESS;
}

/*! unique system wide id numbers */
#define	WBITS		( sizeof(word_t) * 8 )
#define	ID_ELEMS	( (MAX_RESOURCES-1) / WBITS + 1 )
#define MAX_RES		( ID_ELEMS * WBITS )

static word_t idmask[ ID_ELEMS ] = { 0 };
static id_t last_id = 0;

/*! Allocate and return unique id for new system resource */
id_t k_new_id ()
{
	id_t id = -1;
	uint elem, n;
	word_t mask;

	last_id = ( last_id + 1 < MAX_RES ? last_id + 1 : 1 ); /* skip 0 */

	elem = last_id / WBITS;
	mask = idmask [elem] | ( ( 1 << (last_id % WBITS) ) - 1 );
	/* do not look at lower bits (for now) */

	if ( ~mask ) /* current 'elem' has free ids from last_id forward */
	{
		id = lsb_index ( ~mask );
	}
	else {
		n = elem + 1;
		do {
			if ( ~idmask[n] )
			{
				id = lsb_index ( ~idmask[n] );
				elem = n;
				break;
			}
			n = ( n + 1 ) % ID_ELEMS;
		}
		while ( n != elem + 1 );
	}

	ASSERT ( id != -1 );

	idmask [ elem ] |= ( 1 << id );	/* reserve ID */
	id += elem * WBITS;
	last_id = id;

	return id;
}

/*! Release resource id */
void k_free_id ( id_t id )
{
	ASSERT ( id > 0 && id < MAX_RES &&
		 ( idmask [ id / WBITS ] & ( 1 << ( id % WBITS ) ) ) );

	idmask [ id / WBITS ] &= ~ ( 1 << ( id % WBITS ) );
}

#undef	MAX_RES
#undef	WBITS
#undef	ID_ELEMS

/*!
 * Give list of all programs
 * \param buffer Pointer to string where to save all programs names
 * \param buf_size Size of 'buffer'
 * \return 0 if successful, ENOMEM if buffer not big enough
 */
int k_list_programs ( char *buffer, size_t buf_size )
{
	size_t cur_size;
	kprog_t *prog;
	char hdr[] = "List of programs:\n";

	buffer[0] = 0; /* set empty string */
	cur_size = 0;
	prog = list_get ( &progs, FIRST );

	if ( strlen ( hdr ) > buf_size )
		return ENOMEM;
	strcpy ( buffer, hdr );

	while ( prog )
	{
		cur_size += strlen ( prog->prog_name );

		if ( cur_size > buf_size )
			return ENOMEM;

		strcat ( buffer, prog->prog_name );
		strcat ( buffer, " " );
		prog = list_get_next ( &prog->list );
	}

	return EXIT_SUCCESS;
}

/*! print memory layout */
void k_memory_info ()
{
	int i;

	kprintf ( "Memory segments\n"
		 "===============\n"
		 "Type\tsize\tstart addres\tstring\n"
	);

	for ( i = 0; mseg[i].type != MS_END && i < 20; i++ )
	{
		kprintf ( "%d\t%x\t%x\t%s\n", mseg[i].type, mseg[i].size,
					      mseg[i].start, mseg[i].name );
	}
}

/*! Handle memory fault interrupt */
void k_memory_fault ()
{
	LOG ( ERROR, "General Protection Fault!!!");

	if ( arch_prev_mode () == KERNEL_MODE )
	{
		LOG ( ERROR, "PANIC: kernel caused GPF!");
		halt ();
	}
	else {
		/* terminate active thread */
		LOG ( ERROR, "Thread caused GPF, terminating!");
		kthread_exit ( kthread_get_active(), NULL, TRUE );
	}
}

/*! printf (or return) system information (and details) */
int sys__sysinfo ( void *p )
{
	char *buffer;
	size_t buf_size;
	char **param; /* last param is NULL */
	char *param1; /* *param0; */
	char usage[] = "Usage: sysinfo [programs|threads|memory]";
	char look_console[] = "(sysinfo printed on console)";

	buffer = *( (char **) p ); p += sizeof (char *);
	ASSERT_ERRNO_AND_EXIT ( buffer, EINVAL );

	buffer = U2K_GET_ADR ( buffer, kthread_get_process (NULL) );

	buf_size = *( (size_t *) p ); p += sizeof (size_t *);

	param = *( (char ***) p );
	param = U2K_GET_ADR ( param, kthread_get_process (NULL) );
	/* param0 = U2K_GET_ADR ( param[0], kthread_get_process (NULL) );
	-- param0 should be "sysinfo" so actualy its not required */

	if ( param[1] == NULL )
	{
		/* only basic info defined in kernel/startup.c */
		extern char system_info[];

		if ( strlen ( system_info ) > buf_size )
			EXIT ( ENOMEM );

		strcpy ( buffer, system_info );

		EXIT ( EXIT_SUCCESS );
	}
	else {
		param1 = U2K_GET_ADR ( param[1], kthread_get_process (NULL) );

		/* extended info is requested */
		if ( strcmp ( "programs", param1 ) == 0 )
		{
			EXIT ( k_list_programs ( buffer, buf_size ) );
			/* TODO: "program prog_name" => print help_msg */
		}
		else if ( strcmp ( "memory", param1 ) == 0 )
		{
			k_memory_info ();
			if ( strlen ( look_console ) > buf_size )
				EXIT ( ENOMEM );
			strcpy ( buffer, look_console );
			EXIT ( EXIT_SUCCESS );
			/* TODO: "memory [segments|modules|***]" */
		}
		else if ( strcmp ( "threads", param1 ) == 0 )
		{
			kthread_info ();
			if ( strlen ( look_console ) > buf_size )
				EXIT ( ENOMEM );
			strcpy ( buffer, look_console );
			EXIT ( EXIT_SUCCESS );
			/* TODO: "thread id" */
		}
		else {
			if ( strlen ( usage ) > buf_size )
				EXIT ( ENOMEM );
			strcpy ( buffer, usage );
			EXIT ( ESRCH );
		}
	}
}
