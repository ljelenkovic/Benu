/*! Memory management */
#define _K_MEMORY_C_

#include "memory.h"

#include <kernel/kprint.h>
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

/*! List of programs */
list_t kprogs;

/*! Initial memory layout created in arch layer */
void k_memory_init()
{
	int i;
	kprog_t *kprog;

	k_mpool = NULL;
	mseg = arch_memory_init();

	/* initialize dynamic memory allocation subsystem */
	for (i = 0; mseg[i].type != MS_END && !k_mpool; i++)
	{
		if (mseg[i].type == MS_KHEAP)
		{
			k_mpool = k_mem_init(mseg[i].start, mseg[i].size);
			break;
		}
	}

	ASSERT(k_mpool);

	list_init(&kprogs);

	/* look into each segment marked as program and add it to 'progs' */
	for (i = 0; mseg[i].type != MS_END; i++)
	{
		if (mseg[i].type != MS_PROGRAM)
			continue;

		kprog = kmalloc(sizeof(kprog_t));

		kprog->m = &mseg[i];
		kprog->prog = &((module_program_t *) mseg[i].start)->prog;

		kprog->started = FALSE;

		list_append(&kprogs, kprog, &kprog->list);
	}
}

void *k_mem_init(void *segment, size_t size)
{
	return K_MEM_INIT(segment, size);
}
void *kmalloc(size_t size)
{
	return KMALLOC(size);
}
int kfree(void *chunk)
{
	return KFREE(chunk);
}

void *k_process_start_adr(void *proc)
{
	return ((kprocess_t *) proc)->m.start;
}

size_t k_process_size(void *proc)
{
	return ((kprocess_t *) proc)->m.size;
}

/*! kernel <--> user address translation (using segmentation) */
void *k_u2k_adr(void *uadr, kprocess_t *proc)
{
	ASSERT((aint) uadr < proc->m.size);

	return uadr + (aint) proc->m.start;
}
void *k_k2u_adr(void *kadr, kprocess_t *proc)
{
	ASSERT((aint) kadr >= (aint) proc->m.start &&
		(aint) kadr < (aint) proc->m.start + proc->m.size);

	return kadr - (aint) proc->m.start;
}

/*! Allocate space for kernel object and for process descriptor of that object*/
void *kmalloc_kobject(kprocess_t *proc, size_t obj_size)
{
	kobject_t *kobj;

	ASSERT(proc);

	kobj = kmalloc(sizeof(kobject_t) + obj_size);
	ASSERT(kobj);

	kobj->flags = 0;
	kobj->ptr = NULL;

	if (obj_size)
		kobj->kobject = kobj + 1;
	else
		kobj->kobject = NULL;

	list_append(&proc->kobjects, kobj, &kobj->list);

	return kobj;
}

/*! Free space reserved by kernel object */
void *kfree_kobject(kprocess_t *proc, kobject_t *kobj)
{
	ASSERT(proc && kobj);

#ifndef DEBUG
	list_remove(&proc->kobjects, 0, &kobj->list);
#else /* DEBUG */
	ASSERT(list_find_and_remove(&proc->kobjects, &kobj->list));
#endif

	kfree(kobj);

	return EXIT_SUCCESS;
}

/*! Free space reserved by kernel objects associated with process */
int kfree_process_kobjects(kprocess_t *proc)
{
	ASSERT(proc);

	kobject_t *kobj;

	while ((kobj = list_remove(&proc->kobjects, 0, NULL)) != NULL)
		kfree(kobj);

	return EXIT_SUCCESS;
}


/*! unique system wide id numbers */
#define	WBITS		(sizeof(word_t) * 8)
#define	ID_ELEMS	((MAX_RESOURCES-1) / WBITS + 1)
#define MAX_RES		(ID_ELEMS * WBITS)

static word_t idmask[ ID_ELEMS ] = {0};
static id_t last_id = 0;

/*! Allocate and return unique id for new system resource */
id_t k_new_id()
{
	id_t id = -1;
	uint elem, n, start;
	word_t mask;

	last_id++;
	if (last_id == MAX_RES)
		last_id = 1; /* skip 0 */

	elem = last_id / WBITS;
	mask = idmask [elem] |((1 << (last_id % WBITS)) - 1);
	/* do not look at lower bits (for now) */

	if (~mask) /* current 'elem' has free ids from last_id forward */
	{
		id = lsb_index(~mask);
		ASSERT(id != -1);
	}
	else {
		n = start = (elem + 1) % ID_ELEMS;
		do {
			if (~idmask[n])
			{
				id = lsb_index(~idmask[n]);
				elem = n;
				break;
			}
			n = (n + 1) % ID_ELEMS;
		}
		while (n != start);
	}

	ASSERT(id != -1);

	idmask [ elem ] |= (1 << id);	/* reserve ID */
	id += elem * WBITS;
	last_id = id;

	return id;
}

/*! Release resource id */
void k_free_id(id_t id)
{
	ASSERT(id > 0 && id < MAX_RES &&
		(idmask [ id / WBITS ] &(1 << (id % WBITS))));

	idmask [ id / WBITS ] &= ~(1 << (id % WBITS));
}

