#include "../inc/HFactor.h"
#include "../inc/HS256.h"
#include "../inc/openssl/x509.h"
#include "../inc/openssl/hmac.h"

CHS256::CHS256(const char* lpszKey)
{	
	if (!lpszKey || lpszKey[0] == 0)
		m_szKey = "ee4a9521a4f4a352649754932e8abd8c";
	else{        
        char* p = (char*)lpszKey;
		if (strlen(lpszKey) > 32) p[32] = 0;
        m_szKey = p;        
	}
}

CHS256::~CHS256(void)
{
    m_Bytes.clear();
}

std::string CHS256::GetHash( const char* lpszText )
{
	if (Encode(lpszText)) return ToBase64Url();
	return std::string("");
}

bool CHS256::Encode( uint8_t* pData, int iSize )
{
	m_Bytes.clear();
	if (pData == NULL || iSize <= 0) return false;
	
	int text_len = iSize;
	int key_len = m_szKey.size();

	uint8_t *text = pData;
	uint8_t *key = (uint8_t*)m_szKey.c_str();

	const EVP_MD* md_ = EVP_sha256();
	int num_signature_ = EVP_MD_size(md_);
	uint8_t* signature = new uint8_t[num_signature_];
	HMAC_CTX *ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, key, key_len, md_, NULL);
	HMAC_Update(ctx, text, text_len);
	HMAC_Final(ctx, signature, (uint32_t*)&num_signature_);
	HMAC_CTX_free(ctx);

	for (int i = 0; i < num_signature_; ++i) m_Bytes.push_back(signature[i]);	
	delete []signature;
	return true;
}

bool CHS256::Encode( const char* lpszText )
{
	if (!lpszText || lpszText[0] == 0) return false;
	return Encode((uint8_t*)lpszText,strlen(lpszText));
}

std::string CHS256::ToHexString()
{
	if (m_Bytes.size() == 0) return std::string("");
	std::string strHex;
    char czt[8];
	for (int i = 0; i < (int)m_Bytes.size(); ++i)
	{
		sprintf(czt,"%02x",m_Bytes[i]);
		strHex += czt;
	}
	return std::string(strHex);
}

std::string CHS256::ToBase64Url()
{
	if (m_Bytes.size() == 0) return std::string("");
	CBase64Url base64Url;
	if (!base64Url.Encode(m_Bytes.data(),(int)m_Bytes.size())) return std::string("");
	std::vector<uint8_t>* pBytes = base64Url.GetBytes();	
	return std::string((char*)pBytes->data(),pBytes->size());
}