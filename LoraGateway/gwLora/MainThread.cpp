#include "main.h"
#include "MainThread.h"

CMainThread::CMainThread():CEThread(){

}
CMainThread::~CMainThread(){

}
bool CMainThread::IsBelongtoGate(const char* key,uint64_t* nid){
    if (!key || key[0] == 0) return false;
    std::string nodeKey = key;
    CHFT hft;
    hft.Decode(nodeKey.c_str());
    std::string szKey = hft.ToString();
    
    bson_error_t error;	
	bson_t* b = CBSONObject::Parse(szKey.c_str(), &error);
	if (!b) return false;
    CBSONObject oKey(b);
    if (!oKey.HasKey("g")) return false;
    if (!oKey.HasKey("n")) return false; 
    if (nid) *nid = oKey.GetInt64("n");
    return (m_gid == oKey.GetInt64("g"));
}

void CMainThread::InitInstance(){
    printf("[+] CMainThread::InitInstance()\n");
    m_udpClient = NULL;

    std::string gwKey(GATEWAY_KEY);
    CHFT hft;
    hft.Decode(gwKey.c_str());
    std::string szKey = hft.ToString();
    printf("GATE KEY [%s]\n",szKey.c_str());
    bson_error_t error;	
	bson_t* b = CBSONObject::Parse(szKey.c_str(), &error);
	if (!b) {
		printf("Error: %s\n", error.message);
		return;
	}
    CBSONObject oGateKey(b);
    if (!oGateKey.HasKey("g")){
        printf("Error: WHERE gid???\n");
        return;
    }
    if (!oGateKey.HasKey("u")){
        printf("Error: WHERE uid???\n");
        return;
    }

    m_gid = oGateKey.GetInt64("g");
    m_uid = oGateKey.GetInt64("u");
    printf("USERID:%lu GATEID:%lu\n",m_uid,m_gid);


    CEClient::Initialization();
    m_udpClient = new CUDPClient();
    if (m_udpClient->Create(SOCK_DGRAM)){
        if (!m_udpClient->Connect("209.126.13.94",4959)){
            printf("CONNECT err:%d (%s)\n",errno,strerror(errno));
        }
    }

    LoRa_Setup();
}
void CMainThread::Run(){
    //static uint32_t i = 0;
    //static uint64_t tStarted = 0;
    //if (!IsReady(2000,tStarted)) return;
    //++i;    
    //printf(" - Looper %u [%lu]\n",i,GetTickCount());
    //if (i > 10) Exit();
/*    uint64_t nid;
    std::string nodeKey = "pQm9rRTK3-_AlQz52xaT{9ADeD139";
    bool bOk = IsBelongtoGate(nodeKey.c_str(),&nid);
    printf("[*] KEY[%s] => %d\n",nodeKey.c_str(),bOk);
    if (!bOk) return;
    
    std::string szName = StringFormat("Welcome (%d)",i);
    CBSONObject oSensor;
	oSensor.AddMember("name",szName.c_str());
    CBSONObject oData;
    oData.AddMember("g",(int64_t)m_gid);
    oData.AddMember("n",(int64_t)nid);
    oData.AddMember("s",&oSensor);

    std::string szJson = oData.ToJson();
    CHFT hft;
    std::string szEncode = hft.Encode(szJson.c_str());
    printf("%s\n",szJson.c_str());
    printf("[%ld] %s\n",szEncode.size(),szEncode.c_str());
    m_udpClient->Send((char*)szEncode.c_str(),szEncode.size()+1);
*/
    return;
    const char* msg = NULL;
    m_mutexQueue.lock();
    if (!m_queueData.empty()){
        msg = m_queueData.front();
        m_queueData.pop_back();
    }
    m_mutexQueue.unlock();
    if (!msg) return;

    printf("LORA RECIEVER %s\n",msg);
}
void CMainThread::ExitInstance(){
    printf("[-] CMainThread::ExitInstance()\n");
    CEClient::Exitialization();
}
void CMainThread::AddQueue(const char* msg){
    if (!msg || msg[0] == 0) return;
    printf("RECEIVER: %s\n",msg);
    return;
    m_mutexQueue.lock();
    m_queueData.push_back(msg);
    m_mutexQueue.unlock();
}
void CMainThread::SendData(CBSONObject* oMsg){
    if (!oMsg) return;
    if (!oMsg->HasKey("k")) return;
    if (!oMsg->HasKey("s")) return;
    const char* key = oMsg->GetUTF8("k");
    uint64_t nid;    
    if (!IsBelongtoGate(key,&nid)) return;
    bson_t bSensor;
    if (!oMsg->GetObject("s",&bSensor)) return;
    CBSONObject oSensor(&bSensor);
    
    CBSONObject oData;
    oData.AddMember("g",(int64_t)m_gid);
    oData.AddMember("n",(int64_t)nid);
    oData.AddMember("s",&oSensor);

    std::string szJson = oData.ToJson();
    CHFT hft;
    std::string szEncode = hft.Encode(szJson.c_str());
    
    if (!m_udpClient->Send((char*)szEncode.c_str(),szEncode.size()+1)){
        delete m_udpClient;        
        m_udpClient = new CUDPClient();
        if (!m_udpClient->Create(SOCK_DGRAM)) return;        
        if (!m_udpClient->Connect("209.126.13.94",4959)) return;
    }

    printf("KEY[%s] %s\n",key,oSensor.ToJson().c_str());
    if (m_gid == 12 && nid == 51){//nid == 57){
        String msg = "RGB";
        LoRa_sendMessage(msg);
    }
}

////////LORA FUNCTION///////////////////////////////////////////////
void CMainThread::LoRa_Setup(){
    LoRa.setPins(6, 0, 7);

    if (!LoRa.begin(HF_FREQUENCE)) {
        printf("LoRa init failed. Check your connections.\n");
        while (true);                       // if failed, do nothing
    }

    printf("LoRa init succeeded.\n\n");
    printf("LoRa Simple Gateway\n");
    printf("Only receive messages from nodes\n");
    printf("Tx: invertIQ enable\n");
    printf("Rx: invertIQ disable\n\n");

    LoRa.onReceive(_LoRaOnReceive);
    LoRa.onTxDone(_LoRaOnTxDone);
    LoRa_rxMode();
}
void CMainThread::LoRa_rxMode(){
    LoRa.disableInvertIQ();               // normal mode
    LoRa.receive();                       // set receive mode
}
void CMainThread::LoRa_txMode(){
    LoRa.idle();                          // set standby mode
    LoRa.enableInvertIQ();                // active invert I and Q signals
}
void CMainThread::LoRa_sendMessage(String message){
    LoRa_txMode();                        // set tx mode
    LoRa.beginPacket();                   // start packet
    LoRa.print(message);                  // add payload
    LoRa.endPacket(true);                 // finish packet and send it
}

void CMainThread::_LoRaOnReceive(int packetSize){
    String message = "";

    while (LoRa.available()) {
        message += (char)LoRa.read();
    }

    bson_error_t error;
    bson_t* b = CBSONObject::Parse(message.c_str(),&error);
    if (!b) return;
    CBSONObject oMsg(b);
    theApp.SendData(&oMsg);
}

void CMainThread::_LoRaOnTxDone(){
    theApp.LoRa_rxMode();
}
