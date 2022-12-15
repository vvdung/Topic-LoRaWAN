#include "main.h"
#include "MQTTClient.h"

CMQTTClient::CMQTTClient():CEClient(){    
    m_pConnectedBytes = NULL;
}
CMQTTClient::~CMQTTClient(){
    SAFE_DELETE(m_pConnectedBytes);
}

bool CMQTTClient::ConnectEx(const char *lpszHostAddress, uint32_t nHostPort, uint8_t* pData, uint32_t uSize){
    if (!pData || uSize <= 0){
        SAFE_DELETE(m_pConnectedBytes);
        return Connect(lpszHostAddress,nHostPort);
    }
    if (!m_pConnectedBytes) m_pConnectedBytes = new CByteBuffer(uSize);
    else m_pConnectedBytes->Resize(uSize);
    m_pConnectedBytes->Copy(pData,uSize);

    return Connect(lpszHostAddress,nHostPort);
}

void CMQTTClient::OnCreated(){
    CEClient::OnCreated();
    //LOG_WT("CMQTTClient::OnCreated()\n");
    // m_sumRecv.Clear();
    // m_sumSend.Clear();
}
void CMQTTClient::OnConnected(){
    CEClient::OnConnected();

    // LOG_IT("[C] OnConnected(%d<->%d) [%s:%d]-[%s:%d]\n",m_hSocket,m_pConnect->GetSocket(),
    //     m_pConnect->m_czRemoteIP,m_pConnect->m_uRemotePort,m_czRemoteIP,m_uRemotePort);
    if (m_uRemotePort == 0){
        shutdown(m_hSocket,SHUT_RD);
        return;
    }
    if (m_pConnectedBytes){
        Send((char*)m_pConnectedBytes->Get(),m_pConnectedBytes->Size()); 
    }
    else m_pConnect->Send_Established();
}
void CMQTTClient::OnReceived(uint8_t* pBuffer,int uSize){
    CEClient::OnReceived(pBuffer,uSize);
    //m_sumRecv.Add(pBuffer,(uint32_t)uSize);
    
    //LOG_D("[C] OnReceived(%d<->%d) RECV:%d/%lu SUM:%X \tCLIENT \n",m_hSocket,m_pConnect->GetSocket(),uSize,m_uTotalRecv,m_sumRecv.GetChecksum());    
    
   //LOG_I("[C] OnReceived(%d<->%d) RECV:%d/%lu \tCLIENT \n",m_hSocket,m_pConnect->GetSocket(),uSize,m_uTotalRecv);
    //LOG_HEX(pBuffer,uSize);
    m_pConnect->SendConnectData(pBuffer,uSize);
}

void CMQTTClient::OnSend(uint8_t *pBuffer,int uSize){
    CEClient::OnSend(pBuffer,uSize);
    //m_sumSend.Add(pBuffer,(uint32_t)uSize);
    //LOG_D("[C] OnSend    (%d<->%d) SEND:%d/%lu SUM:%X \tCLIENT\n",m_hSocket,m_pConnect->GetSocket(),uSize,m_uTotalSend,m_events,m_sumSend.GetChecksum());
    //LOG_I("[C] OnSend    (%d<->%d) SEND:%d/%lu \tCLIENT\n",m_hSocket,m_pConnect->GetSocket(),uSize,m_uTotalSend);
}

void CMQTTClient::OnClosed(){
    
    if (m_pConnect->GetSocket() != -1){
        m_pConnect->m_socksStep == socks_step_response_finished;
        m_pConnect->WaitSendCompleted();        
        
        //shutdown(m_pConnect->GetSocket(),SHUT_RD);
    } 

    // LOG_W("[C] OnClosed (%d<->%d) [%s:%d]-[%s:%d] SEND[%lu] RECV[%lu] SUM:%X [%X]\n",
    //     m_hSocket,m_pConnect->GetSocket(),
    //     m_pConnect->m_czRemoteIP,m_pConnect->m_uRemotePort,m_czRemoteIP,m_uRemotePort,
    //     m_uTotalSend,m_uTotalRecv,m_sumSend.GetChecksum(),m_sumRecv.GetChecksum());

    CEClient::OnClosed();
}
void CMQTTClient::OnTimerRoutine(){
    uint64_t t = GetTickCount();
    if (t - m_uTTL < 10000) return;
     
}


