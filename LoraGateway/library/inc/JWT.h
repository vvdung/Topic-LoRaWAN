#pragma once

enum JWTType {jwt_base64url = 0,jwt_xor,jwt_aes};

class CJWT
{
public:
	typedef struct _JWTDATA{
		std::string szHeader;
		std::string szPayload;
	}JWTDATA;

	CJWT(const char* lpszKey = NULL);
	~CJWT(void);

	std::string Create(const char* lpszPayload,JWTType type = jwt_base64url,const char* lpszKeyPayload = NULL);
	bool	    Verify(const char* lpszJWT,const char* lpszKeyPayload = NULL,JWTDATA* pJWTData = NULL);
	std::string GetPayload(const char* lpszJWT,std::string* pszHeader = NULL, const char* lpszKeyJWT = NULL);
protected:
	std::string m_szKey;
};