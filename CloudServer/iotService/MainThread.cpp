#include "main.h"
#include "MainThread.h"

#include "HFactorClient.h"

CMainThread::CMainThread():CEThread(){
	
}
CMainThread::~CMainThread(){

}
void CMainThread::InitInstance(){
    printf("[+] CMainThread::InitInstance()\n");
	m_pHFApi = NULL;
	m_udpServer = NULL;
	m_pPoolClient = new CHFactorClientPool("mongodb://factor:factor%40123@127.0.0.1:27017/","factor_db");
    
	m_udpServer = new CUDPServer();
	m_udpServer->Start();

	//InitMongoSensorRandom();

	//InitSensorRandom();
	
	//InitMongoSensorRandom();
	//InitMongoTesting();

}
void CMainThread::Run(){
    //static uint32_t i = 0;
    static uint64_t tStarted = 0;
    if (!IsReady(1000,tStarted)) return;
    //++i;    
    //printf(" - Looper %u [%lu]\n",i,GetTickCount());
    //if (i >= 20) Exit();
}
void CMainThread::ExitInstance(){
    printf("[-] CMainThread::ExitInstance()\n");
    if (m_pHFApi) delete m_pHFApi;
	if (m_pPoolClient) delete m_pPoolClient;
	if (m_udpServer){
		m_udpServer->Exit();
		delete m_udpServer;
	}
}
void CMainThread::AddSensor(CBSONObject* oData){
	if (!oData) return;
	if (!oData->HasKey("n")) return;
	if (!oData->HasKey("g")) return;
	if (!oData->HasKey("s")) return;
	uint64_t gid = oData->GetInt64("g");
	uint64_t nid = oData->GetInt64("n");
	bson_t b;
	oData->GetObject("s",&b);
	CBSONObject oSensor(&b);

    CHFactorClient* pClient = m_pPoolClient->Pop();
	if (!pClient) return;
	CHFactorDatabase* pDB = pClient->GetDatabase();
	uint64_t sid;
	if (pDB->AddSensor(gid,nid,oSensor,sid)){
		//printf("AddSensor(gid:%lu,nid:%lu) => sid[%lu] : OKIE\n",gid,nid,sid);
		if (gid == 12 && nid == 57){	//ADD REAL
			//std::string jzSen = oSensor.ToJson();	
			//printf("ADD(12,57)%s\n",jzSen.c_str());
			double ph, nh3, h2s, no2, w_t, w_f,do_;

			CBSONObject oSensorReal;

			if (!oSensor.HasKey("ph")) {
				ph = RandomMinMax(6500, 8500) / 1000;
				oSensorReal.AddMember("ph",ph);
			}
			else oSensorReal.AddMember("ph",oSensor.GetDouble("ph"));
			
			if (!oSensor.HasKey("nh3")) {
				nh3 = RandomMinMax(1000, 3000) / 100000;
				oSensorReal.AddMember("nh3",nh3);
			}
			else oSensorReal.AddMember("nh3",oSensor.GetDouble("nh3"));
			if (!oSensor.HasKey("h2s")) {
				h2s = RandomMinMax(500, 1000) / 100000;
				oSensorReal.AddMember("h2s",h2s);
			}
			else oSensorReal.AddMember("h2s",oSensor.GetDouble("h2s"));
			if (!oSensor.HasKey("no2")) {
				no2 = RandomMinMax(2000, 5000) / 10000;
				oSensorReal.AddMember("no2",no2);
			}
			else oSensorReal.AddMember("no2",oSensor.GetDouble("no2"));
			if (!oSensor.HasKey("do")) {
				do_ = RandomMinMax(4000, 9000) / 1000;
				oSensorReal.AddMember("do",do_);
			}
			else oSensorReal.AddMember("do",oSensor.GetDouble("do"));

			if (!oSensor.HasKey("water_temp")) {
				w_t = RandomMinMax(20000, 33000) / 1000;
				oSensorReal.AddMember("water_temp",w_t);
			}
			else oSensorReal.AddMember("water_temp",oSensor.GetDouble("water_temp"));

			if (!oSensor.HasKey("water_flow")) {
				w_f = RandomMinMax(2000, 3000) / 10000;
				oSensorReal.AddMember("water_flow",w_f);
			}
			else oSensorReal.AddMember("water_flow",oSensor.GetDouble("water_flow"));
			
			pDB->AddSensor(2,2,oSensorReal,sid);
		}//if (gid == 12 && nid == 57){
		
	}//if (pDB->AddSensor(gid,nid,oSensor,sid)){


	m_pPoolClient->Push(pClient);
}
void CMainThread::InitSensorRandom(){
	m_pHFApi = new CApiHFactor();

    m_pHFApi->User_Get("admin@admin.com",m_HFUser);
    printf("uid:%lu [%s] [%s]\n", m_HFUser.uid, m_HFUser.szUsername.c_str(), m_HFUser.szFullname.c_str());

    m_pHFApi->User_Gate_Gets(m_HFUser.uid, m_lstGate);
	int iCntGate = (int)m_lstGate.size();
	printf("GATE COUNT:%d\n", iCntGate);

    if (iCntGate > 2) iCntGate = 2;
	for (int i = 0; i < iCntGate; ++i) {
		HFGate* pGate = &m_lstGate[i];

        struct tm tmLocal;
        localtime_r(&pGate->date_created,&tmLocal);
        char buffer [32];
        buffer[0] = 0;
        strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", &tmLocal);
        printf("[+] %s %s %s %s\n",
        pGate->szGateId.c_str(),pGate->szDesc.c_str(),pGate->szGateKey.c_str(),buffer);

		m_pHFApi->User_Gate_Node_Gets(pGate->uid, pGate->gid, pGate->lstNode);
		int iCntNode = (int)pGate->lstNode.size();
		printf("[+] idx:%d GATE[%lu] NODE COUNT:%d\n",i,pGate->gid, iCntNode);		
	}

    if (iCntGate < 2) return;
    pthread_t uTD;
    pthread_create(&uTD,NULL,_ThreadSensorRandom,&m_lstGate[0]);
    pthread_create(&uTD,NULL,_ThreadSensorRandom,&m_lstGate[0]);
    pthread_create(&uTD,NULL,_ThreadSensorRandom,&m_lstGate[1]);
    pthread_create(&uTD,NULL,_ThreadSensorRandom,&m_lstGate[1]);
}
void* CMainThread::_ThreadSensorRandom(void* lpData){
    HFGate* pGate = (HFGate*)lpData;
    theApp.OnThreadSensorRandom(pGate);
    return NULL;
}
void CMainThread::OnThreadSensorRandom(HFGate* pGate){
    srand((uint32_t)time(NULL));

	//HFGate* pGate = &m_lstGate[0];
	int iCntNode = (int)pGate->lstNode.size();
	std::string szKey, szJsonSensor;
	CApiHFactor api;
	int iLast = 0;
	double ph, nh3, h2s, no2, w_t, w_f;
    //printf("iCntNode: %d\n",iCntNode);
    while (1) {
		int idx = rand() % iCntNode;
		if (iLast == idx) continue;
		iLast = idx;
		if (idx < iCntNode - 2) {
			ph = RandomMinMax(6500, 8500) / 1000;
			nh3 = RandomMinMax(1000, 3000) / 100000;
			h2s = RandomMinMax(500, 1000) / 100000;
			no2 = RandomMinMax(2000, 5000) / 10000;
			w_t = RandomMinMax(20000, 33000) / 1000;
			w_f = RandomMinMax(2000, 3000) / 10000;
		}
		else {
			ph = RandomMinMax(3500, 10500) / 1000;
			nh3 = RandomMinMax(1000, 30000) / 100000;
			h2s = RandomMinMax(500, 15000) / 100000;
			no2 = RandomMinMax(2000, 35000) / 10000;
			w_t = RandomMinMax(10000, 38000) / 1000;
			w_f = RandomMinMax(2000, 33000) / 10000;
		}
		HFNode* pNode = &pGate->lstNode[idx];
		szKey = StringFormat("%s", pNode->szNodeKey.c_str());
		szJsonSensor = StringFormat("{\"ph\":%0.2lf,\"nh3\":%0.4lf,\"h2s\":%0.4lf,\"no2\":%0.2lf,\"water_temp\":%0.2lf,\"water_flow\":%0.2lf}",
			ph, nh3, h2s, no2, w_t, w_f);

		api.Sensor_Create(szKey, szJsonSensor);

		Sleep(2000);
	}
}

