/*! standalone memory allocator test (stress tests for errors) */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>


#include <pthread.h>

#if 0
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
timespec_t { long tv_sec; long tv_nsec; };
#endif /* CLOCK_REALTIME */
#endif

#if defined(FIRST_FIT)

/*! interface */
typedef void ffs_mpool_t;

void *ffs_init(void *mem_segm, size_t size);
void *ffs_alloc(ffs_mpool_t *mpool, size_t size);
int ffs_free(ffs_mpool_t *mpool, void *chunk_to_be_freed);

#define	MEM_INIT(ADDR, SIZE)		ffs_init(ADDR, SIZE)
#define MEM_ALLOC(MP, SIZE)		ffs_alloc(MP, SIZE)
#define MEM_FREE(MP, ADDR)		ffs_free(MP, ADDR)

#elif defined(GMA)

#define gma_t	void

gma_t *gma_init(void *memory_segment, size_t size, size_t min_chunk_size,
		    uint flags);
void *gma_alloc(gma_t *mpool, size_t size);
int gma_free(gma_t *mpool, void *address);

#define	MEM_INIT(ADDR, SIZE)		gma_init(ADDR, SIZE, 32, 0)
#define MEM_ALLOC(MP, SIZE)		gma_alloc(MP, SIZE)
#define MEM_FREE(MP, ADDR)		gma_free(MP, ADDR)

#endif

/* #define PRINT(format, ...) printf(format, ##__VA_ARGS__) */
#define PRINT(format, ...)

/* testing */
int main()
{
	int pool_size = 1234567;
	int max_block_size = 1512;
	int init_requests = 1500, requests = 100000;
	int i, j, k, used, fail;
	size_t inuse = 0;
	struct req
	{
		void *ptr;
		unsigned int size;
	}
	m[requests];
	void *pool, *mpool;
	struct timespec t1, t2;

	if ((pool = malloc(pool_size)) == NULL)
	{
		printf("Malloc return NULL\n");
		return 1;
	}

	memset(pool, 0, pool_size);

	for (j = 0; j < requests; j++)
	{
		m[j].ptr = NULL;
		m[j].size = 0;
	}

	mpool = MEM_INIT(pool, pool_size);

	used = 0;
	fail = 0;

	/* initial allocations */
	for (j = 0; j < init_requests; j++)
	{
		m[j].size = lrand48() % max_block_size + 4;
		clock_gettime(CLOCK_REALTIME, &t1);
		m[j].ptr = MEM_ALLOC(mpool, m[j].size);
		clock_gettime(CLOCK_REALTIME, &t2);

		if (m[j].ptr != NULL)
		{
			memset(m[j].ptr, 5, m[j].size);
			used++;
			inuse += m[j].size;

			PRINT("%u %u %ld\n", (unsigned int) m[j].ptr, m[j].size,
				(t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec);
		}
		else {
			fail++;
			PRINT("[%d] alloc=%p\t[%u]\n", j, m[j].ptr, m[j].size);
			PRINT("FAIL(%d)\n", fail);

			break;
		}
	}

	printf("Start of tests(j=%d, fail=%d, inuse=%lu)!\n", j, fail, inuse);

	fail = 0;

	for (i = 0; i < requests; i++)
	{
		if (lrand48() & 1)
		{
			/* alloc */
			for (j = 0; j < requests && m[j].ptr != NULL; j++)
				;

			if (j >= requests)
			{
				printf("No free element in m[]!\n");
				break;
			}

			m[j].size = lrand48() %(max_block_size) + 4;

			clock_gettime(CLOCK_REALTIME, &t1);
			m[j].ptr = MEM_ALLOC(mpool, m[j].size);
			clock_gettime(CLOCK_REALTIME, &t2);

			if (m[j].ptr != NULL)
			{
				memset(m[j].ptr, 3, m[j].size);
				used++;
				inuse += m[j].size;

				PRINT("%u %ud %ld\n", (unsigned int) m[j].ptr, m[j].size,
					(t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec);
			}
			else {
				fail++;
				if (fail == 1)
					printf("\tFirst fail(i=%d)!\n", i);
			}
		}
		else {
			/* free */
			while (used > 0)
			{
				k = lrand48() % requests;
				if (m[k].ptr != NULL)
				{
					MEM_FREE(mpool, m[k].ptr);

					m[k].ptr = NULL;

					used--;
					inuse -= m[j].size;

					break;
				}
			}
		}
	}

	printf("End of tests(i=%d, fail=%d, inuse=%lu)!\n", i, fail, inuse);

	return 0;
}
