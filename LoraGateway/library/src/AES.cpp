#include "../inc/HFactor.h"
#include "../inc/AES.h"
#include "../inc/openssl/aes.h"
#include "../inc/openssl/evp.h"
#include "../inc/openssl/err.h"

CAES::CAES(const char* lpszKey){
	if (!lpszKey || lpszKey[0] == 0)
		m_szKey = "ee4a9521a4f4a352649754932e8abd8c";
	else m_szKey = lpszKey;

	//NormalKey256 32Bytes
	m_aKey.resize(32,0);
	uint8_t* pKey = m_aKey.data();	
    size_t uLen = m_szKey.size();
    if (uLen > 32) uLen = 32;
	memcpy(pKey,m_szKey.c_str(),m_szKey.size());
}
CAES::~CAES(void){
	m_Bytes.clear();
	m_aKey.clear();
}

bool CAES::Encode( uint8_t* pData, int iSize)
{
	if (pData == NULL || iSize <= 0) return false;		
	int iPadding = (AES_BLOCK_SIZE - (iSize % AES_BLOCK_SIZE));
	int iNeed = iSize + iPadding;
	//CStdString szEnUpdate('\0',iNeed);
	//BYTE* pEncode = (BYTE*)szEnUpdate.GetBuffer();
	uint8_t* pEncode = new uint8_t[iNeed];
	memset(pEncode,0,iNeed);
	int iTotal = 0,outlen = 0;
	uint8_t* pKey = m_aKey.data();
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, pKey, NULL);
	EVP_EncryptUpdate(ctx, pEncode, &outlen, pData, iSize);
	iTotal += outlen;
	EVP_EncryptFinal_ex(ctx, &pEncode[iTotal], &outlen);
	if (iPadding == AES_BLOCK_SIZE) iTotal = iSize;
	else iTotal += outlen;
	EVP_CIPHER_CTX_free(ctx);
	m_Bytes.clear();
	for (int i = 0; i < iTotal; ++i) m_Bytes.push_back(pEncode[i]);
	delete[] pEncode;
	return true;
}
bool CAES::Encode( const char* lpszText)
{
    if (!lpszText) return false;
	int iSize = (int)strlen(lpszText) + 1;
	uint8_t* pData = (uint8_t*)lpszText;
	return Encode(pData,iSize);
}

bool CAES::Decode( uint8_t* pData, int iSize)
{
	if (pData == NULL || iSize <= 0) return false;	
	uint8_t* pDecode = new uint8_t[iSize];//(BYTE*)szDeUpdate.GetBuffer();
	memset(pDecode,0,iSize);
	int iTotal = 0,outlen = 0;	
	uint8_t* pKey = m_aKey.data();
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, pKey, NULL);	
	EVP_DecryptUpdate(ctx, pDecode, &outlen, pData, iSize);
	iTotal += outlen;
	bool bOk = EVP_DecryptFinal_ex(ctx, &pDecode[iTotal], &outlen);
	if (bOk) iTotal += outlen;
	else iTotal = (int)strlen((char*)pDecode);
	EVP_CIPHER_CTX_free(ctx);
	m_Bytes.clear();
	for (int i = 0; i < iTotal; ++i) m_Bytes.push_back(pDecode[i]);
	delete[] pDecode;
	return true;
}

uint8_t* CAES::DecodeBase64Url( const char* lpzCipherBase64Url)
{
	CBase64Url base64url;
	if (!base64url.Decode((uint8_t*)lpzCipherBase64Url,strlen(lpzCipherBase64Url))) return NULL;
	std::vector<uint8_t>* pBytes = base64url.GetBytes();
	if (!Decode(pBytes->data(),(int)pBytes->size())) return NULL;
	return m_Bytes.data();
    /*std::string str;
	for (int i = 0; i < (int)m_Bytes.size(); ++i){
		str += m_Bytes[i];
	}
	pBytes->clear();
	return std::string(str);*/
}
uint8_t* CAES::DecodeHex(const char* lpzCipherHex){
	int iSize = (int)strlen(lpzCipherHex);
	if (iSize % 2 != 0) return NULL;
	std::vector<uint8_t> bytes;
	for (int i = 0; i < iSize; i += 2)
	{
		char ch[3];
		char *p;
		ch[0] = lpzCipherHex[i];
		ch[1] = lpzCipherHex[i+1];
		ch[2] = 0;
		long b = strtol(ch,&p,16);
		bytes.push_back((uint8_t)b);
	}
	if (!Decode(bytes.data(),(int)bytes.size())) return NULL;
    return m_Bytes.data();
}
std::string CAES::ToBase64Url()
{
	if ((int)m_Bytes.size() == 0) return std::string("");
	CBase64Url base64Url;
	if (!base64Url.Encode(m_Bytes.data(),(int)m_Bytes.size())) return std::string("");
	std::vector<uint8_t>* pBytes = base64Url.GetBytes();
	/*uint8_t* pData = pBytes->data();
	std::string str;
	for (int i = 0; i < (int)pBytes->size(); ++i) str += pData[i];	
	pBytes->clear();*/
	return std::string((char*)pBytes->data());
}

std::string CAES::ToHexString(){
	if ((int)m_Bytes.size() <= 0) return std::string("");
	std::string strHex;
    char czt[8];
	for (int i = 0; i < (int)(int)m_Bytes.size(); ++i)
	{
        sprintf(czt,"%02x",m_Bytes[i]);		
		strHex += czt;
	}
	return std::string(strHex);
}
std::string CAES::ToString(){
	return std::string((char*)m_Bytes.data(),m_Bytes.size());
}
size_t CAES::GetSize(){
	return m_Bytes.size();
}