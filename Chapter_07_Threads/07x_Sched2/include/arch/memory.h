/*! Memory segments */
#pragma once

#include <types/basic.h>

/* Memory segments */
enum {
	MS_KERNEL = 3,
	MS_KHEAP,
	MS_PROGRAM,
	MS_MODULE,
	MS_END
};

typedef struct _mseg_t_
{
	uint	 type;
	void	*start;
	size_t	 size;
}
mseg_t;

mseg_t *arch_memory_init();
