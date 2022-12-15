#pragma once
#include "ESocket.h"

class CEClient : public CESocket
{
public:
    enum State
	{
		NONE,
		CREATED,
		CONNECTED,
		DISCONNECTED
	};

    CEClient();
    virtual ~CEClient();
    static void     Initialization();
    static void     Exitialization();

    bool            Connect(const char* lpszHostAddress, uint32_t nHostPort);
	bool            Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
    bool			Send(char *pBuffer,uint32_t uSize, bool bFlush = true);
	bool			Flush();			// write data to network
protected:    
    virtual void	OnCreated();
    virtual void    OnConnected();
	virtual void	OnReceived(uint8_t* pBuffer,int uSize);
	virtual void	OnSend(int uSize);
	virtual void	OnClosed();
	virtual void	OnTimerRoutine();  


	bool	        Add(uint32_t state);
	bool	        Mod(uint32_t state);
    bool	        Del();

    uint8_t		    m_recvBuffer[MAX_PACKAGE_SIZE];
    uint32_t	    m_uNextReceived;
    uint64_t        m_uTotalRecv;

 	SENDBUFFER*	    m_pSendBuffer;		//buffer sending	
	uint64_t	    m_uTotalSend;
    
    char            m_czRemoteIP[16];//xxx.xxx.xxx.xxx
    int             m_uRemotePort;    
    State           m_State;
    
private:  
    static void         _ProcessEpollin(void* arg,void* user);
    static void         _ProcessEpollout(void* arg,void* user);
    static void*        _ThreadEpollClient(void* lpData);
    void                OnEpollin();
    void                OnEpollout();

    static int	        _epollClient;
    static int		    _epollfdExit;
    static pthread_t    _uThreadID;
};