#include "../inc/HFactor.h"
#include "HttpClient.h"

CHttpClient::CHttpClient(const char* lpszAgent, const char* lpszCookieFile)
{
	m_urlCode = CURLE_OK;
	m_statusCode = 0;
	
	m_pCurl = curl_easy_init();
	if (!m_pCurl){
		//printf("Error curl_easy_init()\n");
		return;
	}
	if (lpszAgent)	curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, lpszAgent);
	if (lpszCookieFile){
		curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, lpszCookieFile);
		curl_easy_setopt(m_pCurl, CURLOPT_COOKIEJAR, lpszCookieFile);
		curl_easy_setopt(m_pCurl, CURLOPT_COOKIESESSION, true);
	}
	curl_easy_setopt(m_pCurl, CURLOPT_AUTOREFERER, true);
	
	// Decompress GZIP
	//curl_easy_setopt(m_pCurl, CURLOPT_ENCODING, "gzip, deflate");

	//SSL do not check
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, false);
}

CHttpClient::~CHttpClient()
{
	if (m_pCurl) curl_easy_cleanup(m_pCurl);
}

bool CHttpClient::SendRequest(const char* lpURL, std::string& szContentA, const char* lpPostData, const char* lpHeaders, const char* lpCookies)
{
	if (lpURL == NULL || lpURL[0] == 0) return false;
	//printf("SendRequest(%s)\n", lpURL);
	szContentA.clear();

	m_statusCode = 0;

	curl_easy_setopt(m_pCurl, CURLOPT_URL, lpURL);
	
	struct curl_slist * plstHeaders = NULL;
	if (lpHeaders) {			
		plstHeaders = curl_slist_append(plstHeaders, lpHeaders);
		if (lpPostData) plstHeaders = curl_slist_append(plstHeaders, "Content-Type: application/x-www-form-urlencoded");
		curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, plstHeaders);
	}
	else {
		//curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, NULL);		 
	}

	if (lpCookies) {
		curl_easy_setopt(m_pCurl, CURLOPT_COOKIE, lpCookies);
	}

	// Auto Re-direct URL
	curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);
	//curl_easy_setopt(m_pCurl, CURLOPT_MAXREDIRS, 10);

	if (lpPostData) {
		curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(m_pCurl, CURLOPT_POST, 1);
		curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, lpPostData);
	}
	else {
		//curl_easy_setopt(m_pCurl, CURLOPT_AUTOREFERER, true);
	}

	// Thiết lập buffer lưu nội dung
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, _OnWriteCallback);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &szContentA);

	//curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, fnWriteHeaderFunction);
	//LogWriteA("SendRequest() ... 1");
	m_urlCode = curl_easy_perform(m_pCurl);
	//LogWriteA("SendRequest() ... 2 m_dwErrorCode:%d",m_dwErrorCode);

	if (plstHeaders) {
		curl_slist_free_all(plstHeaders); /* free the header list */
		curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, NULL);
	}
	
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, NULL);

	if (m_urlCode != CURLE_OK) return false;

	curl_easy_getinfo(m_pCurl, CURLINFO_HTTP_CODE, &m_statusCode);
	return true;
}

size_t CHttpClient::_OnWriteCallback(void *data, size_t size, size_t nmemb, void *userp)
{	
	std::string* pStr = (std::string*)userp;
	std::string s((char*)data,nmemb);
	pStr->append(s);
	return size * nmemb;
}

long CHttpClient::GetStatusCode(){
	return m_statusCode;
}
CURLcode CHttpClient::GetURLCode(){
	return m_urlCode;
}
std::string	CHttpClient::GetURLCodeText(){
	return std::string(curl_easy_strerror(m_urlCode));
}