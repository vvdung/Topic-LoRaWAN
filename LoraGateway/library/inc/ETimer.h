#pragma once
#include <signal.h>
#include <time.h>

typedef void (*FN_TIMER_CB)(int sig, siginfo_t *si, void *data);

class CETimer{
public:
    CETimer();
    virtual ~CETimer();

    static bool     CreateTimer(timer_t* tid,FN_TIMER_CB,void* arg,uint32_t dwPeriod,uint32_t interval);//ms
    bool            AddTimerOnce(uint32_t dwPeriod = 10000);//miliseconds
    bool		    AddTimerLoop(uint32_t dwMiliseconds = 1000, uint32_t dwPeriod = 10);
    void            RemTimer();
protected:
    static void     _handler(int sig, siginfo_t *si, void *data);
    virtual void    OnTimer() = 0;
    timer_t         m_tid;

};