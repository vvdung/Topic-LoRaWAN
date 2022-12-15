#pragma once
#include "ESocket.h"
class CEServer;

class CEConnect : public CESocket
{
public:
	friend class CEServer;
	enum State
	{
		NONE,
		CREATED,
		ACCEPTED,
		DISCONNECTED
	};
    CEConnect();
    virtual ~CEConnect();
	void 			SetServer(CEServer* p);
    bool			Send(char *pBuffer,uint32_t uSize, bool bFlush = true);
	bool			Flush();			// write data to network

protected:
	virtual void 	OnPreBind();
    virtual void	OnCreated();
    virtual void    OnConnected();
	virtual void	OnReceived(uint8_t* pBuffer,int uSize);
	virtual void	OnSend(int uSize);
	virtual void	OnClosed();
	virtual void	OnTimerRoutine();
    void            OnEpollin();
    void            OnEpollout();

    uint8_t		    m_recvBuffer[MAX_PACKAGE_SIZE];
    uint32_t	    m_uNextReceived;
    uint64_t        m_uTotalRecv;

 	SENDBUFFER*	    m_pSendBuffer;		//buffer sending	
	uint64_t	    m_uTotalSend;
    
    char            m_czRemoteIP[16];//xxx.xxx.xxx.xxx
    int             m_uRemotePort;    
    State           m_State;
	CEServer*		m_pServer;
};