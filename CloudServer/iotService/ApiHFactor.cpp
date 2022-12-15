#include "main.h"
#include "ApiHFactor.h"

CApiHFactor::CApiHFactor():CHttpClient(){
	m_iErrorCode = 0;
	m_pJWTAdmin = new CJWT("#pi$xV4nYURmAYsYTJR7Vxvp-0aMYVCN");	
	m_pHttp = new CHttpClient();
	m_szTokenHFT = "";
	m_HFUser.uid = 0;
	m_HFUser.gid = 0;	
}
CApiHFactor::~CApiHFactor() {
	delete m_pJWTAdmin;
	delete m_pHttp;
}

bool CApiHFactor::User_Get(const char* czEmail, HFUser& HFUser){
    std::string szEmail(czEmail);
	if (szEmail.size() < 3 || szEmail.find("@") < 0) {
		return SetError(-1001, "Email Invalid");
	}
    std::string m = StringFormat("m={\"email\":\"%s\"}",szEmail.c_str());
    std::string szContentA;
    //
    if (!m_pHttp->SendRequest("http://api.h-factor.vn/v1/user/get", szContentA, m.c_str())) {
    //if (!m_pHttp->SendRequest("https://emproxy.net/api/v1/user/get", szContentA, m.c_str())) {
		return SetError(-1003, m_pHttp->GetURLCodeText().c_str());
	}
    long dwCode = m_pHttp->GetStatusCode();
    
	if (dwCode == API_STATUS_CODE_ERROR) return ReturnServiceFailed(szContentA);
	if (dwCode != API_STATUS_CODE_OK) return SetError(dwCode, "Service Error");

    bson_t b;
	API_RESPONSE apiRes;
	if (!ParseApiResponse(szContentA, apiRes, &b)) return false;// SetError(dwCode, L"Lỗi ParseApiResponse(APIRES)");
    
	CBSONIter it;
	CBSONObject o(&b);

	if (o.KeyExist("uid", &it)) HFUser.uid = it.GetInt64();
	if (o.KeyExist("gid", &it)) HFUser.gid = it.GetInt();
	if (o.KeyExist("username", &it)) HFUser.szUsername = it.GetUTF8().c_str();
	if (o.KeyExist("email", &it)) HFUser.szEmail = it.GetUTF8().c_str();
	if (o.KeyExist("fullname", &it)) HFUser.szFullname = it.GetUTF8().c_str();

    return true;
}
bool CApiHFactor::User_Gate_Gets(std::vector<HFGate>& lstGate){
	if (m_szTokenHFT.empty()) return SetError(-1001, "Must logined");
	return User_Gate_Gets(m_HFUser.uid, lstGate);
}

bool CApiHFactor::User_Gate_Gets(uint64_t uUserID, std::vector<HFGate>& lstGate){
	
	std::string m = StringFormat("m={\"uid\":%lu}", uUserID);

	lstGate.clear();

	std::string szContentA;

	if (!m_pHttp->SendRequest("http://api.h-factor.vn/v1/user/gate/gets", szContentA, m.c_str())) {
		return SetError(-1003, m_pHttp->GetURLCodeText().c_str());
	}
	long dwCode = m_pHttp->GetStatusCode();
	//LOGA("HTTP StatusCode: %d", dwCode);
	//LOGA("[%s]", szContentA);
	if (dwCode == API_STATUS_CODE_ERROR) return ReturnServiceFailed(szContentA);
	if (dwCode != API_STATUS_CODE_OK) return SetError(dwCode, "Service error");

    bson_t b;
	API_RESPONSE apiRes;
	if (!ParseApiResponse(szContentA, apiRes, &b)) return false;// SetError(dwCode, L"Lỗi ParseApiResponse(APIRES)");

    CBSONObject oNode(&b);
	CBSONIter it;
	if (!oNode.KeyExist("data", &it)) return SetError(-2001, "Data is not found");;
	if (!it.IsArray()) return SetError(-2002, "Data is not ARRAY");
	it.Recurse(&it);
	CBSONIter itTemp;
	bson_t bTemp;
	while (it.Next()) {
		//LOGA("Found KEY[%s] ",it.GetKey().c_str());
		if (!it.GetObject(&bTemp)) continue;
		CBSONObject oTemp(&bTemp);
		HFGate hfGate;
		if (oTemp.KeyExist("gid", &itTemp)) hfGate.gid = itTemp.GetInt64();
		if (oTemp.KeyExist("uid", &itTemp)) hfGate.uid = itTemp.GetInt64();
		if (oTemp.KeyExist("gwid", &itTemp)) hfGate.szGateId = itTemp.GetUTF8().c_str();
		if (oTemp.KeyExist("description", &itTemp)) hfGate.szDesc = itTemp.GetUTF8().c_str();
		if (oTemp.KeyExist("date_created", &itTemp)) hfGate.date_created = itTemp.GetInt64() / 1000;
		if (oTemp.KeyExist("gwkey", &itTemp)) hfGate.szGateKey = itTemp.GetUTF8().c_str();

		lstGate.push_back(hfGate);
		//LOGA(" [+] %lu %lu KEY:%s", hfNode.nid, hfNode.gid, hfNode.szNodeKey);
	}
    return true;
}

