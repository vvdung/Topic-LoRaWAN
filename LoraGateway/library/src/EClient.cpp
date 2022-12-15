#include "../inc/HFactor.h"
#include "../inc/EClient.h"

int CEClient::_epollClient = -1;
int	CEClient::_epollfdExit = -1;
pthread_t CEClient::_uThreadID = 0;

CEClient::CEClient():CESocket(){
    m_State = NONE;
    m_uNextReceived = MAX_PACKAGE_SIZE;
    m_uTotalRecv = 0;

    m_pSendBuffer = NULL;
    m_uTotalSend = 0;
}
CEClient::~CEClient(){
    m_State = NONE;
}

void CEClient::OnCreated(){
    m_uStarted = GetTickCount();
    m_State = CREATED;
    m_uNextReceived = MAX_PACKAGE_SIZE;
    m_uTotalRecv = 0;
    m_pSendBuffer = NULL;
    m_uTotalSend = 0;

/*	int         nZero = 0;      
	setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));*/  
/*    nZero = 0;   
	setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));*/  
/*	linger      lingerStruct;
    lingerStruct.l_onoff = 0;   
	lingerStruct.l_linger = 0;
	setsockopt(m_hSocket, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct));*/  
}
void CEClient::OnConnected(){
    m_uTTL = GetTickCount();    
    m_State = CONNECTED;

    struct sockaddr_in peeraddr; 
    memset(&peeraddr,0, sizeof(peeraddr)); 
    socklen_t clilen = sizeof(struct sockaddr_in);        
    getpeername(m_hSocket,reinterpret_cast<sockaddr*>(&peeraddr), &clilen); 
    strcpy(m_czRemoteIP,inet_ntoa(peeraddr.sin_addr));
    m_uRemotePort = ntohs(peeraddr.sin_port);   
    //AddTimerLoop();
}

void CEClient::OnReceived(uint8_t* pBuffer,int uSize){
    m_uTTL = GetTickCount();
}
void CEClient::OnSend(int uSize){
    m_uTTL = GetTickCount();
}
void CEClient::OnClosed(){     
    m_State = DISCONNECTED;   
}
void CEClient::OnTimerRoutine(){
    CESocket::OnTimerRoutine();
    
}

