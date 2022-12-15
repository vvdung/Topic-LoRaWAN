#include "main.h"
#include "UDPServer.h"

CUDPServer::CUDPServer():CEServer(){

}
CUDPServer::~CUDPServer(){

}
bool CUDPServer::OnInit(){
    if (!Create(SOCK_DGRAM, 4959)){
        printf("[-] CUDPServer::OnInit() error:%d\n",errno);
        return false;
    }
    
    return true;
}
bool CUDPServer::OnExit(){
    printf("CUDPServer::OnExit()....\n");
    return true;
}
CUDPConnect* CUDPServer::CreateConnect(struct sockaddr_in* pClientAddr){
    CUDPConnect* p = (CUDPConnect*)m_queueConnect.Pop();
    if (!p) p = new CUDPConnect();
    p->SetServer(this);

    if (!p->Socket(SOCK_DGRAM)){
        printf("CreateUDP() p->Socket failed\n");
        delete p;return NULL;
    }  
    if (!SetNonblocking(p->GetSocket())){
        printf("CreateUDP() SetNonblocking failed\n");
        delete p;return NULL; 
    }
    int reuse = 1;
    if (setsockopt(p->GetSocket(), SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
    {
        printf("CreateUDP() setsockopt(SO_REUSEADDR) failed\n");
        delete p; return NULL;
    } 

    sockaddr_in srvaddr; 
	memset(&srvaddr,0, sizeof(srvaddr)); 
	socklen_t size = sizeof(srvaddr);
    getsockname(m_hSocket, reinterpret_cast<sockaddr*>(&srvaddr), &size);
    if (!p->Bind((const SOCKADDR*)&srvaddr,sizeof(struct sockaddr))){
        printf("CreateUDP() p->Bind failed\n");
        delete p; return NULL;
    }
    if (connect(p->GetSocket(),(struct sockaddr*)pClientAddr,sizeof(struct sockaddr)) == SOCKET_ERROR){
        printf("CreateUDP() p->connect failed\n");
        delete p; return NULL;
    }
    return p; 
}
void CUDPServer::OnAccept(){
    int len;
    char buff[MAX_PACKAGE_SIZE];
    char czip[16];
    struct sockaddr_in clientaddr;
    socklen_t clilen = sizeof(struct sockaddr);
    len = recvfrom(m_hSocket, buff, sizeof(buff), 0, (struct sockaddr*)&clientaddr, &clilen);
    char* pIP = inet_ntoa(clientaddr.sin_addr);
    strcpy(czip,pIP);
    int port = (int)ntohs(clientaddr.sin_port);

    CUDPConnect* p = CreateConnect(&clientaddr);
    if (!p){
        printf("ERROR CreateUDP()\n");
        return;
    }   
        
    p->m_uRemotePort = port;
    strcpy(p->m_czRemoteIP,czip);
    p->OnCreated();

    Add(p,EPOLLIN | EPOLLET | EPOLLRDHUP);
    p->OnConnected();

    memcpy(p->m_recvBuffer,buff,len);
    p->m_uTotalRecv += len;
    p->OnReceived(p->m_recvBuffer,len);
    p->AddTimerLoop(1000);
    
}
void CUDPServer::OnDisconnect(CEConnect* p){
    //printf("CUDPServer::OnDisconnect(%p) map:%lu queue:%lu\n",p,m_mapSocket.size(),m_queueSocket.size());
}
void CUDPServer::OnCreated(){
    CEServer::OnCreated();
    printf("UDP Server listening @ %u \n",m_uPortBind);
}
