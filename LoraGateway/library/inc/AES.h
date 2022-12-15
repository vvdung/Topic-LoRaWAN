#pragma once

class CAES
{
public:
	CAES(const char* lpszKey = NULL);
	~CAES(void);

	bool Encode(uint8_t* pData, int iSize);
	bool Encode(const char* lpszText);

	bool Decode(uint8_t* pData, int iSize);
	uint8_t* DecodeBase64Url(const char* lpzCipherBase64Url);
    uint8_t* DecodeHex(const char* lpzCipherHeX);
	std::string ToBase64Url();
    std::string ToHexString();
	std::string ToString();
	size_t 	GetSize();
protected:
	std::string m_szKey;
	std::vector<uint8_t> m_Bytes;
	std::vector<uint8_t> m_aKey;
};