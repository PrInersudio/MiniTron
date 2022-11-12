#ifndef THREADS
#define THREADS
void Pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg);
void  Pthread_join(pthread_t thread, void **retval);
void Pthread_cancel(pthread_t thread);
int Pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
int Pthread_mutex_destroy(pthread_mutex_t *mutex);
int Pthread_mutex_lock(pthread_mutex_t *mutex);
int Pthread_mutex_unlock(pthread_mutex_t *mutex);
#endif
