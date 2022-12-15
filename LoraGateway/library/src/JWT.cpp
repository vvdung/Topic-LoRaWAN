#include "../inc/HFactor.h"
#include "../inc/JWT.h"

CJWT::CJWT(const char* lpszKey)
{
	if (!lpszKey || lpszKey[0] == 0)
		m_szKey = "ee4a9521a4f4a352649754932e8abd8c";
	else{        
        char* p = (char*)lpszKey;
		if (strlen(lpszKey) > 32) p[32] = 0;
        m_szKey = p;        
	}
}

CJWT::~CJWT(void){}

std::string CJWT::Create(const char* lpszPayload,JWTType type,const char* lpszKeyPayload)
{	
	if (!lpszPayload || lpszPayload[0] == 0) return std::string("");
    std::string szPayload(lpszPayload);	
	bool bKeyPayload = (lpszKeyPayload && lpszKeyPayload[0] != 0);
    std::string szKeyPayload,szHeader,szPayload64Url;
    if (bKeyPayload) szKeyPayload = lpszKeyPayload;
    else szKeyPayload = m_szKey;
	
	switch (type){
		case jwt_xor:{
			if (!bKeyPayload){
				//szHeader = StringFormat("{\"alg\": \"HS256\",\"typ\": \"JWT\",\"t\":%d}",type);
				szHeader = StringFormat("{\"t\":%d}",type);				
			}else{
				CXOR4 xor_(m_szKey.c_str());
				xor_.Encode((uint8_t*)szKeyPayload.c_str(),szKeyPayload.size());
				//szHeader = StringFormat("{\"alg\": \"HS256\",\"typ\": \"JWT\",\"t\":%d,\"k\":\"%s\"}",type,xor.ToBase64Url());
				szHeader = StringFormat("{\"t\":%d,\"k\":\"%s\"}",type,xor_.ToBase64Url(false).c_str());
			}

			CXOR4 xor_(szKeyPayload.c_str());
			xor_.Encode((uint8_t*)szPayload.c_str(),szPayload.size());
			szPayload64Url = xor_.ToBase64Url(false);            
					 }
			break;
		case jwt_aes:{
			if (!bKeyPayload){
				//szHeader = StringFormat("{\"alg\": \"HS256\",\"typ\": \"JWT\",\"t\":%d}",type);
				szHeader = StringFormat("{\"t\":%d}",type);				
			}else{
				CAES aes(m_szKey.c_str());
				aes.Encode(szKeyPayload.c_str());
				//szHeader = StringFormat("{\"alg\": \"HS256\",\"typ\": \"JWT\",\"t\":%d,\"k\":\"%s\"}",type,aes.ToBase64Url());
				szHeader = StringFormat("{\"t\":%d,\"k\":\"%s\"}",type,aes.ToBase64Url().c_str());
			}

			CAES aes(szKeyPayload.c_str());
			aes.Encode(szPayload.c_str());
			szPayload64Url = aes.ToBase64Url();
					 }
			break;
		default:{
			//szHeader = "{\"alg\": \"HS256\",\"typ\": \"JWT\",\"t\":0}";
			szHeader = "{\"t\":0}";
            //szHeader = "{\"alg\": \"HS256\"}";
			CBase64Url base64url;
            base64url.Encode((uint8_t*)lpszPayload,strlen(lpszPayload));
			szPayload64Url = base64url.ToString();//(char*)base64url.GetBytes()->data();
				}
			break;
	}
    
	CBase64Url base64url;
    base64url.Encode((uint8_t*)szHeader.c_str(),szHeader.size());
	std::string szHeader64Url = base64url.ToString();//(char*)base64url.GetBytes()->data();
	std::string szMsg = szHeader64Url + '.' + szPayload64Url;
	CHS256 hs256(m_szKey.c_str());
	std::string szSign = hs256.GetHash(szMsg.c_str());
    std::string szJWT = szMsg + "." + szSign;
	return std::string(szJWT);
}

bool CJWT::Verify(const char* lpszJWT,const char* lpszKeyPayload,JWTDATA* pJWTData)
{
    //Analysic JWT
	if (!lpszJWT || lpszJWT[0] == 0) return false;
    std::vector<std::string> JWTs = SplitString(lpszJWT,".");
	if (JWTs.size() != 3) return false;

	std::string szKeyPayload(m_szKey);
	if (lpszKeyPayload && lpszKeyPayload[0] != 0) szKeyPayload = lpszKeyPayload;
	std::string szMsg = JWTs[0] + "." + JWTs[1];
	CHS256 hs256(szKeyPayload.c_str());
	std::string szSign = hs256.GetHash(szMsg.c_str());	

	if (szSign.compare(JWTs[2]) != 0) return false;
	if (pJWTData){
		pJWTData->szHeader = JWTs[0];
		pJWTData->szPayload = JWTs[1];
	}
	return true;
}
std::string CJWT::GetPayload(const char* lpszJWT, std::string* pszHeader, const char* lpszKeyJWT){
	JWTDATA jwtData;
	if (!Verify(lpszJWT,lpszKeyJWT,&jwtData)) return std::string("");
	CBase64Url base64url;
	base64url.Decode((uint8_t*)jwtData.szHeader.c_str(),jwtData.szHeader.size());    
    std::string szHeader = base64url.ToString();
    bson_error_t error;
	
	bson_t* b = CBSONObject::Parse(szHeader.c_str(), &error);
	if (!b) return std::string("");
	CBSONObject o(b);
    CBSONIter it;
    if (!o.KeyExist("t",&it)) return std::string("");
	
    if (pszHeader) *pszHeader = szHeader;

	std::string szKeyPayload = m_szKey;
	if (lpszKeyJWT && lpszKeyJWT[0] != 0) szKeyPayload = lpszKeyJWT;	

	int type = it.GetInt();    
	switch (type){
		case jwt_xor:{
			if (o.KeyExist("k",&it)){
				CXOR4 xor_(m_szKey.c_str());  
                std::string key4 = it.GetUTF8();                        
				szKeyPayload = xor_.DecodeBase64Url(key4.c_str(),false);
				//if (pszHeader) (*pszHeader).replace(key4)				
			}
			CXOR4 xor_(szKeyPayload.c_str());
            return xor_.DecodeBase64Url(jwtData.szPayload.c_str(),false);                        
					 }
			break;
		case jwt_aes:{
			if (o.KeyExist("k",&it)){
				CAES aes(m_szKey.c_str());
				szKeyPayload = (char*)aes.DecodeBase64Url(it.GetUTF8().c_str());
			}
			CAES aes(szKeyPayload.c_str());
            aes.DecodeBase64Url(jwtData.szPayload.c_str());
            return aes.ToString();
					 }
			break;
		default:
			base64url.Decode((uint8_t*)jwtData.szPayload.c_str(),jwtData.szPayload.size());
			return base64url.ToString();
            break;
	}	
	return std::string("");
}