void CMainThread::InitMongoSensorRandom(){
	CHFactorClient* pClient = m_pPoolClient->Pop();
	CHFactorDatabase* pDB = pClient->GetDatabase();

	pDB->GetHFUser("admin@admin.com",m_HFUser);
    //printf ("uid:%lu gid:%u\n",m_HFUser.uid,m_HFUser.gid);
    //printf ("%s %s %s\n",m_HFUser.szUsername.c_str(),m_HFUser.szFullname.c_str(),m_HFUser.szEmail.c_str());          

	pDB->GetGateways(m_HFUser.uid,m_lstGate,0,2);
	int iCntGate = (int)m_lstGate.size();
	//printf("GATE COUNT:%d\n", iCntGate);

	//if (iCntGate > 2) iCntGate = 2;
	for (int i = 0; i < iCntGate; ++i) {
		HFGate* pGate = &m_lstGate[i];

        //struct tm tmLocal;
        //localtime_r(&pGate->date_created,&tmLocal);
        //char buffer [32];
        //buffer[0] = 0;
        //strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", &tmLocal);
        //printf("[-] %lu %s (%s) [%s] %s\n",pGate->gid,
        //pGate->szGateId.c_str(),pGate->szDesc.c_str(),pGate->szGateKey.c_str(),buffer);

		pDB->GetNodes(pGate->uid, pGate->gid, pGate->lstNode);
		//int iCntNode = (int)pGate->lstNode.size();
		//printf(" [+] idx:%d GATE[%lu] NODE COUNT:%d\n",i,pGate->gid, iCntNode);		
	}

	m_pPoolClient->Push(pClient);

	if (iCntGate < 2) return;
    pthread_t uTD;
    pthread_create(&uTD,NULL,_ThreadMongoSensorRandom,&m_lstGate[0]);
    pthread_create(&uTD,NULL,_ThreadMongoSensorRandom,&m_lstGate[0]);
    pthread_create(&uTD,NULL,_ThreadMongoSensorRandom,&m_lstGate[1]);
    pthread_create(&uTD,NULL,_ThreadMongoSensorRandom,&m_lstGate[1]);

}
void* CMainThread::_ThreadMongoSensorRandom(void* lpData){
    HFGate* pGate = (HFGate*)lpData;
    theApp.OnThreadMongoSensorRandom(pGate);
    return NULL;
}
void CMainThread::OnThreadMongoSensorRandom(HFGate* pGate){
	srand((uint32_t)time(NULL));
	int iCntNode = (int)pGate->lstNode.size();
	int iLast = 0;
	double ph, nh3, h2s, no2, w_t, w_f,do_;
	uint32_t delay = 2000;
	while (1){
		int idx = rand() % iCntNode;
		if (idx == 0 || iLast == idx) continue;
		iLast = idx;
		
		Sleep(delay);
		delay = 2000;

		CHFactorClient* pClient = m_pPoolClient->Pop();
		if (!pClient) continue;
		CHFactorDatabase* pDB = pClient->GetDatabase();
		
		if (idx < iCntNode - 2) {
			ph = RandomMinMax(6500, 8500) / 1000;
			nh3 = RandomMinMax(1000, 3000) / 100000;
			h2s = RandomMinMax(500, 1000) / 100000;
			no2 = RandomMinMax(2000, 5000) / 10000;
			w_t = RandomMinMax(20000, 33000) / 1000;
			w_f = RandomMinMax(2000, 3000) / 10000;
			do_ = RandomMinMax(4000, 9000) / 1000;
		}
		else {
			ph = RandomMinMax(3500, 10500) / 1000;
			nh3 = RandomMinMax(1000, 30000) / 100000;
			h2s = RandomMinMax(500, 15000) / 100000;
			no2 = RandomMinMax(2000, 35000) / 10000;
			w_t = RandomMinMax(10000, 38000) / 1000;
			w_f = RandomMinMax(2000, 33000) / 10000;
			do_ = RandomMinMax(1500, 7000) / 1000;
		}
		HFNode* pNode = &pGate->lstNode[idx];
		CBSONObject oSensor;
		oSensor.AddMember("ph",ph);
		oSensor.AddMember("nh3",nh3);
		oSensor.AddMember("h2s",h2s);
		oSensor.AddMember("no2",no2);
		oSensor.AddMember("do",do_);
		oSensor.AddMember("water_temp",w_t);
		oSensor.AddMember("water_flow",w_f);
		uint64_t sid;
		if (pDB->AddSensor(pNode->gid,pNode->nid,oSensor,sid)){
			//printf("AddSensor(gid:%lu,nid:%lu) => sid[%lu] : OKIE\n",pNode->gid,pNode->nid,sid);
		}

		m_pPoolClient->Push(pClient);
	}
}

