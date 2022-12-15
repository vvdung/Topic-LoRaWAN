#pragma once
#include <curl/curl.h>

#define API_STATUS_CODE_OK		200
#define API_STATUS_CODE_ERROR	302
#define API_STATUS_CODE_BADGW	502
#define API_ENCRYPT_RESULT		1

class CHttpClient
{
public:

	CHttpClient(const char* lpszAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:57.0) Gecko/20100101 Firefox/57.0",
		const char* lpszCookieFile = "cookieFile.bin");
	virtual ~CHttpClient();

	bool SendRequest(const char* lpURL, std::string& szContentA, const char* lpPostData = NULL, const char* lpHeaders = NULL, const char* lpCookies = NULL);
	long		GetStatusCode();
	CURLcode	GetURLCode();
	std::string	GetURLCodeText();
protected:	
	static size_t _OnWriteCallback(void *ptr, size_t size, size_t nmemb, void *userp);
	
	CURL*		m_pCurl;	
	CURLcode	m_urlCode;
	long		m_statusCode;	
};