bool CApiHFactor::User_Gate_Node_Gets(uint64_t uGWID, std::vector<HFNode>& lstNode){
	if (m_szTokenHFT.empty()) return SetError(-1001, "Must Logined");
	return User_Gate_Node_Gets(m_HFUser.uid, uGWID, lstNode);
}
bool CApiHFactor::User_Gate_Node_Gets(uint64_t uUserID, uint64_t uGWID,std::vector<HFNode>& lstNode){
	std::string m = StringFormat("m={\"uid\":%lu,\"gid\":%lu}", uUserID, uGWID);
	lstNode.clear();

	std::string szContentA;

	if (!m_pHttp->SendRequest("http://api.h-factor.vn/v1/user/gate/node/gets", szContentA, m.c_str())) {
		return SetError(-1003, m_pHttp->GetURLCodeText().c_str());
	}

	long dwCode = m_pHttp->GetStatusCode();
	if (dwCode == API_STATUS_CODE_ERROR) return ReturnServiceFailed(szContentA);
	if (dwCode != API_STATUS_CODE_OK) return SetError(dwCode, "Service error");

    bson_t b;
	API_RESPONSE apiRes;
	if (!ParseApiResponse(szContentA, apiRes, &b)) return false;// SetError(dwCode, L"Lỗi ParseApiResponse(APIRES)");

    CBSONObject oNode(&b);
	CBSONIter it;
	if (!oNode.KeyExist("data", &it)) return SetError(-2001, "Data is not found");;
	if (!it.IsArray()) return SetError(-2002, "Data is not ARRAY");
	it.Recurse(&it);
	CBSONIter itTemp;
	bson_t bTemp;
	while (it.Next()) {
		//LOGA("Found KEY[%s] ",it.GetKey().c_str());
		if (!it.GetObject(&bTemp)) continue;
		CBSONObject oTemp(&bTemp);
		HFNode hfNode;
		if (oTemp.KeyExist("nid", &itTemp)) hfNode.nid = itTemp.GetInt64();
		if (oTemp.KeyExist("gid", &itTemp)) hfNode.gid = itTemp.GetInt64();
		if (oTemp.KeyExist("nodeid", &itTemp)) hfNode.szNodeId = itTemp.GetUTF8().c_str();
		if (oTemp.KeyExist("description", &itTemp)) hfNode.szDesc = itTemp.GetUTF8().c_str();
		if (oTemp.KeyExist("date_created", &itTemp)) hfNode.date_created = itTemp.GetInt64()/1000;
		if (oTemp.KeyExist("nkey", &itTemp)) hfNode.szNodeKey = itTemp.GetUTF8().c_str();

		lstNode.push_back(hfNode);
		//LOGA(" [+] %lu %lu KEY:%s", hfNode.nid, hfNode.gid, hfNode.szNodeKey);
	}
	return true;
}


