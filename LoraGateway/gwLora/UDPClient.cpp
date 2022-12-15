#include "main.h"
#include "UDPClient.h"

#define MAX_UDP_BUFFER  1024 //8192  //512

CUDPClient::CUDPClient():CEClient(){

}
CUDPClient::~CUDPClient(){

}

void CUDPClient::OnCreated(){
    CEClient::OnCreated();
    
}
void CUDPClient::OnConnected(){
    static char czPING = 1;
    CEClient::OnConnected();
    printf("[C] OnConnected(%d) [%s:%d]\n",m_hSocket,m_czRemoteIP,m_uRemotePort);
    Send(&czPING,1);
 
}
void CUDPClient::OnReceived(uint8_t* pBuffer,int uSize){
    CEClient::OnReceived(pBuffer,uSize);
    if (uSize == 1 && pBuffer[0] == 1){
        printf("-----CHECK LIVE---- RECV:%lu SEND:%lu\n",m_uTotalRecv,m_uTotalSend);
        return;
    }
    printf("[+] OnReceived(%d) [%s:%d] RECV:%d/%lu Send_UF[%lu/%lu]\n",m_hSocket,m_czRemoteIP,m_uRemotePort,uSize,m_uTotalRecv,
            m_lstSendBuffer.m_lstUsed.size(),m_lstSendBuffer.m_lstFree.size());
}

void CUDPClient::OnSend(int uSize){
    CEClient::OnSend(uSize);

    //size_t cntUsed = m_lstSendBuffer.m_lstUsed.size();
    /*printf("CUDPClient::OnSend(%d) uSize:%d/%lu Send_UF[%lu/%lu]\n",
                m_hSocket,uSize,m_uTotalSend,
                cntUsed,
                m_lstSendBuffer.m_lstFree.size()); */    

    //if (cntUsed > 0) return;      
}

void CUDPClient::OnClosed(){
    CEClient::OnClosed();
    printf("CUDPClient::OnClosed(%d) RECV:%lu SEND:%lu\n",m_hSocket,m_uTotalRecv,m_uTotalSend);
    //printf("[+] OnClosed(%d) [%s:%d] ME[%p]\n",m_hSocket,m_czRemoteIP,m_uRemotePort,this);
    Close();
}
void CUDPClient::OnTimerRoutine(){
    uint64_t t = GetTickCount();
    if (t - m_uTTL < 10000) return;
    m_uTTL = t;
    
}