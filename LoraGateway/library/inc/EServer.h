#pragma once
#include "EQueueConnect.h"
#include <sys/epoll.h>

class CEServer : public CESocket{
public:
    friend class CEConnect;
    CEServer();
    virtual ~CEServer();
    void	Start();
    void    Exit();
protected:
    static void     _ProcessEpollin(void* arg,void* user);
    static void     _ProcessEpollout(void* arg,void* user);
    static void     _ProcessAccept(void* arg,void* user);
    static void     _ProcessDisconnect(void* arg,void* user);

	static void*    _ThreadEpollRoutine(void* lpData);
	void	        OnThreadEpollRoutine(); 
    virtual bool    OnInit() = 0;
    virtual bool    OnExit() = 0;
    virtual void    OnAccept() = 0;
    virtual void    OnDisconnect(CEConnect* p) = 0;
    
    virtual void 	OnPreBind();
    virtual void	OnCreated();

	bool	        Add(CESocket* pSocket, uint32_t state);
	bool	        Mod(CESocket* pSocket, uint32_t state);
    bool	        Del(CESocket* pSocket);


    pthread_t	    m_uThreadID;
    CPoolThread*    m_thPool;	
    int			    m_epoll;
    CEQueueConnect  m_queueConnect;
    //std::queue<CEConnect*>   m_queueSocket;
};