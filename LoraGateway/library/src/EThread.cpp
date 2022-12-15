#include "../inc/HFactor.h"
#include "../inc/EThread.h"

CEThread::CEThread(void){
    m_done = false;
    m_dwDelay = 100;
    m_uThreadID = 0;

}
CEThread::~CEThread(void){

}
void CEThread::Start(uint32_t dwDelay){
	if (m_uThreadID != 0) return;
	m_dwDelay = dwDelay;
	pthread_create(&m_uThreadID,NULL,_ThreadEpollRoutine,this);
}
void CEThread::Exit(){
    if (m_uThreadID == 0) return;
    m_done = true;
    if (m_uThreadID != pthread_self()) pthread_join(m_uThreadID,NULL); 
}
void CEThread::NextDelay(uint32_t dwDelay){
    m_dwDelay = dwDelay;
}
void CEThread::WaitCompleted(){
    if (m_uThreadID == 0) return;
    if (m_uThreadID != pthread_self()) pthread_join(m_uThreadID,NULL);    
}
void* CEThread::_ThreadEpollRoutine(void* lpData){
    CEThread* p = (CEThread*)lpData;
    p->OnThreadEpollRoutine();
    return NULL;
}
void CEThread::OnThreadEpollRoutine(){    
    InitInstance();
    while (!m_done){                
        Run();
        Sleep(m_dwDelay);
    }
    ExitInstance();
}