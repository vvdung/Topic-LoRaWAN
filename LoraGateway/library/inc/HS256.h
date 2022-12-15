#pragma once

class CHS256
{
public:
	CHS256(const char* lpszKey = NULL);
	~CHS256(void);

	std::string GetHash(const char* lpszText);

	bool Encode(uint8_t* pData, int iSize);
	bool Encode(const char* lpszText);

	std::string ToHexString();
	std::string ToBase64Url();

protected:
	std::string m_szKey;
	std::vector<uint8_t> m_Bytes;
};