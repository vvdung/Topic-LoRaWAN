#pragma once
#include "../inc/EThread.h"

#include "UDPClient.h"
#include "LoRa.h"

#define GATEWAY_KEY "o-rqfxHxbrH9eLun27G!cf77Ab07"

const long HF_FREQUENCE = 433175000;  // LoRa Frequency

class CMainThread: public CEThread
{
public:
    CMainThread();
    virtual ~CMainThread();
	bool 	IsBelongtoGate(const char* key,uint64_t* nid = NULL);

	//LORA
	void 	LoRa_Setup();
	void 	LoRa_rxMode();
	void 	LoRa_txMode();
	void 	LoRa_sendMessage(String message);
	static void 	_LoRaOnReceive(int packetSize);
	static void 	_LoRaOnTxDone();
	void    AddQueue(const char* msg);
	void 	SendData(CBSONObject* oMsg);
protected:
	virtual void	InitInstance();
	virtual void	Run();
	virtual void	ExitInstance();

	CUDPClient*		m_udpClient;
	uint64_t		m_uid;
	uint64_t		m_gid;
	std::vector<const char*> m_queueData;
	std::mutex		m_mutexQueue;
};