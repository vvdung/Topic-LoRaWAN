
#include "../inc/HFactor.h"
#include "../inc/PoolThread.h"

////////CPoolThread//////////////////////////////////

CPoolThread::CPoolThread(int maxThreads){
    m_cond  = PTHREAD_COND_INITIALIZER;
    m_mutex = PTHREAD_MUTEX_INITIALIZER;    
    int iCPUs = get_nprocs_conf();
    m_maxThreads = maxThreads;
    if (m_maxThreads <= 0) m_maxThreads = iCPUs * 2;
    m_shutdown = p_shutdown_none;
    m_threadCount = 0;
    m_threadStarted = 0;
    m_pThreads = (pthread_t*)malloc(sizeof(pthread_t) * m_maxThreads);
    for (int i = 0; i < m_maxThreads; ++i){
        if (pthread_create(&m_pThreads[i], NULL, _ThreadWorker, this) == 0){
            ++m_threadCount;
            ++m_threadStarted;
        }        
    }
    printf("[=] POOL THREADS: %d/%d\n",m_threadStarted,m_maxThreads);
}

CPoolThread::~CPoolThread(void){
    Destroy();
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
    free(m_pThreads);
    while (!m_queueTask.empty()){
        POOL_TASK* task = m_queueTask.front();
        m_queueTask.pop();
        free(task);
    }
    //printf("\nThreads Count[%d] Started[%d] Max[%d] \n",m_threadCount,m_threadStarted,m_maxThreads);
}

int CPoolThread::Lock(){
    return pthread_mutex_lock(&m_mutex);
}
int CPoolThread::Unlock(){
    return pthread_mutex_unlock(&m_mutex);
}
int CPoolThread::Wait(pthread_cond_t* cond){
    if (!cond) return pthread_cond_wait(&m_cond,&m_mutex);
    return pthread_cond_wait(cond,&m_mutex);
}
int CPoolThread::Signal(pthread_cond_t* cond){
    if (!cond) return pthread_cond_signal(&m_cond);
    return pthread_cond_signal(cond);
}

int CPoolThread::AddTask(FN_Task fn, void* arg, void* user){
    if (!fn) return -1;
    int err = 0;
    POOL_TASK* task;
    Lock();
    if (m_shutdown != p_shutdown_none){
        err = -2;
        goto _AT_OUT;
    }
    task = (POOL_TASK *)malloc(sizeof(POOL_TASK));
    if (!task) goto _AT_OUT;
    task->fn = fn;
    task->arg = arg;
    task->user = user;
    m_queueTask.push(task);
    Signal();

_AT_OUT:    
    Unlock();
    return err;
}

void* CPoolThread::_ThreadWorker(void *arg){
    CPoolThread* pThread = (CPoolThread*)arg;
    pThread->OnThreadWorker();
    return NULL;
}

void CPoolThread::OnThreadWorker(){
    POOL_TASK* task;
    while (1){
        Lock();
        while (m_queueTask.size() == 0 && m_shutdown == p_shutdown_none){
            Wait();
        }
        if(m_shutdown == p_shutdown_immediate) break;
        else if((m_shutdown == p_shutdown_graceful) && (m_queueTask.size() == 0)) break;

        if (m_queueTask.empty()){
            Unlock();
            continue;
        }
        task = m_queueTask.front();
        m_queueTask.pop();
        Unlock();
        task->fn(task->arg,task->user);
        free(task);
    }//while (1){
    --m_threadStarted;
    Unlock();
    pthread_exit(NULL);
}
int CPoolThread::Destroy(POOL_SHUTDOWN shutdown){
    Lock();
    int err = 0;
    do{
        if(m_shutdown != p_shutdown_none){
            err = -1;
            break;
        }

        m_shutdown = (shutdown) ? p_shutdown_graceful : p_shutdown_immediate;

        if(pthread_cond_broadcast(&m_cond) != 0){
            err = -2;
            break;
        }

        if(Unlock() != 0){
            err = -3;
            break;
        }

        for(int i = 0; i < m_threadCount; i++){
            pthread_join(m_pThreads[i], NULL);
        }
    }while(0);

    return err;
}
void CPoolThread::ShowInfo(){
    Lock();
    printf("\n-------POOL THREADS--------------\n");
    printf("Threads Count[%d] Started[%d] Max[%d] QueueTask[%lu]\n",
        m_threadCount,m_threadStarted,m_maxThreads,m_queueTask.size());
    printf("\n---------------------\n");
    Unlock();
}