#pragma once
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <map>
#include <queue>

typedef void (*FN_Task)(void*,void*);
typedef struct _POOL_TASK{
    FN_Task fn;
    void* arg;
    void* user; //class called
}POOL_TASK;

typedef enum{
    p_shutdown_none = 0,
    p_shutdown_immediate,
    p_shutdown_graceful
}POOL_SHUTDOWN;

class CPoolThread
{
public:
    CPoolThread(int maxThreads = 0);
	~CPoolThread(void);

    int                 AddTask(FN_Task fn, void* arg, void* user = NULL);
    int                 Destroy(POOL_SHUTDOWN = p_shutdown_graceful);
    void                ShowInfo();

protected:
    static void*        _ThreadWorker(void *);
    void		        OnThreadWorker();

    int                 Lock();
    int                 Unlock();
    int                 Wait(pthread_cond_t* cond = NULL);
    int                 Signal(pthread_cond_t* cond = NULL);

    pthread_t*              m_pThreads;
    int                     m_threadCount;
    std::queue<POOL_TASK*>  m_queueTask;
    pthread_mutex_t         m_mutex;
    pthread_cond_t          m_cond;
    int                     m_maxThreads;
    POOL_SHUTDOWN           m_shutdown;
    int                     m_threadStarted;
};