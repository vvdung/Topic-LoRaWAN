#include "../inc/HFactor.h"
#include "../inc/EServer.h"

CEServer::CEServer():CESocket(){
	m_uThreadID = 0;
	m_epoll = -1;
    m_thPool = new CPoolThread(_ProcessOneShot,sizeof(SHOTDATA));
}
CEServer::~CEServer(){
    delete m_thPool;
    if (m_epoll != -1) close(m_epoll);
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
void* CEServer::_ThreadEpollRoutine(void* lpData){
    CEServer* pServer = (CEServer*)lpData;
    pServer->OnThreadEpollRoutine();
    return NULL;
}
void CEServer::OnThreadEpollRoutine(){
    m_epoll = epoll_create1(0);
	if (m_epoll == -1){
		printf("epoll_create() error:%d \n",errno);
		return;
	}

    if (!OnInit()) return;    
    Add(this,EPOLLIN | EPOLLET);
    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(events) * 1024);

    CEConnect* pConnect;
    int iRet;
    while (1){
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
                    printf(" SERVER EVENT %d SOCKET:%d Events:%X iRet:%d\n",i,hSocket,events[i].events,iRet);				
                    continue;
                }
                  
                this->OnDisconnect(pConnect);	
                this->Del(pConnect);		
                pConnect->OnClosed();	
                m_queueSocket.push(pConnect);
                continue;
			}

            if (hSocket == this->m_hSocket){
                CThreadInfo* pThread = m_thPool->GetQueue();
                SHOTDATA* pData = (SHOTDATA*)pThread->GetArgs();
                pData->pConnect = NULL;
                pData->pServer = this;                
                pThread->RunRoutine();
                continue;                
            }
            
            if (events[i].events & EPOLLIN){
                CThreadInfo* pThread = m_thPool->GetQueue();
                SHOTDATA* pData = (SHOTDATA*)pThread->GetArgs();
                pData->pConnect = pConnect;
                pData->pServer = this;                
                pThread->RunRoutine();
            }
            else {
                printf("STATUS [%X] CLIENT=%d\n",events[i].events,hSocket);
            }                           
        }//for (int i = 0; i < iRet; ++i)
    }//while (bLoop){
    OnExit();
    free(events);
    pthread_exit(NULL);
}

/*void CEServer::OnThreadEpollRoutine(){
    m_epoll = epoll_create1(0);
	if (m_epoll == -1){
		printf("epoll_create() error:%d \n",errno);
		return;
	}

    if (!OnInit()) return;    
    Add(this,EPOLLIN | EPOLLET);
    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(events) * 1024);

    CEConnect* pConnect;
    int iRet;
    while (1){
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
                    printf(" SERVER EVENT %d SOCKET:%d Events:%X iRet:%d\n",i,hSocket,events[i].events,iRet);				
                    continue;
                }
                  
                this->OnDisconnect(pConnect);	
                this->Del(pConnect);		
                pConnect->OnClosed();	
                m_queueSocket.push(pConnect);
                continue;
			}

            if (hSocket == this->m_hSocket){
                CThreadInfo* pThread = m_thPool->GetQueue();
                SHOTDATA* pData = (SHOTDATA*)pThread->GetArgs();
                pData->pConnect = NULL;
                pData->pServer = this;                
                pThread->RunRoutine();
                //OnAccept();	
                //CEConnect* p = OnAccept();
                //if (!p){
                //    printf("ERROR CreateConnect from OnAccept()\n");
                //    continue;
                //}
                //Add(p,EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT);
                continue;                
            }
            
            if (events[i].events & EPOLLIN){
                CThreadInfo* pThread = m_thPool->GetQueue();
                SHOTDATA* pData = (SHOTDATA*)pThread->GetArgs();
                pData->pConnect = pConnect;
                pData->pServer = this;                
                pThread->RunRoutine();

                //Mod(pSocket,EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT);
            }
            else {
                printf("STATUS [%X] CLIENT=%d\n",events[i].events,hSocket);
            }

                           
        }//for (int i = 0; i < iRet; ++i)
    }//while (bLoop){
    OnExit();
    free(events);
    pthread_exit(NULL);
}*/

bool CEServer::_ProcessOneShot(void* th){
    CThreadInfo* thInfo = (CThreadInfo*)th;
    SHOTDATA* pData = (SHOTDATA*)thInfo->GetArgs();
    if (!pData->pConnect){
        pData->pServer->OnAccept();
        return true;
    }
    pData->pServer->OnEpollOneShot(pData->pConnect);
    //pData->pServer->Mod(pData->pConnect,EPOLLIN | EPOLLET | EPOLLONESHOT);
    return true;
/*
    char buff[MAX_PACKAGE_SIZE];
    SOCKET s = pData->pConnect->GetSocket();
    int len = read(s, buff, sizeof(buff));
    printf("READ CLIENT=%d RECV:%d\n",s,len);
    pData->pServer->Mod(pData->pConnect,EPOLLIN | EPOLLET | EPOLLONESHOT);
    return true;*/

    /*if (pData->isRead){

    }                
    printf("WRITE CLIENT=%d\n",s);
    pData->pServer->Mod(pData->pConnect,EPOLLIN | EPOLLET | EPOLLONESHOT);
    return 1;*/
}
bool CEServer::OnEpollOneShot(CEConnect* p){
    Mod(p,EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT);
    return true;
}
bool CEServer::Add(CESocket* pSocket, uint32_t state){
	if (!pSocket) return false;
	int iSocket = pSocket->GetSocket();

	//if (m_mapSocket[iSocket]) return false;
    struct epoll_event ev;
	ev.events = state;
	//ev.data.fd = iSocket;
    ev.data.ptr = pSocket;
	int iRet = epoll_ctl(m_epoll, EPOLL_CTL_ADD, iSocket, &ev);
	if (iRet == -1){
		return false;
	}	
	//m_mapSocket[iSocket] = (CEConnect*)pSocket;
	return true;
} 
bool CEServer::Mod(CESocket* pSocket, uint32_t state){
	if (!pSocket) return false;
	int iSocket = pSocket->GetSocket();
    //CESocket* p = m_mapSocket[iSocket];
    //if (!p) return false;
    struct epoll_event ev;
	ev.events = state;
	//ev.data.fd = iSocket;
    ev.data.ptr = pSocket;
	if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, iSocket, &ev) == -1)
	{
		return false;
	}	
    return true;
}
bool CEServer::Del(CESocket* pSocket){
	if (!pSocket) return false;
	int iSocket = pSocket->GetSocket();
	//std::map<int,CEConnect*>::iterator it = m_mapSocket.find(iSocket);
	//if (it == m_mapSocket.end()) return false;
    struct epoll_event ev;
	memset(&ev,0,sizeof(ev));
	int iRet = epoll_ctl(m_epoll, EPOLL_CTL_DEL, iSocket, &ev);
	if (iRet == -1){
		return false;
	}	
	//m_mapSocket.erase(it);
	return true;
}