#include "sal.h"
#include <pthread.h>

struct sal_mutex
{
    pthread_mutex_t lock;
};

struct sal_cond
{
    pthread_cond_t cond;
};

/*added by ychen for recusive lock in 2011-06-29*/
sal_err_t sal_mutex_recursive_create(sal_mutex_t** pmutex)
{
    sal_mutex_t* mutex;
    pthread_mutexattr_t attr;    

    SAL_MALLOC(mutex, sal_mutex_t *, sizeof(sal_mutex_t));
    if (!mutex)
        return ENOMEM;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex->lock, &attr);
    pthread_mutexattr_destroy(&attr);      

    *pmutex = mutex;
    return 0;
}
/*ychen end*/
sal_err_t sal_mutex_create(sal_mutex_t** pmutex)
{
    sal_mutex_t* mutex;

    SAL_MALLOC(mutex, sal_mutex_t *, sizeof(sal_mutex_t));
    if (!mutex)
        return ENOMEM;

    pthread_mutex_init(&mutex->lock, NULL);

    *pmutex = mutex;
    return 0;
}

sal_err_t sal_cond_create(sal_cond_t** pcond)
{
    sal_cond_t* cond;

    SAL_MALLOC(cond, sal_cond_t *, sizeof(sal_cond_t));
    if (!cond)
        return ENOMEM;

    pthread_cond_init(&cond->cond, NULL);

    *pcond = cond;
    return 0;
}


void sal_mutex_destroy(sal_mutex_t *mutex)
{
    pthread_mutex_destroy(&mutex->lock);
    SAL_FREE(mutex);
}

void sal_mutex_lock(sal_mutex_t *mutex)
{
    pthread_mutex_lock(&mutex->lock);
}

void sal_mutex_unlock(sal_mutex_t *mutex)
{
    pthread_mutex_unlock(&mutex->lock);
}

bool sal_mutex_try_lock(sal_mutex_t *mutex)
{
    return pthread_mutex_trylock(&mutex->lock) != EBUSY;
}

void sal_task_cond_signal(sal_cond_t* p_cond)
{
    pthread_cond_signal(&p_cond->cond);
}

void sal_task_cond_wait(sal_mutex_t* p_mutex, sal_cond_t* p_cond)
{
    pthread_cond_wait(&p_cond->cond, &p_mutex->lock);
}

