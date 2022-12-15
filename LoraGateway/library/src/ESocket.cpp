#include "../inc/HFactor.h"
#include "../inc/ESocket.h"

CBufferList::CBufferList(){}
CBufferList::~CBufferList(){}

void CBufferList::Lock()
{
	m_mutex.lock();
}

void CBufferList::Unlock()
{
	m_mutex.unlock();
}

bool CBufferList::IsEmpty()
{
	return this->empty();
}

SENDBUFFER* CBufferList::RemoveHead()
{
	SENDBUFFER* p = this->front();
	this->pop_front();
	return p;
}

void CBufferList::AddTail(SENDBUFFER* p)
{
	Lock();
	this->push_back(p);
	Unlock();
}

void CBufferList::AddHead(SENDBUFFER* p)
{
	Lock();
	this->push_front(p);
	Unlock();
}

void CBufferList::Remove(SENDBUFFER* p)
{
	Lock();
	this->remove(p);
	Unlock();
}
//////////CReuseSendBuffer//////////////////////////////
CReuseSendBuffer::CReuseSendBuffer()
{

}

CReuseSendBuffer::~CReuseSendBuffer()
{
	DestroyFree();
	DestroyUsed();
}

SENDBUFFER* CReuseSendBuffer::New()
{
	SENDBUFFER* p = NULL;
	m_lstFree.Lock();
	if (!m_lstFree.IsEmpty()) p = m_lstFree.RemoveHead();
	else p = (SENDBUFFER*)malloc(sizeof(SENDBUFFER));
	m_lstFree.Unlock();

	AddUse(p);
	return p;
}

void CReuseSendBuffer::AddUse( SENDBUFFER* p )
{
	m_lstUsed.AddTail(p);
}

void CReuseSendBuffer::AddFree( SENDBUFFER* p )
{
	m_lstFree.Lock();    
	if (m_lstFree.size() < 5) m_lstFree.push_back(p);
	else free(p);	
	m_lstFree.Unlock();
}

void CReuseSendBuffer::RemoveUse( SENDBUFFER* p, bool bDelete)
{
	if (!p) return;
	m_lstUsed.Remove(p);
	if (!bDelete) AddFree(p);
	else free(p);
}

SENDBUFFER* CReuseSendBuffer::RemoveHeadUsed()
{
	SENDBUFFER* p = NULL;
	m_lstUsed.Lock();
	if (!m_lstUsed.IsEmpty()) p = m_lstUsed.RemoveHead();
	m_lstUsed.Unlock();
	return p;
}

void CReuseSendBuffer::DestroyUsed()
{
	m_lstUsed.Lock();
	while (!m_lstUsed.IsEmpty())
	{
		SENDBUFFER* p = m_lstUsed.RemoveHead();
		free(p);
	}
	m_lstUsed.Unlock();
}

void CReuseSendBuffer::DestroyFree()
{
	m_lstFree.Lock();
	while (!m_lstFree.IsEmpty())
	{
		SENDBUFFER* p = m_lstFree.RemoveHead();
		free(p);
	}
	m_lstFree.Unlock();
}



//////////CESocket//////////////////////////////
CESocket::CESocket(void)
{
	m_hSocket = INVALID_SOCKET;
	m_uPortBind = 0;
	m_uStarted = 0;
	m_uTTL = 0;
    m_hTimer = NULL;
}

CESocket::~CESocket(void)
{
	if (m_hSocket != INVALID_SOCKET) close(m_hSocket);			
	m_hSocket = INVALID_SOCKET;
	m_uPortBind = 0;
    RemTimer();
}
bool CESocket::SetNonblocking(SOCKET sock){
    int old = fcntl(sock, F_GETFD, 0);
    if (old == INVALID_SOCKET) return false;
    if (fcntl(sock, F_SETFL, old | O_NONBLOCK) == INVALID_SOCKET) return false;    
    return true;
}

bool CESocket::Socket(int nSocketType,int nAddressFormat,int nProtocolType){
    m_hSocket = socket(nAddressFormat, nSocketType, nProtocolType);
    return (m_hSocket != SOCKET_ERROR);
}
bool CESocket::Bind(uint32_t nSocketPort, const char* lpszSocketAddress){
    SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	if (lpszSocketAddress == NULL)
		sockAddr.sin_addr.s_addr = INADDR_ANY;//htonl(INADDR_ANY);
	else
	{
		uint32_t lResult = inet_addr(lpszSocketAddress);
		if (lResult == INADDR_NONE) return false;
		sockAddr.sin_addr.s_addr = lResult;
	}
	sockAddr.sin_port = htons((u_short)nSocketPort);
	return Bind((struct sockaddr*)&sockAddr, sizeof(sockAddr));
    
}
bool CESocket::Bind (const SOCKADDR* lpSockAddr, int nSockAddrLen){
    return (bind(m_hSocket,lpSockAddr, nSockAddrLen) != SOCKET_ERROR);
}
bool CESocket::Listen(int nConnectionBacklog){
    return (listen(m_hSocket, nConnectionBacklog) != SOCKET_ERROR);
}
void CESocket::OnPreBind(){

}
bool CESocket::Create(int nSocketType, uint32_t nSocketPort,const char* lpszSocketAddress){
    if (!Socket(nSocketType, AF_INET, 0)) return false;    
    if (!SetNonblocking(m_hSocket)) return false;
	OnPreBind();
    if (Bind(nSocketPort,lpszSocketAddress)){
        sockaddr_in addr; 
		memset(&addr,0, sizeof(addr)); 
		unsigned int size = sizeof(addr);
		getsockname(m_hSocket,reinterpret_cast<sockaddr*>(&addr), &size);
		m_uPortBind = ntohs(addr.sin_port);
        
        OnCreated();
        return true;
    }
    Close();
    return false;
}
void CESocket::Close(){ 
    if (m_hSocket != INVALID_SOCKET){
        close(m_hSocket);
    }     
	m_hSocket = INVALID_SOCKET;
}
void CESocket::_SignalTimer(int sig, siginfo_t *si, void *data){
    if (si->si_errno != 0){
        printf("ERROR _SignalTimer() %d code:%d\n",si->si_errno,si->si_code);
        return;
    }
    CESocket* pSocket = (CESocket*)si->si_ptr;
    if (!pSocket){
        printf("ERROR _SignalTimer() %d code:%d pThis=NULL\n",si->si_errno,si->si_code);
        return;
    }
    pSocket->OnTimerRoutine();
}
void CESocket::OnTimerRoutine()
{
    //printf("... CESocket::OnTimerRoutine() \n");
}
bool CESocket::AddTimerOnce(uint32_t dwPeriod){
    return CETimer::CreateTimer(&m_hTimer,_SignalTimer,this,dwPeriod,0);
}
bool CESocket::AddTimerLoop(uint32_t dwMiliseconds, uint32_t dwPeriod){
    return CETimer::CreateTimer(&m_hTimer,_SignalTimer,this,dwPeriod,dwMiliseconds);
}
void CESocket::RemTimer(){
    if (m_hTimer == NULL) return;
    if (timer_delete(m_hTimer) == -1){
        printf("??? timer_delete(%d) [%p] err:%d (%s) \n",m_hSocket,m_hTimer,errno,strerror(errno));
    }
    m_hTimer = NULL; 
}
SOCKET CESocket::GetSocket(){
    return m_hSocket;
}
