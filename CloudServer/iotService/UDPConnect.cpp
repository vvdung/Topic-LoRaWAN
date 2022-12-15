#include "main.h"
#include "UDPConnect.h"

CUDPConnect::CUDPConnect():CEConnect(){
    m_uCntPING = 0;
}
CUDPConnect::~CUDPConnect(){

}

void CUDPConnect::OnCreated(){
    CEConnect::OnCreated();
    m_uCntPING = 0;
}
void CUDPConnect::OnConnected(){
    CEConnect::OnConnected();
    m_uCntPING = 0;
    printf("[+] OnConnected(%d) [%s:%d] ME[%p]\n",m_hSocket,m_czRemoteIP,m_uRemotePort,this);
}
void CUDPConnect::OnReceived(uint8_t* pBuffer,int uSize){
    CEConnect::OnReceived(pBuffer,uSize);
    m_uCntPING = 0;
    if (uSize <= 0) return;
    if (uSize == 1 && pBuffer[0] == 1){
        printf("==> LIVE(%d) [%s:%d] => RECV:%lu SEND:%lu\n",m_hSocket,m_czRemoteIP,m_uRemotePort,m_uTotalRecv,m_uTotalSend);
        return;
    }
    if (pBuffer[uSize - 1] != 0){
        printf("[?] NOT ZERO [%s:%d] => RECV:%lu SEND:%lu\n",m_czRemoteIP,m_uRemotePort,m_uTotalRecv,m_uTotalSend);
        return;
    }
    CHFT hft;
    uint8_t* pStr = hft.Decode((char*)pBuffer);
    if (!pStr){
        printf("[?] NOT HFACTOR [%s:%d] => RECV:%lu SEND:%lu\n",m_czRemoteIP,m_uRemotePort,m_uTotalRecv,m_uTotalSend);
        return;
    }
    std::string szJson = hft.ToString();
    //printf("[+] OnReceived(%d) [%s:%d] RECV:%d/%lu\n",m_hSocket,m_czRemoteIP,m_uRemotePort,uSize,m_uTotalRecv);
    //printf("%s\n",szJson.c_str());
    bson_error_t error;	
	bson_t* b = CBSONObject::Parse(szJson.c_str(), &error);
	if (!b){
        printf("CBSONObject::Parse() Error - %s\n",error.message);
        return;
    }
    CBSONObject oData(b);
    theApp.AddSensor(&oData);
}

void CUDPConnect::OnSend(int uSize){
    CEConnect::OnSend(uSize);
    
    /*size_t cntUsed = m_lstSendBuffer.m_lstUsed.size();
    printf("[+] OnSend(%d) uSize:%d/%lu Send_UF[%lu/%lu]\n",
                m_hSocket,uSize,m_uTotalSend,
                cntUsed,
                m_lstSendBuffer.m_lstFree.size());*/    
}

void CUDPConnect::OnClosed(){
    CEConnect::OnClosed();
    printf("[+] OnClosed(%d) [%s:%d] RECV[%lu] SEND[%lu] PING:%d\n",m_hSocket,m_czRemoteIP,m_uRemotePort,m_uTotalRecv,m_uTotalSend,m_uCntPING);
    Close();
}
void CUDPConnect::OnTimerRoutine(){
    static char czPING = 1;
    uint64_t t = GetTickCount();
    if (t - m_uTTL < 15000) return;
    if (m_uCntPING > 1){//>1 = 30s
        shutdown(m_hSocket,SHUT_RDWR);
    }
    else{
        ++m_uCntPING;
        Send(&czPING,1);
    } 
}