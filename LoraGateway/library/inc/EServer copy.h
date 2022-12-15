#pragma once
#include "ESocket.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>

#define MAKE64(h,l) ((uint64_t) (((uint64_t)h << 32) | (uint64_t)l))
#define HIGH32(v) ((int)((uint64_t)v >> 32))
#define LOW32(v) ((int)v)

class CEServer;
class CEConnect;
typedef struct _SHOTDATA{
    CEConnect*      pConnect;
    CEServer*       pServer;
}SHOTDATA;

class CEServer : public CESocket{
public:
    CEServer();
    virtual ~CEServer();
    void	Start();
protected:
    static bool      _ProcessOneShot(void* th);
	static void*    _ThreadEpollRoutine(void* lpData);
	void	        OnThreadEpollRoutine(); 
    virtual bool    OnInit() = 0;
    virtual bool    OnExit() = 0;
    virtual CEConnect*  OnAccept() = 0;
    virtual void        OnDisconnect(CEConnect* p) = 0;
    virtual bool    OnEpollOneShot(CEConnect* p);
    virtual void 	OnPreBind();
    virtual void	OnCreated();

	bool	        Add(CESocket* pSocket, uint32_t state);
	bool	        Mod(CESocket* pSocket, uint32_t state);
    bool	        Del(CESocket* pSocket);


    pthread_t	    m_uThreadID;
    CPoolThread*    m_thPool;	
    int			    m_epoll;
    

    //std::map<int,CEConnect*> m_mapSocket;
    std::queue<CEConnect*>   m_queueSocket;
};