/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __OS_H__
#define __OS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#include <time.h>

#ifdef _WIN32 /* if compiling in windows */
#include <windows.h>
typedef HANDLE thread_t;
typedef HANDLE mutex_t;
typedef HINSTANCE dll_t;

/** @todo semaphores and read/write-locks windows style! */
#include <pthread.h>
#include <semaphore.h>
typedef pthread_rwlock_t lock_rw_t;
//typedef HANDLE sem_t;

#else
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <dlfcn.h>
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_rwlock_t lock_rw_t;
typedef void * dll_t;
#endif


/**
 * Create thread.
 */
DLLEXP thread_t thread_create(int (*fn)(void *), void *data);
/**
 * Wait for thread to finish.
 */
DLLEXP void thread_wait(thread_t thread);
/**
 * Kill thread (force thread to quit).
 */
DLLEXP void thread_kill(thread_t thread);

/**
 * Initialize mutex.
 *
 * @return whether call was succesful(0) or not(-1).
 */
DLLEXP int mutex_init(mutex_t *mutex);

/**
 * Destroy mutex.
 *
 * @return whether call was succesful(0) or not(-1).
 */
DLLEXP int mutex_destroy(mutex_t *mutex);

/**
 * Lock mutex. Can be called multiple times inside same thread
 * (Recursive mutex). If so, unlock must be called same number of times.
 *
 * @return whether call was succesful(0) or not(-1).
 */
DLLEXP int mutex_lock(mutex_t *mutex);

/**
 * Unlock mutex.
 *
 * @return whether call was succesful(0) or not(-1).
 */
DLLEXP int mutex_unlock(mutex_t *mutex);

/**
 * Create read-write lock.
 */
DLLEXP int lock_rw_init(lock_rw_t *lock);

/**
 * Lock for reading.
 */
DLLEXP int lock_rw_read(lock_rw_t *lock);

/**
 * Lock for writing.
 */
DLLEXP int lock_rw_write(lock_rw_t *lock);

/**
 * Release lock.
 */
DLLEXP int lock_rw_unlock(lock_rw_t *lock);

/**
 * Destroy lock.
 */
DLLEXP int lock_rw_destroy(lock_rw_t *lock);

/**
 * Open dynamic library.
 *
 * @param name name of dynamic library to open
 * @return handle to library on success, NULL on errors
 */
DLLEXP dll_t dll_open(char *name);

/**
 * Find handle to function/object in dynamic library.
 *
 * @param d handle to dynamic library
 * @param name name of function/object
 * @return pointer to function/object on success, NULL on errors
 */
DLLEXP void *dll_sym(dll_t d, char *name);

/**
 * Close handle to dynamic library.
 *
 * @param d handle to dynamic library
 */
DLLEXP void dll_close(dll_t d);

/**
 * Get current time in seconds with maximum precision available.
 *
 * @return current time in seconds
 */
DLLEXP long double os_time();

/**
 * Sleep until given amount of time has passed.
 * Accuracy depends but mostly do not use values less than 1 millisecond.
 *
 * @param t time to sleep in seconds
 */
DLLEXP void os_sleep(long double t);


#ifdef __cplusplus
}
#endif

#endif /* __OS_H__ */
