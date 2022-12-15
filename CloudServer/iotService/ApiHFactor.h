#pragma once

#include "HttpClient.h"

typedef struct _APIResponse {
	int r;		
	std::string m;
}API_RESPONSE;

typedef struct _HFUser {
public:
	uint64_t	uid;
	uint32_t	gid;	
	std::string	szUsername;
	std::string	szEmail;
	std::string	szFullname;
}HFUser;

typedef struct _HFNode
{	
	uint64_t	nid;
	uint64_t	gid;
	std::string	szNodeId;
	std::string	szDesc;
	time_t		date_created;
	std::string	szNodeKey;
}HFNode;

typedef struct _HFGate
{
	uint64_t	uid;
	uint64_t	gid;
	std::string	szGateId;
	std::string	szDesc;
	time_t		date_created;
	std::string	szGateKey;

	std::vector<HFNode> lstNode;
}HFGate;

class CApiHFactor: public CHttpClient
{
public:
	CApiHFactor();
	virtual ~CApiHFactor();

    bool User_Get(const char* czEmail, HFUser& HFUser);
    bool User_Gate_Gets(std::vector<HFGate>& lstGate);
	bool User_Gate_Gets(uint64_t uUserID, std::vector<HFGate>& lstGate);

	bool User_Gate_Node_Gets(uint64_t uGWID, std::vector<HFNode>& lstNode);
	bool User_Gate_Node_Gets(uint64_t uUserID, uint64_t uGWID,std::vector<HFNode>& lstNode);

    bool Sensor_Create(std::string szNodeKeyHWT, std::string szJsonSensors);
protected:
    bool SetError(int iError, const char* lpszErrorText);
    bool ReturnServiceFailed(std::string& szContentA);
    bool ParseApiResponse(std::string& szContentA, API_RESPONSE& pAIGResponse,bson_t* bson = NULL);

    CHttpClient*	m_pHttp;
    int				m_iErrorCode;
	std::string		m_szErrorText;

	CJWT*			m_pJWTAdmin;
	std::string		m_szTokenHFT;
    HFUser			m_HFUser;
};