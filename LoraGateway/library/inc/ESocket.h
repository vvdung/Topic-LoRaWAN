#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <list>

#define MAX_PACKAGE_SIZE    10240 //(10*1024)
typedef int                 SOCKET;
#define INVALID_SOCKET		(int)(~0)
#define SOCKET_ERROR		(-1)
#define SOCKADDR_IN			sockaddr_in
#define SOCKADDR			sockaddr

typedef struct _SendBuffer
{
	uint8_t		pBytes[MAX_PACKAGE_SIZE];
	uint32_t	uSize;
	uint32_t	uSend;
}SENDBUFFER;

class CBufferList: public std::list<SENDBUFFER*>
{
public:
	CBufferList();
	~CBufferList();
	void			Lock();
	void			Unlock();
	bool			IsEmpty();
	SENDBUFFER*		RemoveHead();
	void			AddTail(SENDBUFFER* p);
	void			AddHead(SENDBUFFER* p);
	void			Remove(SENDBUFFER* p);	
protected:	
	std::mutex 		m_mutex;
};

class CReuseSendBuffer{
public:
	CReuseSendBuffer();
	~CReuseSendBuffer();

	SENDBUFFER*		New();
	void			AddUse(SENDBUFFER* p);
	void			AddFree(SENDBUFFER* p);
	void			RemoveUse(SENDBUFFER* p, bool bDelete = false);
	void			DestroyUsed();	//xoa toan bo m_lstUsed
	void			DestroyFree();	//xoa toan bo m_lstFree
	SENDBUFFER*		RemoveHeadUsed();

	CBufferList		m_lstUsed;	//danh sach dang su dung
	CBufferList		m_lstFree;	//danh sach free

};

class CESocket
{
public:

    CESocket(void);
	virtual ~CESocket(void);
    static bool    SetNonblocking(SOCKET sock);

    bool    Socket(int nSocketType = SOCK_STREAM,int nAddressFormat = AF_INET,int nProtocolType = 0);
    bool    Bind(uint32_t nSocketPort, const char* lpszSocketAddress = NULL);
	bool    Bind(const SOCKADDR* lpSockAddr, int nSockAddrLen);
    void    Close();
    bool    Listen(int nConnectionBacklog = 1024);
    bool    Create(int nSocketType = SOCK_STREAM,uint32_t nSocketPort = 0,const char* lpszSocketAddress = NULL);

	bool	AddTimerOnce(uint32_t dwPeriod = 10000);
	bool	AddTimerLoop(uint32_t dwMiliseconds = 1000, uint32_t dwPeriod = 10);//10ms
	void	RemTimer();

    SOCKET  GetSocket();
	
protected:
    static void     _SignalTimer(int sig, siginfo_t *si, void *data);
    virtual void 	OnPreBind();
	virtual void	OnCreated() = 0;
	virtual void	OnTimerRoutine();
    
	SOCKET		m_hSocket;
	uint32_t	m_uPortBind;
	
	uint64_t	m_uStarted;
	uint64_t	m_uTTL;

	timer_t		m_hTimer;
	std::mutex  m_csSend;

	CReuseSendBuffer		m_lstSendBuffer;	//danh sach dang cho send
};