void CMainThread::InitMongoTesting(){
	//mongoc_init ();
	//m_pClient = new CMongoClient("mongodb://factor:factor%40123@127.0.0.1:27017/","factor_db");
    //m_pPoolClient = new CHFactorClientPool("mongodb://factor:factor%40123@127.0.0.1:27017/","factor_db");
	
	//createIndexes Database;
	CHFactorClient* pClient = m_pPoolClient->Pop();
	CHFactorDatabase* pDB = pClient->GetDatabase();
	//pDB->InitIndexes();

	//pDB->AddGroup("Administrators","H-Factor Admin Group");
	//pDB->AddGroup("Operators","H-Factor Operator Group");
	//pDB->AddGroup("User","H-Factor User Group");

	//CConfigCollection* pCol_config_test = pDB->GetConfigCollection();
	//CGroupCollection*  pCol_group_test = pDB->GetGroupCollection();
	//printf("[%s]\n",pCol_config_test->_me.c_str());
	//GROUP
	/*CBSONObject oKey1;
	oKey1.AddMember("gid",1);

	pCol_group_test->CreateIndexes(&oKey1,true);
	CBSONObject oKey2;
	oKey2.AddMember("name",1);
	oKey2.AddMember("gid",-1);
	pCol_group_test->CreateIndexes(&oKey2);
	int64_t uInc = 0;
	for (int i = 0; i < 10; ++i){
		if (pCol_config_test->IncreaseField("nextGroupId",uInc)){
			CBSONObject oDoc;
			oDoc.AddMember("gid",uInc);
			oDoc.AddMember("name",StringFormat("NAME Group %ld",uInc).c_str());
			oDoc.AddMember("desc",StringFormat("DESC Group %ld",uInc).c_str());
			pCol_group_test->InsertOne(&oDoc);
		}
	}
*/

	pDB->GetHFUser("admin@admin.com",m_HFUser);
    printf ("uid:%lu gid:%u\n",m_HFUser.uid,m_HFUser.gid);
    printf ("%s %s %s\n",m_HFUser.szUsername.c_str(),m_HFUser.szFullname.c_str(),m_HFUser.szEmail.c_str());          

	//pDB->GetGateways(m_HFUser.uid);
	//pDB->GetNodes(m_HFUser.uid,m_HFUser.gid);
	/*CBSONObject oSensor;
	oSensor.AddMember("ph",4.57);
	oSensor.AddMember("no2",1.95);
	pDB->AddSensor(2,47,oSensor);

	m_pPoolClient->Push(pClient);*/

	/*pthread_t uTD;
	for (int i = 0; i < 10; ++i){
		pthread_create(&uTD,NULL,_ThreadMongoTesting,&i);
		Sleep(10);
	}*/
}
void* CMainThread::_ThreadMongoTesting(void* lpData){   
	//mongoc_init (); 
    theApp.OnThreadMongoTesting(*(int*)lpData);
    return NULL;
}
void CMainThread::OnThreadMongoTesting(int index){
	return;
/*	CMongoClient client("mongodb://factor:factor%40123@127.0.0.1:27017/","factor_db");	
	CMongoCollection col_config_test("config_test",&client);	
	CMongoCollection col_group_test("group_test",&client);	
	int64_t uInc = 0;
	while (1){
		if (col_config_test.IncreaseField("nextGroupId",uInc)){
			CBSONObject oDoc;
			oDoc.AddMember("gid",uInc);
			oDoc.AddMember("name",StringFormat("NAME Group %ld",uInc).c_str());
			oDoc.AddMember("desc",StringFormat("DESC Group %ld",uInc).c_str());
			col_group_test.InsertOne(&oDoc);
		}
		Sleep(100);
	}
*/
	uint64_t uInc = 0;
	while (1){
		Sleep(100);
		CMongoClient* pClient = m_pPoolClient->Pop();
		if (!pClient) continue;

		CMongoCollection* pCol_config_test = pClient->GetCollection("config_test");	
		CMongoCollection* pCol_group_test = pClient->GetCollection("group_test");

		if (pCol_config_test->IncreaseField("nextGroupId",uInc)){
			CBSONObject oDoc;
			oDoc.AddMember("gid",(int64_t)uInc);
			oDoc.AddMember("name",StringFormat("NAME Group %ld",uInc).c_str());
			oDoc.AddMember("desc",StringFormat("DESC Group %ld",uInc).c_str());
			pCol_group_test->InsertOne(&oDoc);
		}

		/*CMongoDatabase* pDatabase = pClient->GetDatabase();
		printf("%02d - [%lX] [%p][%p] [%p][%p]\n",index,pthread_self(),
		pClient,pClient->Get(),
		pDatabase,pDatabase->Get());*/

		m_pPoolClient->Push(pClient);
	}
}
