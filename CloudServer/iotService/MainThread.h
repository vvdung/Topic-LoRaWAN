#pragma once
//#include "../inc/EThread.h"

#include "ApiHFactor.h"

#include "HFactorClientPool.h"
#include "UDPServer.h"

//#define g_pHFApi			theApp.m_pHFApi

class CMainThread: public CEThread
{
public:
    CMainThread();
    virtual ~CMainThread();
	void 			AddSensor(CBSONObject* oData);
protected:
	virtual void	InitInstance();
	virtual void	Run();
	virtual void	ExitInstance();

	void 			InitSensorRandom();
	static void*	_ThreadSensorRandom(void* lpData);
	void OnThreadSensorRandom(HFGate* pGate);

	void 			InitMongoSensorRandom();
	static void*	_ThreadMongoSensorRandom(void* lpData);
	void OnThreadMongoSensorRandom(HFGate* pGate);

	void 			InitMongoTesting();
	static void*	_ThreadMongoTesting(void* lpData);
	void OnThreadMongoTesting(int index);

	CApiHFactor*	m_pHFApi;
	HFUser			m_HFUser;
	std::vector<HFGate>	m_lstGate;

	CHFactorClientPool* m_pPoolClient;
	
	CUDPServer*		m_udpServer;
};