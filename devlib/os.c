/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include <math.h>
#include <time.h>
#include <errno.h>
#include "os.h"

#ifdef _WIN32 /* If compiling in windows. */


thread_t thread_create(int (*fn)(void *), void *data)
{
	thread_t thread;

	DWORD tid;
	thread = CreateThread(NULL, 0, (unsigned long (__stdcall*)(void*))fn, data, 0, &tid);

	return thread;
}

void thread_wait(thread_t thread)
{
	WaitForSingleObject(thread, INFINITE);
	//GetExitCodeThread(thread, (unsigned long *)ret);
	return;
}

void thread_kill(thread_t thread)
{
	TerminateThread(thread, 0);
	return;
}
int mutex_init(mutex_t *mutex)
{
	(*mutex) = CreateMutex(NULL, FALSE, NULL);
	if ((*mutex) == NULL)
		return -1;

	return 0;
}

int mutex_destroy(mutex_t *mutex)
{
	return (int)(CloseHandle(mutex) == FALSE ? -1 : 0);
}

int mutex_lock(mutex_t *mutex)
{
	return (int)(WaitForSingleObject(mutex, INFINITE) == FALSE ? -1 : 0);
}

int mutex_unlock(mutex_t *mutex)
{
	return (int)(ReleaseMutex(mutex) == FALSE ? -1 : 0);
}

int lock_rw_init(lock_rw_t *lock)
{
	return pthread_rwlock_init(lock, NULL);
}

int lock_rw_read(lock_rw_t *lock)
{
	return pthread_rwlock_rdlock(lock);
}

int lock_rw_write(lock_rw_t *lock)
{
	return pthread_rwlock_wrlock(lock);
}

int lock_rw_unlock(lock_rw_t *lock)
{
	return pthread_rwlock_unlock(lock);
}

int lock_rw_destroy(lock_rw_t *lock)
{
	return pthread_rwlock_destroy(lock);
}

dll_t dll_open(char *name)
{
	return LoadLibraryA(name);
}

void *dll_sym(dll_t d, char *name)
{
	return GetProcAddress(d, name);
}

void dll_close(dll_t d)
{
	FreeLibrary(d);
}

long double os_time()
{
	LARGE_INTEGER frequency, ticks;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&ticks);
	return (long double)ticks.QuadPart / (long double)frequency.QuadPart;
}

void os_sleep(long double t)
{
	HANDLE timer;
	LARGE_INTEGER ticks;
	/* SetWaitableTimer() uses 100 nanosecond interval, negative valute indicates relative time */
	ticks.QuadPart = -(LONGLONG)(t * 1e7);
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ticks, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

#else

void *thread_run(void *d)
{
	int (*fn)(void *);
	void *data;

	/* extract data */
	fn = (int (*)(void *))((void **)d)[0];
	data = ((void **)d)[1];

	/* free data */
	free(d);

	/* execute function */
	fn(data);

	/* exit */
	pthread_exit(NULL);

	/* prevent warnings */
	return NULL;
}

thread_t thread_create(int (*fn)(void *), void *data)
{
	thread_t thread;

	/* create thread data */
	void **d = (void **)malloc(sizeof(void *) * 2);
	if (!d)
	{
		return 0;
	}

	/* setup data */
	d[0] = (void *)fn;
	d[1] = data;

	/* create thread */
	if (pthread_create(&thread, NULL, thread_run, d) != 0)
	{
		free(d);
		return -1;
	}

	return thread;
}

void thread_wait(thread_t thread)
{
	pthread_join(thread, NULL);
	return;
}

void thread_kill(thread_t thread)
{
	pthread_kill(thread, SIGKILL);
	return;
}

int mutex_init(mutex_t *mutex)
{
	int err = 0;
	pthread_mutexattr_t p_attr;
	pthread_mutexattr_init(&p_attr);
	pthread_mutexattr_settype(&p_attr, PTHREAD_MUTEX_RECURSIVE_NP);
	err = pthread_mutex_init(mutex, &p_attr);
	pthread_mutexattr_destroy(&p_attr);
	return err;
}

int mutex_destroy(mutex_t *mutex)
{
	return pthread_mutex_destroy(mutex);
}

int mutex_lock(mutex_t *mutex)
{
	return pthread_mutex_lock(mutex);
}

int mutex_unlock(mutex_t *mutex)
{
	return pthread_mutex_unlock(mutex);
}

int lock_rw_init(lock_rw_t *lock)
{
	return pthread_rwlock_init(lock, NULL);
}

int lock_rw_read(lock_rw_t *lock)
{
	return pthread_rwlock_rdlock(lock);
}

int lock_rw_write(lock_rw_t *lock)
{
	return pthread_rwlock_wrlock(lock);
}

int lock_rw_unlock(lock_rw_t *lock)
{
	return pthread_rwlock_unlock(lock);
}

int lock_rw_destroy(lock_rw_t *lock)
{
	return pthread_rwlock_destroy(lock);
}

dll_t dll_open(char *name)
{
	return dlopen(name, RTLD_LAZY);
}

void *dll_sym(dll_t d, char *name)
{
	return dlsym(d, name);
}

void dll_close(dll_t d)
{
	dlclose(d);
}

long double os_time()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (long double)((long double)tp.tv_sec + (long double)tp.tv_nsec / 1e9);
}

void os_sleep(long double t)
{
	struct timespec tp;
	long double integral;
	t += os_time();
	tp.tv_nsec = (long)(modfl(t, &integral) * 1e9);
	tp.tv_sec = (time_t)integral;
	while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &tp, NULL) == EINTR);
}

#endif

