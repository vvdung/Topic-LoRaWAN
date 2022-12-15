#include "../inc/HFactor.h"
#include "../inc/EServer.h"

///////CQueueConnect////////////////////////////////////  


///////CEServer////////////////////////////////////    
CEServer::CEServer():CESocket(){
	m_uThreadID = 0;
	m_epoll = -1;
    m_thPool = GetPoolThread();//new CPoolThread();
}
CEServer::~CEServer(){
    //delete m_thPool;
}
void CEServer::OnPreBind(){
    CESocket::OnPreBind();
    int reuse = 1; 
    if (setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR , (const char*)&reuse, sizeof(reuse)) < 0)
    {printf("[-] CEServer::OnCreated() setsockopt(SO_REUSEADDR) failed\n");} 
}
void CEServer::OnCreated(){
    /*int reuse = 1;
    if (setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
    {printf("[-] CEServer::OnCreated() setsockopt(SO_REUSEADDR) failed\n");} */   
}

void CEServer::Start(){
    if (m_uThreadID != 0) return;
	pthread_create(&m_uThreadID,NULL,_ThreadEpollRoutine,this);
}
void CEServer::Exit(){
    if (m_uThreadID == 0) return;
    shutdown(m_hSocket,SHUT_RDWR);
    if (m_uThreadID == pthread_self()) return;
    pthread_join(m_uThreadID,NULL);
}

void* CEServer::_ThreadEpollRoutine(void* lpData){
    CEServer* pServer = (CEServer*)lpData;
    pServer->OnThreadEpollRoutine();
    return NULL;
}
void CEServer::OnThreadEpollRoutine(){

    if (!OnInit()) return;   

    m_epoll = epoll_create1(0);
	if (m_epoll == -1){
		printf("epoll_create() error:%d \n",errno);
        OnExit();
        pthread_exit(NULL);
		return;
	}

    Add(this,EPOLLIN | EPOLLET);
    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(events) * 1024);

    CEConnect* pConnect;
    int iRet;
    bool bLoop = true;

    while (bLoop){
        iRet = epoll_wait(m_epoll,events, 1024, -1);		
		if (iRet == -1){
			if (errno == EINTR) continue;
			break;
		}
		else if (iRet == 0){
			continue;
		}

        for (int i = 0; i < iRet; ++i)
		{
            pConnect   = (CEConnect*)events[i].data.ptr;
            if (!pConnect) continue;

            int hSocket = pConnect->GetSocket();
			if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP || events[i].events & EPOLLRDHUP)
			{
                if (hSocket == this->m_hSocket){
                    //printf(" SERVER EVENT %d SOCKET:%d Events:%X iRet:%d\n",i,hSocket,events[i].events,iRet);				
                    bLoop = false;
                    break;
                }                
                
                pConnect->RemTimer();
                this->Del(pConnect);
                _ProcessDisconnect(pConnect,this);
                //m_thPool->AddTask(_ProcessDisconnect,pConnect,this);             
                continue;
			}

            if (hSocket == this->m_hSocket){                
                _ProcessAccept(NULL,this);
                //m_thPool->AddTask(_ProcessAccept,NULL,this);
                continue;                
            }
            
            if (events[i].events & EPOLLIN){
                m_thPool->AddTask(_ProcessEpollin,pConnect,this);
            }
            else if (events[i].events & EPOLLOUT){
                m_thPool->AddTask(_ProcessEpollout,pConnect,this);
            }
            else {
                printf("STATUS [%X] CONNECT=%d\n",events[i].events,hSocket);
            }                                      
        }//for (int i = 0; i < iRet; ++i)
    }//while (bLoop){
    OnExit();
    free(events);
    close(m_epoll);
    pthread_exit(NULL);
}

void CEServer::_ProcessEpollin(void* arg,void* user){
    if (!user || !arg) return;
    CEConnect* pConnect = (CEConnect*)arg;
    pConnect->OnEpollin();   
}
void CEServer::_ProcessEpollout(void* arg,void* user){
    if (!user || !arg) return;
    CEConnect* pConnect = (CEConnect*)arg;
    pConnect->OnEpollout();  
}
void CEServer::_ProcessAccept(void* arg,void* user){
    if (!user) return;
    CEServer* pServer = (CEServer*)user;
    //pServer->m_queueConnect.Lock();
    pServer->OnAccept();
    //pServer->m_queueConnect.Unlock();
}
void CEServer::_ProcessDisconnect(void* arg,void* user){
    if (!user || !arg) return;
    CEConnect* pConnect = (CEConnect*)arg;
    CEServer* pServer = (CEServer*)user;
    pServer->OnDisconnect(pConnect);	
    //pServer->Del(pConnect);		
    pConnect->OnClosed();	
    //pServer->m_queueConnect.Lock();
    pServer->m_queueConnect.Push(pConnect);
    //if (!pServer->m_queueConnect.Push(pConnect)) delete pConnect;
    //pServer->m_queueConnect.Unlock();
}
bool CEServer::Add(CESocket* pSocket, uint32_t state){
	if (!pSocket) return false;
	int iSocket = pSocket->GetSocket();
    struct epoll_event ev;
	ev.events = state;
    ev.data.ptr = pSocket;
	int iRet = epoll_ctl(m_epoll, EPOLL_CTL_ADD, iSocket, &ev);
	if (iRet == -1){
        printf("[?] CEServer::Add() err:%d (%s)\n",errno,strerror(errno));
		return false;
	}	
	return true;
} 
bool CEServer::Mod(CESocket* pSocket, uint32_t state){
	if (!pSocket) return false;
	int iSocket = pSocket->GetSocket();
    struct epoll_event ev;
	ev.events = state;
    ev.data.ptr = pSocket;
	if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, iSocket, &ev) == -1)
	{
        printf("[?] CEServer::Mod() err:%d (%s)\n",errno,strerror(errno));
		return false;
	}	
    return true;
}
bool CEServer::Del(CESocket* pSocket){
	if (!pSocket) return false;
	int iSocket = pSocket->GetSocket();
    struct epoll_event ev;
	memset(&ev,0,sizeof(ev));
	int iRet = epoll_ctl(m_epoll, EPOLL_CTL_DEL, iSocket, &ev);
	if (iRet == -1){
        printf("[?] CEServer::Del() err:%d (%s)\n",errno,strerror(errno));
		return false;
	}	
	return true;
}