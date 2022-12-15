#pragma once

class CXOR
{
public:
	CXOR(const char* lpszKey = NULL);
	~CXOR(void);
	void SetKey(std::string& key);
	const char*	GetKey();
	bool Encode(uint8_t* msgBytes, uint32_t imsgSize,int kStart = 0);
	bool Encode(const char* lpszText,int kStart = 0);
	bool Decode(uint8_t* msgBytes, uint32_t imsgSize,int kStart = 0);
	std::string DecodeHex(const char* lpzCipherHex,int kStart = 0);	
	std::string DecodeBase64Url(const char* lpzCipherBase64Url,int kStart = 0);
	std::string ToHexString();
	std::string ToBase64Url();
	std::string ToString();
	std::vector<uint8_t>*	GetBytes();
	
protected:
	void XorBytes(uint8_t* msgBytes, uint32_t imsgSize,int kStart = 0);		
private:
	std::string m_szKey;
	std::vector<uint8_t> m_Bytes;
};

class CXOR_HWT
{
public:
	CXOR_HWT(const char* lpszKey = NULL);
	~CXOR_HWT(void);
	void SetKey(std::string& key);
	const char*	GetKey();
	bool Encode(uint8_t* msgBytes, uint32_t imsgSize);
	bool Encode(const char* lpszText);
	bool Decode(uint8_t* msgBytes, uint32_t imsgSize);	
	std::string DecodeBase64Url(const char* lpzCipherBase64Url);
	std::string ToBase64Url();
	std::string ToString();
	std::vector<uint8_t>*	GetBytes();
	
protected:
	void Xor_Hwt(uint8_t* msgBytes, uint32_t imsgSize);			
private:
	std::string m_szKey;
	std::vector<uint8_t> m_Bytes;
};

class CXOR4
{
public:
	CXOR4(const char* lpszKey = NULL);
	~CXOR4(void);
	void SetKey(std::string& key);
	bool Encode(uint8_t* msgBytes, uint32_t imsgSize);
	bool Encode(const char* lpszText);
	bool Decode(uint8_t* msgBytes, uint32_t imsgSize);	
	std::string DecodeBase64Url(const char* lpzCipherBase64Url,bool bHF = true);
	std::string ToBase64Url(bool bHF = true);
	std::string ToString();
	uint8_t*	GetBytes();
	size_t		GetSize();
	std::string	GetKey();
protected:
	void Xor4(uint8_t* msgBytes, uint32_t imsgSize);		
private:
	int 		m_iDevice;
	int			m_iModule;
	std::vector<uint8_t> m_Keys;
	std::vector<uint32_t> m_Bytes;
};

class CBase64Url
{
public:
	CBase64Url(void);
	~CBase64Url(void);
	bool Encode(uint8_t* lpData, int iSize,bool bHF = false);
	std::string Encode(const char* lpText,bool bHF = false);
	bool Decode(uint8_t* lpData, int iSize,bool bHF = false);
	std::string Decode(const char* lpszBase64A,bool bHF = false);
	std::vector<uint8_t>* GetBytes();
	std::string ToString();
	static std::string	GenerateKey(int nSize = 32);

protected:
	bool IsBase64Url(uint8_t c);
private:
	static bool m_bInit;
	static std::string BASE64URL_CHARS;
	static std::string BASE64URL_CHARS_HF; 
	std::vector<uint8_t> m_Bytes;
};

class CChecksum
{
public:
	CChecksum(void);
	~CChecksum(void);
	void		Clear();
	uint32_t	GetChecksum();

	void 		Add(uint8_t bValue);
	void 		Add(uint16_t wValue);
	void 		Add(uint32_t dwValue);
	void 		Add(uint8_t* lpByte, uint32_t nSize);
	void 		Add(const char* lpStrA);

	static uint32_t FromFile(const char* lpFileName);
	static uint32_t Get(uint8_t *buff, int len);
	static uint32_t Get(const char* str);
protected:
	bool		m_bShiftRight;
	uint16_t	m_wConst1;
	uint16_t	m_wConst2;
	uint16_t	m_wRule;
	uint32_t	m_dwChecksum;
};

class CHFT{
public:
	CHFT();
	~CHFT();	
	std::string Encode(uint8_t* lpBytes,size_t size);
	std::string Encode(const char* czClear);
	
	uint8_t* 	Decode(const char* czEncode);
	std::string ToString();
	size_t 		Size();
protected:	
	void 		GenarateKey();
	char 		GetDelimiter(const char* czEncode);
	CXOR4* 		m_pXOR;
};