bool CEClient::Connect(const char* lpszHostAddress, uint32_t nHostPort){    
    if (lpszHostAddress == NULL) return false;
    SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	char* lpszAscii = (char*)lpszHostAddress;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		struct hostent * lphost;
		lphost = gethostbyname(lpszAscii);
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((struct in_addr*)lphost->h_addr)->s_addr;
		else{
            Close();
            return false;
        } 		
	}

	sockAddr.sin_port = htons((u_short)nHostPort);
    return Connect((SOCKADDR*)&sockAddr,sizeof(sockAddr));
}
bool CEClient::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen){
    uint32_t flags = EPOLLIN | EPOLLET | EPOLLRDHUP;// | EPOLLONESHOT;
    if (connect(m_hSocket,lpSockAddr,nSockAddrLen) == -1){
        if (errno == EINPROGRESS){
            flags |= EPOLLOUT;
        }
        else{
            printf("[?] CEClient::Connect() err:%d (%s)\n",errno,strerror(errno));
            Close();
            return false;
        } 
        Add(flags);       
    } 
    else{
        Add(flags);
        OnConnected(); 
    }   
    //printf("[*] CEClient::Connect() err:%d (%s)\n",errno,strerror(errno));    
    return true; 
}
bool CEClient::Send(char *pBuffer,uint32_t uSize, bool bFlush){
    if (uSize == 0 || m_hSocket == -1) return false;
    m_csSend.lock();

	int iNeed = (int)ceil((float)uSize/MAX_PACKAGE_SIZE);
	uint32_t iLen = uSize;
	uint32_t iPos = 0;
	for (int i = 0; i < iNeed; ++i)
	{
		SENDBUFFER* p = m_lstSendBuffer.New();	
		if (!p){
			printf("??? p = NULL ??? CESocket::Send(%d)\n",uSize);
            m_csSend.unlock();
            return false;
		}
		if (iLen >= MAX_PACKAGE_SIZE)
		{			
			p->uSize = MAX_PACKAGE_SIZE;
			iLen = iLen - MAX_PACKAGE_SIZE;
		}
		else p->uSize = iLen;
		p->uSend = 0;
		memcpy((void*)p->pBytes,(const void*)&pBuffer[iPos],p->uSize);
		iPos += p->uSize;		
	}

    m_csSend.unlock();    
	if (!bFlush) return true;     
    Mod(EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP);
    return true;
}
bool CEClient::Flush(){
    if (m_hSocket == -1) return false;

    m_csSend.lock();
	if (m_pSendBuffer){
		if (m_pSendBuffer->uSend >= m_pSendBuffer->uSize){
			m_lstSendBuffer.AddFree(m_pSendBuffer);			
			m_pSendBuffer = m_lstSendBuffer.RemoveHeadUsed();
		}
	}
	else m_pSendBuffer = m_lstSendBuffer.RemoveHeadUsed();
    SENDBUFFER* p = m_pSendBuffer;
	if (p != NULL)
	{	
        int nBytes = write(m_hSocket,&p->pBytes[p->uSend],p->uSize - p->uSend);   				
        if (nBytes > 0){
            p->uSend += nBytes;
            m_uTotalSend += nBytes;  
            m_csSend.unlock();
            OnSend(nBytes);
        } 
        else m_csSend.unlock(); 
        Mod(EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP); 
    }
    else{
        m_csSend.unlock(); 
    }    
    return true;
}
void CEClient::Initialization(){
    if (_epollClient != -1) return;

    _epollfdExit = eventfd(0, EFD_NONBLOCK);
	if (_epollfdExit == -1){
        printf("[?] CEClient::Initialize() eventfd() err:%d (%s)\n",errno,strerror(errno));
		return;
	}
    _epollClient = epoll_create1(0);
    if (_epollClient == -1){
        printf("[?] CEClient::Initialize() epoll_create1() err:%d (%s)\n",errno,strerror(errno));
        return;
    }
    bool bWaiting = true;
    if (pthread_create(&_uThreadID,NULL,_ThreadEpollClient,&bWaiting) == 0) {
        for (int i = 0; i < 10; ++i){
            if (!bWaiting) break;
            Sleep(10);        
        }
    }

}
void CEClient::Exitialization(){
    if (_epollClient == -1) return;    
	uint64_t u = MAKE64(1,10);
	write(_epollfdExit,&u,sizeof(uint64_t));
	if (_uThreadID != pthread_self()){
		pthread_join(_uThreadID, NULL);
	}	    
    close(_epollfdExit);
    close(_epollClient);

}
void* CEClient::_ThreadEpollClient(void* lpData){
    bool* pWaiting = (bool*)lpData;
    printf("[*] CEClient::Initialize() STARTED\n");
    struct epoll_event ev;
	ev.events = EPOLLIN;
    ev.data.fd = _epollfdExit;
    if (epoll_ctl(_epollClient, EPOLL_CTL_ADD, _epollfdExit, &ev) == -1){
        printf("[?] CEClient::Initialize() epoll_ctl() err:%d (%s)\n",errno,strerror(errno));
    }
    CPoolThread* pthPool = GetPoolThread();
    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(events) * 1024);
    int iRet;
    uint64_t u;
    bool bLoop = true;
    CEClient* pClient;
    *pWaiting = false;
    while (bLoop){
        iRet = epoll_wait(_epollClient,events, 1024, -1);		
		if (iRet == -1){
			if (errno == EINTR) continue;
			break;
		}
		else if (iRet == 0){
			continue;
		}
		for (int i = 0; i < iRet; ++i)
		{
			if (events[i].data.fd == _epollfdExit){				
				read(_epollfdExit,&u,sizeof(uint64_t));
				bLoop = false;
				break;
			}	
            pClient = (CEClient*)events[i].data.ptr;
            if (!pClient) continue;
            
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP || events[i].events & EPOLLRDHUP)
			{
                             
                pClient->RemTimer();
                pClient->Del();
                pClient->OnClosed();            
                continue;
			}         

            if (events[i].events & EPOLLIN){                
                pthPool->AddTask(_ProcessEpollin,pClient,pClient);
            }
            else if (events[i].events & EPOLLOUT){
                pthPool->AddTask(_ProcessEpollout,pClient,pClient);
            }
            else {
                printf("STATUS [%X] CLIENT=%d\n",events[i].events,pClient->GetSocket());
            } 

        }//for (int i = 0; i < iRet; ++i)

    }//while (1){

    printf("[*] CEClient::Initialize() EXIT\n");

    pthread_exit(NULL);
    return NULL;
}
void CEClient::_ProcessEpollin(void* arg,void* user){
    if (!arg) return;
    CEClient* pClient = (CEClient*)arg;    
    if (pClient->m_State == CREATED) pClient->OnConnected();    
    pClient->OnEpollin();    
}
void CEClient::_ProcessEpollout(void* arg,void* user){
    if (!arg) return;
    CEClient* pClient = (CEClient*)arg;  
    if (pClient->m_State == CREATED) pClient->OnConnected();  
    pClient->OnEpollout();
}
void CEClient::OnEpollin(){
    if (m_uNextReceived <= 0 || m_uNextReceived > MAX_PACKAGE_SIZE) m_uNextReceived = MAX_PACKAGE_SIZE;
    while (1){
        int iSize = read(m_hSocket,m_recvBuffer,m_uNextReceived);
        if (iSize > 0){
            m_uTotalRecv += iSize;
            OnReceived(m_recvBuffer,iSize);
        } 
        else break;
    } 
    uint32_t flags = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (!m_lstSendBuffer.m_lstUsed.empty()) flags |= EPOLLOUT;
    Mod(flags);
}
void CEClient::OnEpollout(){
    Flush();  
}

bool CEClient::Add(uint32_t state){
    struct epoll_event ev;
	ev.events = state;
    ev.data.ptr = this;
	int iRet = epoll_ctl(_epollClient, EPOLL_CTL_ADD, GetSocket(), &ev);
	if (iRet == -1){
        printf("[?] CEClient::Add() err:%d (%s)\n",errno,strerror(errno));
		return false;
	}	
	return true;
}
bool CEClient::Mod(uint32_t state){
    struct epoll_event ev;
	ev.events = state;
    ev.data.ptr = this;
	if (epoll_ctl(_epollClient, EPOLL_CTL_MOD, GetSocket(), &ev) == -1)
	{
        printf("[?] CEClient::Mod() err:%d (%s)\n",errno,strerror(errno));
		return false;
	}	
    return true;
}
bool CEClient::Del(){
    struct epoll_event ev;
	memset(&ev,0,sizeof(ev));
	int iRet = epoll_ctl(_epollClient, EPOLL_CTL_DEL, GetSocket(), &ev);
	if (iRet == -1){
        printf("[?] CEClient::Del() err:%d (%s)\n",errno,strerror(errno));
		return false;
	}	
	return true;
}

