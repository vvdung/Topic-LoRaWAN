#pragma once
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <map>
#include <queue>

typedef bool (*FN_Routine)(void* thInfo);

class CPoolThread;

class CThreadInfo{
public:
    friend class CPoolThread;
    pthread_t           GetThreadId();
    void*               GetArgs();
    void                RunRoutine();
protected:
    CThreadInfo(CPoolThread* thPool);
    ~CThreadInfo();
    bool                Create();

	static void*	    _ThreadRoutine(void* lpData);
	void			    OnThreadRoutine();
    
    pthread_t           m_uTID;
    FN_Routine          m_pfn;
    void*               m_args;
    pthread_cond_t      m_cond;
    CPoolThread*        m_thPool;        
};

class CPoolThread
{
public:
    friend class CThreadInfo;
    CPoolThread(FN_Routine fn, int sizeArgs, int maxThreads = 0);
	~CPoolThread(void);

    int                 Lock();
    int                 Unlock();
    int                 Wait(pthread_cond_t* cond = NULL);
    int                 Signal(pthread_cond_t* cond = NULL);
    int                 GetMaxThreads();
    CThreadInfo*        GetQueue();
    void                ShowInfo();

protected:
    std::queue<CThreadInfo*>        m_queue;
    pthread_mutex_t                 m_mutex;
    pthread_cond_t                  m_cond;
    int                             m_maxThreads;
    FN_Routine                      m_pfnRoutine;
};