bool CApiHFactor::Sensor_Create(std::string szNodeKeyHWT, std::string szJsonSensors){
    std::string szKeyHWT(szNodeKeyHWT);	
	CHFT hft;
	uint8_t* pData = hft.Decode(szKeyHWT.c_str());
	if (!pData) return SetError(-1001, "Key Error");
	std::string szDecode = hft.ToString();
    std::string szSensor = szDecode;

    bson_error_t error;
	bson_t* b = CBSONObject::Parse(szDecode.c_str(), &error);
	if (!b) return SetError(-1002, "Key invalid");    
	bson_destroy(b);
	b = CBSONObject::Parse(szJsonSensors.c_str(), &error);
	if (!b) return SetError(-1003, "Sensor invalid");
	bson_destroy(b);

	std::string szValue = StringFormat(",\"s\":%s",szJsonSensors.c_str());
	szDecode.insert(szDecode.size() - 1, szValue);

    //printf("[%s] \n%s\n",szSensor.c_str(),szDecode.c_str());
    
	std::string jwt_m = m_pJWTAdmin->Create(szDecode.c_str(), jwt_xor);
	std::string m = StringFormat("p=2&m=%s", jwt_m.c_str());

	//printf("[%s]\n",m.c_str());

    std::string szContentA;

	if (!m_pHttp->SendRequest("http://api.h-factor.vn/v1/sensor/add", szContentA, m.c_str())) {
		return SetError(-1003, m_pHttp->GetURLCodeText().c_str());
	}
	long dwCode = m_pHttp->GetStatusCode();
	//printf("HTTP StatusCode: %ld\n", dwCode);
	//printf("[%s]\n", szContentA.c_str());
	if (dwCode == API_STATUS_CODE_ERROR) return ReturnServiceFailed(szContentA);
	if (dwCode != API_STATUS_CODE_OK) return SetError(dwCode, "Service Error");

	API_RESPONSE apiRes;
	if (!ParseApiResponse(szContentA, apiRes)) return false;// SetError(dwCode, L"Lỗi ParseApiResponse(APIRES)");

	printf("r = %d [%s] %s\n", apiRes.r, apiRes.m.c_str(),szSensor.c_str());

	return (apiRes.r == 0);
}
//////////////////////
bool CApiHFactor::SetError(int iError, const char* lpszErrorText){
    m_iErrorCode = iError;
	m_szErrorText = lpszErrorText;
	return false;
}

bool CApiHFactor::ReturnServiceFailed(std::string& szContentA){
	bson_error_t error;
	bson_t* b = CBSONObject::Parse(szContentA.c_str(), &error);
	if (!b) return SetError(error.code, error.message);
	CBSONObject o(b);
	CBSONIter it;
	m_iErrorCode = -1;
	if (o.KeyExist("r", &it)) m_iErrorCode = it.GetInt();
	if (o.KeyExist("m", &it)) m_szErrorText = it.GetUTF8().c_str();
	return false;	
}
 bool CApiHFactor::ParseApiResponse(std::string& szContentA, API_RESPONSE& apiResponse,bson_t* bson){
	apiResponse.r = -1;
	apiResponse.m = "";
	//LOGA("ParseApiResponse----\n%s ", szContentA);
	bson_error_t error;
	bson_t* b = CBSONObject::Parse(szContentA.c_str(), &error);
	if (!b) return SetError(error.code, error.message);
	CBSONIter it;
	CBSONObject o(b);
	
	if (!o.KeyExist("r", &it)) return SetError(-2000, "Where r field?");
	if (!BSON_ITER_HOLDS_INT(&it)) return SetError(-2001, "r field wrong type");
	apiResponse.r = it.GetInt();
	//LOGA("r = %d", apiResponse.r);

	if (!o.KeyExist("m", &it)) return SetError(-2002, "Where m field?");
	if (apiResponse.r < 0) {		
		if (!BSON_ITER_HOLDS_UTF8(&it)) return SetError(-2003, "m must string");
		apiResponse.m = it.GetUTF8().c_str();
		return true;
	}
	if (apiResponse.r == 0) {
		if (BSON_ITER_HOLDS_DOCUMENT(&it)){
            if (bson) it.GetObject(bson);                            
            else apiResponse.m = it.GetSubDocument().c_str();
        }
		else if (BSON_ITER_HOLDS_UTF8(&it)) apiResponse.m = it.GetUTF8().c_str();
		else return SetError(-2004, "m must document/string");
		return true;
	}

	if (!BSON_ITER_HOLDS_UTF8(&it)) return SetError(-2005, "m must string");
	std::string mValue = it.GetUTF8();
	//LOGA("1. %s", mValue.c_str());	
	apiResponse.m = m_pJWTAdmin->GetPayload(mValue.c_str()).c_str();
	return true;
 }