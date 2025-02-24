/*! Test SSE support(x86) */

#include <stdio.h>
#include <pthread.h>
#include <time.h>

char PROG_HELP[] = "Test SSE support by using SSE register.";

#define THR_NUM	3
#define ITERS	5

static timespec_t sleep;

static void *sse_test_thread(void *param)
{
	int thr_no = (int) param;

	printf("Thread %d starting\n", thr_no);

#ifdef USE_SSE
	struct xmm_reg
	{
		int p1, p2, p3, p4;
	}
	reg1, reg2;
	int i;
	extern uint32 arch_sse_supported;
	if (!arch_sse_supported)
	{
		printf("SSE support compiled, "
			 "but not supported by processor!\n");
		printf("Thread %d exiting\n", thr_no);
		return NULL;
	}

	/* load xmm0 register */
	reg1.p1 = reg1.p2 = reg1.p3 = reg1.p4 = thr_no;
	asm volatile ("movups %0, %%xmm0" :: "m" (reg1));

	for (i = 1; i <= ITERS; i++)
	{
		printf("Thread %d: iter %d - ", thr_no, i);

		/* load from xmm0 register */
		asm volatile ("movups %%xmm0, %0" : "=m" (reg2));
		printf("xmm0 register: %d, %d, %d, %d\n",
			reg2.p1, reg2.p2, reg2.p3, reg2.p4);

		nanosleep(&sleep, NULL);
	}
#else
	printf("SSE support not included(compiled)!\n");
#endif
	printf("Thread %d exiting\n", thr_no);

	return NULL;
}

int sse_test(char *args[])
{
	pthread_t thread[THR_NUM];
	int i, j;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	sleep.tv_sec = 1;
	sleep.tv_nsec = 0;

	for (i = 0; i < THR_NUM; i++)
		if (pthread_create(&thread[i], NULL,
				      sse_test_thread, (void *) i))
		{
			printf("Thread not created!\n");
			break;
		}

	nanosleep(&sleep, NULL);
	nanosleep(&sleep, NULL);

	for (j = 0; j < i; j++)
		pthread_join(thread[j], NULL);

	return 0;
}
