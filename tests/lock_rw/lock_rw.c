#include <stdio.h>
#include <os.h>

#define THREAD_COUNT 5

/* global read-write lock to test */
lock_rw_t lock;

int t_write_lock(void *data)
{
	int i;
	long int t = (long int)data;
	for (i = 0; i < 15; i++)
	{
		printf("%ld trying write lock...\n", t);
		lock_rw_write(&lock);
		printf("%ld write locked\n", t);
		os_sleep(1.0);
		printf("%ld write released\n", t);
		lock_rw_unlock(&lock);
		os_sleep(1.0);
	}

	printf("%ld exit\n", t);
	return 0;
}

int t_read_lock(void *data)
{
	int i;
	long int t = (long int)data;
	for (i = 0; i < 10; i++)
	{
		lock_rw_read(&lock);
		printf("%ld read locked\n", t);
		os_sleep((long double)(250000 + (rand() % 100000)) / 1000000.0);
		printf("%ld read released\n", t);
		lock_rw_unlock(&lock);
		os_sleep((long double)(250000 + (rand() % 100000)) / 1000000.0);
	}

	printf("%ld exit\n", t);
	return 0;
}

int main(void)
{
	long int i;
	thread_t threads[THREAD_COUNT];

	lock_rw_init(&lock);

	for (i = 0; i < THREAD_COUNT; i++)
	{
		threads[i] = thread_create(i < 2 ? t_write_lock : t_read_lock, (void *)i);
	}

	for (i = 0; i < THREAD_COUNT; i++)
	{
		thread_wait(threads[i]);
	}

	return 0;
}
