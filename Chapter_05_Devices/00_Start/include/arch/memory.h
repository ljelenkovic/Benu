/*! Multiboot configuration */
#pragma once

#include <lib/list.h>

/* Memory segments */
enum {
	MS_KERNEL = 1,
	MS_KHEAP,
	MS_PROGRAM,
	MS_MODULE,
	MS_END
};

typedef struct _mseg_t_
{
	uint	type;	/* MS_KERNEL, MS_KHEAP, MS_PROGRAM, MS_MODULE  */
	char	*name;
	void	*start;
	size_t	size;

	list_h	list;
}
mseg_t;

mseg_t *arch_memory_init ();