/*! Check if "id" is used (if object is alive) */
int k_check_id(id_t id)
{
	if (
		id < 1 || id >= MAX_RES ||
		(idmask [ id / WBITS ] &(1 << (id % WBITS))) == 0
	)
		return 0;
	else
		return 1;

}

#undef	MAX_RES
#undef	WBITS
#undef	ID_ELEMS


/* use bitmap to find free memory block for thread stack */
void *kprocess_stack_alloc(kprocess_t *kproc)
{
	int i, j, m;

	m = (kproc->smap_size + sizeof(uint)*8 - 1) /(sizeof(uint)*8);

	/* find first 0 in stack mask */
	for (i = 0; i < m; i++)
	{
		if (kproc->smap[i] != (uint) -1)
			break;
	}
	if (i == m)
		return NULL;

	j = lsb_index(~kproc->smap[i]);

	kproc->smap[i] |= 1<<j;

	return kproc->stack + kproc->thread_stack_size * j;
}
/* free thread stack */
void kprocess_stack_free(kprocess_t *kproc, void *stack)
{
	int i, j;

	j = ((uint) stack - (uint) kproc->stack) / kproc->thread_stack_size;
	i = j /(sizeof(uint) * 8);
	j %= (sizeof(uint) * 8);

	kproc->smap[i] &= ~(1<<j);
}



/*!
 * Give list of all programs
 * \param buffer Pointer to string where to save all programs names
 * \param buf_size Size of 'buffer'
 * \return 0 if successful, ENOMEM if buffer not big enough
 */
int k_list_programs(char *buffer, size_t buf_size)
{
	size_t cur_size;
	kprog_t *kprog;
	char hdr[] = "List of programs:\n";

	buffer[0] = 0; /* set empty string */
	cur_size = 0;
	kprog = list_get(&kprogs, FIRST);

	if (strlen(hdr) > buf_size)
		return ENOMEM;
	strcpy(buffer, hdr);

	while (kprog)
	{
		cur_size += strlen(kprog->prog->name);

		if (cur_size > buf_size)
			return ENOMEM;

		strcat(buffer, kprog->prog->name);
		strcat(buffer, " ");
		kprog = list_get_next(&kprog->list);
	}

	return EXIT_SUCCESS;
}

/*! print memory layout */
void k_memory_info()
{
	int i;

	kprintf("Memory segments\n"
		 "===============\n"
		 "Type\tsize\t\tstart addres\n"
	);

	for (i = 0; mseg[i].type != MS_END && i < 20; i++)
	{
		kprintf("%d\t%x\t%x\n", mseg[i].type, mseg[i].size,
					  mseg[i].start);
	}
}

/*! Handle memory fault interrupt(and others undefined) */
void k_memory_fault()
{
	LOG(ERROR, "Undefined fault(exception)!!!");

	if (arch_prev_mode() == KERNEL_MODE)
	{
		LOG(ERROR, "PANIC: kernel caused GPF!");
		halt();
	}
	else {
		/* terminate active thread */
		LOG(ERROR, "Thread caused GPF, terminating!");
		kthread_exit(kthread_get_active(), NULL, TRUE);
	}
}

/*! printf (or return) system information (and details) */
int sys__sysinfo(void *p)
{
	char *buffer;
	size_t buf_size;
	char **param; /* last param is NULL */
	char *param1; /* *param0; */
	char usage[] = "Usage: sysinfo [programs|threads|memory]";
	char look_console[] = " (sysinfo printed on console)";

	buffer = *((char **) p); p += sizeof(char *);
	ASSERT_ERRNO_AND_EXIT(buffer, EINVAL);

	buffer = U2K_GET_ADR(buffer, kthread_get_process(NULL));

	buf_size = *((size_t *) p); p += sizeof(size_t *);

	param = *((char ***) p);
	param = U2K_GET_ADR(param, kthread_get_process(NULL));
	/* param0 = U2K_GET_ADR(param[0], kthread_get_process(NULL));
	-- param0 should be "sysinfo" so actualy its not required */

	if (param[1] == NULL)
	{
		/* only basic info defined in kernel/startup.c */
		extern char system_info[];

		if (strlen(system_info) > buf_size)
			EXIT(ENOMEM);

		strcpy(buffer, system_info);

		EXIT(EXIT_SUCCESS);
	}
	else {
		param1 = U2K_GET_ADR(param[1], kthread_get_process(NULL));

		/* extended info is requested */
		if (strcmp("programs", param1) == 0)
		{
			EXIT(k_list_programs(buffer, buf_size));
			/* TODO: "program prog_name" => print help_msg */
		}
		else if (strcmp("memory", param1) == 0)
		{
			k_memory_info();
			if (strlen(look_console) > buf_size)
				EXIT(ENOMEM);
			strcpy(buffer, look_console);
			EXIT(EXIT_SUCCESS);
			/* TODO: "memory [segments|modules|***]" */
		}
		else if (strcmp("threads", param1) == 0)
		{
			kthread_info();
			if (strlen(look_console) > buf_size)
				EXIT(ENOMEM);
			strcpy(buffer, look_console);
			EXIT(EXIT_SUCCESS);
			/* TODO: "thread id" */
		}
		else {
			if (strlen(usage) > buf_size)
				EXIT(ENOMEM);
			strcpy(buffer, usage);
			EXIT(ESRCH);
		}
	}
}
