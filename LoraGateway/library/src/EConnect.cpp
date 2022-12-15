#include "../inc/HFactor.h"
#include "../inc/EConnect.h"

CEConnect::CEConnect():CESocket(){
    m_State = NONE;
    m_uNextReceived = MAX_PACKAGE_SIZE;
    m_uTotalRecv = 0;

    m_pSendBuffer = NULL;
    m_uTotalSend = 0;
}
CEConnect::~CEConnect(){
    m_State = NONE;
}
void CEConnect::OnPreBind(){
    CESocket::OnPreBind();

}

void CEConnect::OnCreated(){
    m_uStarted = GetTickCount();
    m_State = CREATED;
    m_uNextReceived = MAX_PACKAGE_SIZE;
    m_uTotalRecv = 0;
    m_pSendBuffer = NULL;
    m_uTotalSend = 0;

/*	int         nZero = 0;      
	setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));  */
    /*nZero = 0;   
	setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));*/  
/*	linger      lingerStruct;
    lingerStruct.l_onoff = 0;   
	lingerStruct.l_linger = 0;
	setsockopt(m_hSocket, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct));*/     
}
void CEConnect::OnConnected(){
    m_uTTL = GetTickCount(); 
    m_State = ACCEPTED;
}
void CEConnect::OnReceived(uint8_t* pBuffer,int uSize){
    m_uTTL = GetTickCount();
}
void CEConnect::OnSend(int uSize){
    m_uTTL = GetTickCount();
}
void CEConnect::OnClosed(){ 
    m_State = DISCONNECTED;       
}
void CEConnect::OnTimerRoutine(){
    CESocket::OnTimerRoutine();
}
void CEConnect::OnEpollin(){
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
    m_pServer->Mod(this,flags);
}
void CEConnect::OnEpollout(){
    Flush();   
}

void CEConnect::SetServer(CEServer* p){
    m_pServer = p;
}
bool CEConnect::Send(char *pBuffer,uint32_t uSize, bool bFlush){
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
    m_pServer->Mod(this,EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP);
    return true;
}
bool CEConnect::Flush(){
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
	if (p)
	{	
        int nBytes = write(m_hSocket,&p->pBytes[p->uSend],p->uSize - p->uSend);            				
        if (nBytes > 0){
            p->uSend += nBytes;
            m_uTotalSend += nBytes;
            m_csSend.unlock();
            OnSend(nBytes);	                           	
        }   
        else m_csSend.unlock();     
        m_pServer->Mod(this, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP);
    }
    else m_csSend.unlock();
    return true;
}