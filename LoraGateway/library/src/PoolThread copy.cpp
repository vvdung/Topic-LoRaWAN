
#include "../inc/HFactor.h"
#include "../inc/PoolThread.h"
//#include <unistd.h>



CThreadInfo::CThreadInfo(CPoolThread* thPool){
    m_uTID = 0;
    m_args = NULL;
    m_pfn = NULL;
    m_cond = PTHREAD_COND_INITIALIZER;
    m_thPool = thPool;
}

CThreadInfo::~CThreadInfo(){
    if (m_args) free(m_args);
}
bool CThreadInfo::Create(){
    pthread_attr_t tAttr;		
	pthread_attr_init(&tAttr);
	pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
    int iRes = pthread_create(&m_uTID, &tAttr, _ThreadRoutine, this);
    if (iRes == 0) return true;
    return false;
}
pthread_t CThreadInfo::GetThreadId(){
    return m_uTID;
}
void* CThreadInfo::GetArgs(){
    return m_args;
}
void CThreadInfo::RunRoutine(){  
    m_thPool->Signal(&m_cond);
}
void* CThreadInfo::_ThreadRoutine(void* lpData){
    CThreadInfo* pthInfo = (CThreadInfo*)lpData;
    pthInfo->OnThreadRoutine();
    return NULL;
}
void CThreadInfo::OnThreadRoutine(){
   
    while (1){        	                    
        m_thPool->Lock();  
        
        m_thPool->m_queue.push(this);
        m_thPool->Signal();

        m_thPool->Wait(&m_cond);        	    
        if (m_pfn) m_pfn(this);     
	    m_thPool->Unlock(); 
    }
    
} 
////////CPoolThread//////////////////////////////////

CPoolThread::CPoolThread(FN_Routine fn,int sizeArgs, int maxThreads){
    m_cond  = PTHREAD_COND_INITIALIZER;
    m_mutex = PTHREAD_MUTEX_INITIALIZER;    
    int iCPUs = get_nprocs_conf();
    m_maxThreads = maxThreads;
    if (m_maxThreads == 0) m_maxThreads = iCPUs * 5;
    //printf("CPUs:%d PoolThreads:%d\n",iCPUs,m_maxThreads);
    CThreadInfo* p = NULL;
    for (int i = 0; i < m_maxThreads; ++i){
        p = new CThreadInfo(this);
        if (p->Create()){
            p->m_pfn = fn;
            p->m_args = malloc(sizeArgs);
        }
        else{
            delete p;            
        }        
    }
    while ((int)m_queue.size() != m_maxThreads) usleep(1000);
}

CPoolThread::~CPoolThread(void){
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
        
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
int CPoolThread::GetMaxThreads(){
    return m_maxThreads;
}
CThreadInfo* CPoolThread::GetQueue(){
    CThreadInfo* p = NULL;
    Lock();
    while (m_queue.empty()) Wait();
    p = m_queue.front();
    m_queue.pop();
    Unlock();     
    return p;
}

void CPoolThread::ShowInfo(){
    Lock();
    printf("\n---------------------\n");
    printf("maxThreads[%d] Queue[%lu]\n",m_maxThreads,m_queue.size());
    printf("\n---------------------\n");
    Unlock();
}