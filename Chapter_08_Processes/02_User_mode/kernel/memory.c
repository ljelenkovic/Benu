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

list_t kobjects; /* list of kernel objects reserved by programs */

/*! Initial memory layout created in arch layer */
void k_memory_init ()
{
	int i;

	mseg = arch_memory_init ();

	/* find kernel heap */
	for ( i = 0; mseg[i].type != MS_END && !k_mpool; i++ )
	{
		if ( mseg[i].type == MS_KHEAP )
		{
			k_mpool = k_mem_init ( mseg[i].start, mseg[i].size );
			break;
		}
	}

	ASSERT ( k_mpool );
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

/*! Allocate space for kernel object and descriptor of that object */
void *kmalloc_kobject ( size_t obj_size )
{
	kobject_t *kobj;

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

	list_append ( &kobjects, kobj, &kobj->list );

	return kobj;
}
void *kfree_kobject ( kobject_t *kobj )
{
#ifndef DEBUG
	list_remove ( &kobjects, 0, &kobj->list );
#else /* DEBUG */
	ASSERT ( list_find_and_remove ( &kobjects, &kobj->list ) );
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

	char usage[] = "Usage: sysinfo [programs|threads|memory]";
	char look_console[] = "(sysinfo printed on console)";

	buffer =   *( (char **) p );	p += sizeof (char *);
	buf_size = *( (size_t *) p );	p += sizeof (size_t *);
	param =    *( (char ***) p );

	ASSERT_ERRNO_AND_EXIT ( buffer, EINVAL );

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
		/* extended info is requested */
		if ( strcmp ( "memory", param[1] ) == 0 )
		{
			k_memory_info ();
			if ( strlen ( look_console ) > buf_size )
				EXIT ( ENOMEM );
			strcpy ( buffer, look_console );
			EXIT ( EXIT_SUCCESS );
			/* TODO: "memory [segments|modules|***]" */
		}
		else if ( strcmp ( "threads", param[1] ) == 0 )
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
