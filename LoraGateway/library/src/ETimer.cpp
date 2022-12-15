#include "../inc/HFactor.h"
#include "ETimer.h"


CETimer::CETimer(){
    m_tid = NULL;
}
CETimer::~CETimer(){
    RemTimer();
}

void CETimer::_handler(int sig, siginfo_t *si, void *data){
    if (si->si_errno != 0){
        printf("ERROR _handler() %d code:%d\n",si->si_errno,si->si_code);
        return;
    }
    CETimer* pThis = (CETimer*)si->si_ptr;
    if (!pThis){
        printf("ERROR _handler() %d code:%d pThis=NULL\n",si->si_errno,si->si_code);
        return;
    }
    pThis->OnTimer();

}


bool CETimer::CreateTimer(timer_t* pTid,FN_TIMER_CB fnHander,void* arg,uint32_t dwPeriod,uint32_t interval){
    static const int SIG = SIGRTMIN;
     
    if (!pTid) return false;
    if (*pTid) timer_delete(*pTid);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = fnHander;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1){
        printf("Error sigaction() : %d\n",errno);
        return false;
    }
    
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = arg;
    if (timer_create(CLOCK_REALTIME, &sev, pTid) == -1){
        printf("Error timer_create() : %d\n",errno);
        return false;        
    }    

    /* Start the timer */

    uint64_t value_nanosecs = (uint64_t)dwPeriod * 1000000; //mili to nano
    uint64_t interval_nanosecs = (uint64_t)interval * 1000000; //mili to nano
    struct itimerspec its;    
    its.it_value.tv_sec = value_nanosecs / 1000000000;
    its.it_value.tv_nsec = value_nanosecs % 1000000000;
    its.it_interval.tv_sec = interval_nanosecs /  1000000000;
    its.it_interval.tv_nsec = interval_nanosecs % 1000000000;

    if (timer_settime(*pTid, 0, &its, NULL) == -1){
        printf("Error timer_settime() : %d\n",errno);
        return false;  
    }
    return true;
}
bool CETimer::AddTimerOnce(uint32_t dwPeriod){
    return CreateTimer(&m_tid,_handler,this,dwPeriod,0);
}
bool CETimer::AddTimerLoop(uint32_t dwMiliseconds, uint32_t dwPeriod){
    return CreateTimer(&m_tid,_handler,this,dwPeriod,dwMiliseconds);
}
void CETimer::RemTimer(){
    if (!m_tid) return;
    if (timer_delete(m_tid) == -1){
        printf("??? timer_delete err:%d (%s) \n",errno,strerror(errno));
    }    
    m_tid = NULL;    
}