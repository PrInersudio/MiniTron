#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "threads.h"
extern int tie;

void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
{
	int res = pthread_create(thread, attr, start_routine, arg);
	if (res)
	{
		fprintf(stderr, "pthread_create error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

void  Pthread_join(pthread_t thread, void **retval)
{
	int res = pthread_join(thread, retval);
	if (res)
	{
		fprintf(stderr, "pthread_join error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

void Pthread_cancel(pthread_t thread)
{
	int res = pthread_cancel(thread);
	if (res)
	{
		fprintf(stderr, "pthread_cancel error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

int Pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr)
{
	int res = pthread_mutex_init(mutex, attr);
	if (res)
	{
		fprintf(stderr, "pthread_mutex_init error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

int Pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	int res = pthread_mutex_destroy(mutex);
	if (res)
	{
		fprintf(stderr, "pthread_mutex_destroy error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

int Pthread_mutex_lock(pthread_mutex_t *mutex)
{
	int res = pthread_mutex_lock(mutex);

	if (res)
	{
		fprintf(stderr, "pthread_mutex_lock error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

int Pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	int res = pthread_mutex_unlock(mutex);
	if (res)
	{
		fprintf(stderr, "pthread_mutex_unlock error %d\n", res);
		tie = 1;
		exit(EXIT_FAILURE);
	}
}
