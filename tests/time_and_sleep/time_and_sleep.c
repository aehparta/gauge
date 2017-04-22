#include <stdio.h>
#include <os.h>
#include <time.h>

int main(void)
{
	int i, j;

	printf("testing os_time() and os_sleep() functions\n");

#ifndef _WIN32
	struct timespec tp;
	clock_getres(CLOCK_MONOTONIC, &tp);
	printf("CLOCK_MONOTONIC resolution in this system is %ld.%09ld\n", tp.tv_sec, tp.tv_nsec);
#endif

	printf("os_sleep(1.0) * 10 loop\n");
	for (i = 0; i < 1; i++)
	{
		os_sleep(1.0);
		printf("1 sec ...\n");
	}

	printf("os_sleep(0.001) * 1000 * 10 loop\n");
	for (i = 0; i < 10; i++)
	{
		long double diff = 0.0;
		for (j = 0; j < 1000; j++)
		{
			long double start = os_time();
			os_sleep(0.001);
			diff += os_time() - start;
		}
		printf("1 sec, diff %lf...\n", (double)(diff / 1000));
	}

	return 0;
}
