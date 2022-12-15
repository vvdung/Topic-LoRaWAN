#include "../inc/HFactor.h"
#include "../inc/ECrypto.h"

//////CXOR////////////////////////////////////
CXOR::CXOR(const char* lpszKey)
{
	if (!lpszKey || lpszKey[0] == 0)		
		m_szKey = "ee4a9521a4f4a352649754932e8abd8c";//"@KeY!SeCrEcT^256BITS#H-FACTOR+!$";
	else m_szKey = lpszKey;
}

CXOR::~CXOR(void){
	m_Bytes.clear();
}
void CXOR::SetKey(std::string& key){
	m_szKey = key;
}
const char* CXOR::GetKey(){
	return m_szKey.c_str();
}
void CXOR::XorBytes( uint8_t* msgBytes, uint32_t imsgSize,int kStart /*= 0*/ )
{
	m_Bytes.clear();
	uint8_t* keyBytes = (uint8_t*)m_szKey.c_str();
	int ikeySize = m_szKey.size();
	int k = kStart;
	for (uint32_t i = 0; i < imsgSize; ++i){
		if (k > ikeySize) k = 0;
		uint8_t x = msgBytes[i] ^ keyBytes[k];
		m_Bytes.push_back(x);
		++k;
	}
}

bool CXOR::Encode( const char* lpszText,int kStart /*= 0*/ )
{
	if (lpszText == NULL) return false;
	uint32_t iTxtSize = (uint32_t)strlen(lpszText);	
	XorBytes((uint8_t*)lpszText,iTxtSize,kStart);
	return true;
}

bool CXOR::Encode(uint8_t* msgBytes, uint32_t imsgSize, int kStart /*= 0*/ )
{
	if (msgBytes == NULL || imsgSize <= 0) return false;
	XorBytes(msgBytes,imsgSize,kStart);
	return true;
}
bool CXOR::Decode(uint8_t* msgBytes, uint32_t imsgSize,int kStart){
	return Encode(msgBytes,imsgSize,kStart);
}
std::string CXOR::DecodeHex( const char* lpzCipherHex,int kStart /*= 0*/ )
{
	if (lpzCipherHex == NULL || lpzCipherHex[0] == 0) return std::string("");

	int iSize = strlen(lpzCipherHex);
	if (iSize % 2 != 0) return std::string("");
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
	XorBytes(bytes.data(),(int)bytes.size(),kStart);
	bytes.clear();
	return ToString();
}
std::string CXOR::DecodeBase64Url( const char* lpzCipherBase64Url,int kStart /*= 0*/ )
{
	CBase64Url base64url;
	if (base64url.Decode(lpzCipherBase64Url).empty()) return std::string("");
	std::vector<uint8_t>* pBytes = base64url.GetBytes();
	XorBytes(pBytes->data(),(int)pBytes->size(),kStart);
	pBytes->clear();
	return ToString();
}

std::string CXOR::ToHexString()
{
	if (m_Bytes.size() == 0) return std::string("");
	std::string strHex;
	char czTmp[8];
	for (int i = 0; i < (int)m_Bytes.size(); ++i)
	{
		sprintf(czTmp,"%02x",m_Bytes[i]);
		strHex += czTmp;
	}
	return std::string(strHex);
}

std::string CXOR::ToBase64Url()
{
	if (m_Bytes.size() == 0) return std::string("");
	CBase64Url base64Url;
	if (!base64Url.Encode(m_Bytes.data(),(int)m_Bytes.size())) return std::string("");
	std::vector<uint8_t>* pBytes = base64Url.GetBytes();
	uint8_t* pData = pBytes->data();
	std::string str;
	for (int i = 0; i < (int)pBytes->size(); ++i) str += pData[i];
	pBytes->clear();
	return std::string(str);
}
std::string CXOR::ToString()
{
	std::string str;
	for (int i = 0; i < (int)m_Bytes.size(); ++i) str += m_Bytes[i];	
	return std::string(str);
}

std::vector<uint8_t>* CXOR::GetBytes()
{
	return &m_Bytes;
}
//////CXOR_HWT////////////////////////////////////
CXOR_HWT::CXOR_HWT(const char* lpszKey){
	if (!lpszKey || lpszKey[0] == 0)		
		m_szKey = CBase64Url::GenerateKey(4);
	else m_szKey = lpszKey;	
}

CXOR_HWT::~CXOR_HWT(){
	m_Bytes.clear();
}
void CXOR_HWT::Xor_Hwt(uint8_t* msgBytes, uint32_t imsgSize){
	m_Bytes.clear();
	uint32_t uKey = *((uint32_t*)m_szKey.c_str());
	uint8_t* pKey = (uint8_t*)&uKey;
	int iCount = imsgSize / 4;
	int iMod = imsgSize % 4;
	uint32_t* pMsg = (uint32_t*)msgBytes;
	uint32_t value;
	uint8_t* p = (uint8_t*)&value;
	for (int i = 0; i < iCount; ++i){
		value = pMsg[i] ^ uKey;
		m_Bytes.push_back(p[0]);
		m_Bytes.push_back(p[1]);
		m_Bytes.push_back(p[2]);
		m_Bytes.push_back(p[3]);		
	}
	switch (iMod){
		case 1:
			m_Bytes.push_back(msgBytes[imsgSize - 3] ^ pKey[0]);
		case 2:
			m_Bytes.push_back(msgBytes[imsgSize - 2] ^ pKey[1]);
		case 3:
			m_Bytes.push_back(msgBytes[imsgSize - 1] ^ pKey[2]);
	}
}
void CXOR_HWT::SetKey(std::string& key){
	m_szKey = key;
}
const char* CXOR_HWT::GetKey(){
	return m_szKey.c_str();
}
bool CXOR_HWT::Encode(uint8_t* msgBytes, uint32_t imsgSize){
	if (msgBytes == NULL || imsgSize <= 0) return false;
	Xor_Hwt(msgBytes,imsgSize);
	return true;
}
bool CXOR_HWT::Encode(const char* lpszText){
	if (lpszText == NULL) return false;
	uint32_t iTxtSize = (uint32_t)strlen(lpszText);	
	Xor_Hwt((uint8_t*)lpszText,iTxtSize);
	return true;
}
bool CXOR_HWT::Decode(uint8_t* msgBytes, uint32_t imsgSize){
	return Encode(msgBytes,imsgSize);
}
std::string CXOR_HWT::DecodeBase64Url( const char* lpzCipherBase64Url)
{
	CBase64Url base64url;
	if (base64url.Decode(lpzCipherBase64Url).empty()) return std::string("");
	std::vector<uint8_t>* pBytes = base64url.GetBytes();
	Xor_Hwt(pBytes->data(),(int)pBytes->size());
	pBytes->clear();
	return ToString();
}

std::string CXOR_HWT::ToString()
{
	std::string str;
	for (int i = 0; i < (int)m_Bytes.size(); ++i) str += m_Bytes[i];	
	return std::string(str);
}
std::string CXOR_HWT::ToBase64Url()
{
	if (m_Bytes.size() == 0) return std::string("");
	CBase64Url base64Url;
	if (!base64Url.Encode(m_Bytes.data(),(int)m_Bytes.size())) return std::string("");
	std::vector<uint8_t>* pBytes = base64Url.GetBytes();
	uint8_t* pData = pBytes->data();
	std::string str;
	for (int i = 0; i < (int)pBytes->size(); ++i) str += pData[i];
	pBytes->clear();
	return std::string(str);
}
std::vector<uint8_t>* CXOR_HWT::GetBytes(){
	return &m_Bytes;
}

//////CXOR4////////////////////////////////////
CXOR4::CXOR4(const char* lpszKey){
	if (!lpszKey || lpszKey[0] == 0){
		char* p = (char*)CBase64Url::GenerateKey(4).c_str();
		for (int i = 0 ; i < 4; ++i) m_Keys.push_back(p[i]);
	}				
	else{
		char* p = (char*)lpszKey;
		int iLen = strlen(lpszKey);			
		for (int i = 0 ; i < 4; ++i){
			if (i < iLen) m_Keys.push_back(p[i]);
			else m_Keys.push_back(0);
		} 		
	}
	m_iDevice = 0;
	m_iModule = 0;
}

CXOR4::~CXOR4(){
	m_Bytes.clear();
}
void CXOR4::Xor4(uint8_t* msgBytes, uint32_t imsgSize){
	m_Bytes.clear();	

	uint32_t uKey = *((uint32_t*)m_Keys.data()); //m_szKey.c_str());
	
	m_iDevice = imsgSize / 4;
	m_iModule = imsgSize % 4;
	uint32_t* pMsg = (uint32_t*)msgBytes;
	uint32_t value;
	
	for (int i = 0; i < m_iDevice; ++i){
		value = pMsg[i] ^ uKey;
		m_Bytes.push_back(value);		
	}
	if (m_iModule > 0){
		memcpy(&value,&msgBytes[imsgSize - m_iModule],m_iModule);
		m_Bytes.push_back(value ^ uKey);
	}
}
uint8_t* CXOR4::GetBytes(){
	return (uint8_t*)m_Bytes.data();
}
size_t CXOR4::GetSize(){
	return (size_t)(m_iDevice * 4 + m_iModule);
}
void CXOR4::SetKey(std::string& key){
	m_Keys.clear();
	char* p = (char*)key.c_str();
	int iLen = strlen(p);			
	for (int i = 0 ; i < 4; ++i){
		if (i < iLen) m_Keys.push_back(p[i]);
		else m_Keys.push_back(0);
	}	
}
std::string CXOR4::GetKey(){
	std::string key((char*)m_Keys.data(),4);
	return std::string(key);
}
bool CXOR4::Encode(uint8_t* msgBytes, uint32_t imsgSize){
	if (msgBytes == NULL || imsgSize <= 0) return false;
	Xor4(msgBytes,imsgSize);
	return true;
}
bool CXOR4::Encode(const char* lpszText){
	if (lpszText == NULL) return false;
	uint32_t iTxtSize = (uint32_t)strlen(lpszText);	
	Xor4((uint8_t*)lpszText,iTxtSize);
	return true;
}
bool CXOR4::Decode(uint8_t* msgBytes, uint32_t imsgSize){
	return Encode(msgBytes,imsgSize);
}	
std::string CXOR4::DecodeBase64Url(const char* lpzCipherBase64Url,bool bHF){
	CBase64Url base64url;
	bool bOk = base64url.Decode((uint8_t*)lpzCipherBase64Url,strlen(lpzCipherBase64Url),bHF);
	if (!bOk) return std::string("");
	std::vector<uint8_t>* pBytes = base64url.GetBytes();
	Xor4(pBytes->data(),(int)pBytes->size());
	pBytes->clear();
	return ToString();
}
std::string CXOR4::ToBase64Url(bool bHF){
	if (m_Bytes.size() == 0) return std::string("");
	CBase64Url base64Url;
	if (!base64Url.Encode(GetBytes(),(int)GetSize(),bHF)) return std::string("");
	return base64Url.ToString();// std::string((char*)base64Url.GetBytes()->data());
}
std::string CXOR4::ToString(){	
	return std::string((char*)GetBytes(),GetSize());
}
//////CBase64Url////////////////////////////////////
bool CBase64Url::m_bInit = false;
std::string CBase64Url::BASE64URL_CHARS 	 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
std::string CBase64Url::BASE64URL_CHARS_HF   = "abcdefghijklmnopqrstuvwxyz0123456789-_ABCDEFGHIJKLMNOPQRSTUVWXYZ";

CBase64Url::CBase64Url(void){}
CBase64Url::~CBase64Url(void){
	m_Bytes.clear();
}

std::string CBase64Url::GenerateKey( int nSize /*= 32*/ )
{

	if (!m_bInit){		
		srand((uint32_t)time(NULL));
		m_bInit = true;
	}
	int iMax = BASE64URL_CHARS.size() - 1;
	std::string szText = "";
	for (int i = 0; i < nSize; ++i)
	{
		int k = (int)(rand() % iMax );		
		szText += BASE64URL_CHARS[k];
	}
	return std::string(szText);
}
bool CBase64Url::IsBase64Url(uint8_t c)
{
	return (isalnum(c) || (c == '-') || (c == '_'));
}

bool CBase64Url::Encode( uint8_t* lpData, int iSize,bool bHF )
{
	m_Bytes.clear();
	if (lpData == NULL || iSize <= 0) return false;
	int in_len = iSize;
	uint8_t* pData = (uint8_t*)lpData;
	uint8_t*  pBASE64= (bHF == true ? (uint8_t*)BASE64URL_CHARS_HF.c_str() : (uint8_t*)BASE64URL_CHARS.c_str());
	int i = 0;
	int j = 0;
	uint8_t char_array_3[3];
	uint8_t char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(pData++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++) m_Bytes.push_back(pBASE64[char_array_4[i]]);
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++) m_Bytes.push_back(pBASE64[char_array_4[j]]);
		//while((i++ < 3)) szEncode += '=';

	}

	return true;
}

std::string CBase64Url::Encode( const char* lpText,bool bHF )
{
	int iSize = (int)strlen(lpText);
	if (!Encode((uint8_t*)lpText,iSize,bHF)) return std::string("");
	return ToString();//std::string((char*)m_Bytes.data(),m_Bytes.size());
}

bool CBase64Url::Decode( uint8_t* lpData, int iSize,bool bHF )
{
	if (lpData == NULL || iSize <= 0) return false;
	int in_len = iSize;
	uint8_t* encoded_string = (uint8_t*)lpData;
	std::string*  pBASE64= (bHF == true ? &BASE64URL_CHARS_HF : &BASE64URL_CHARS);
	m_Bytes.clear();

	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];

	while (in_len-- && ( encoded_string[in_] != '=') && IsBase64Url(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = pBASE64->find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++) m_Bytes.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = pBASE64->find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) m_Bytes.push_back(char_array_3[j]);
	}
	return true;
}

std::string CBase64Url::Decode( const char* lpszBase64A,bool bHF )
{
	if (!Decode((uint8_t*)lpszBase64A,(int)strlen(lpszBase64A),bHF)) return std::string("");	
	return ToString();//std::string((char*)m_Bytes.data(),m_Bytes.size());
}
std::string CBase64Url::ToString(){
	return std::string((char*)m_Bytes.data(),m_Bytes.size());
}
std::vector<uint8_t>* CBase64Url::GetBytes()
{
	return &m_Bytes;
}

//////CChecksum////////////////////////////////////
CChecksum::CChecksum(void)
{
	Clear();
}

CChecksum::~CChecksum(void){}
void CChecksum::Clear()
{
	m_dwChecksum = 0; m_wRule = 55665; m_wConst1 = 52845; m_wConst2 = 22719; m_bShiftRight = false;
}
uint32_t CChecksum::GetChecksum(){
	return m_dwChecksum;
}
void CChecksum::Add(uint8_t bValue)
{
	m_bShiftRight = !m_bShiftRight;
	bool cipher;
	if (m_bShiftRight)  cipher = (bValue ^ (m_wRule >> 8));
	else				cipher = (bValue ^ (m_wRule >> 4));
	m_wRule = (cipher + m_wRule) * m_wConst1 + m_wConst2;
	m_dwChecksum += cipher;
}
void CChecksum::Add(uint16_t wValue)
{
	uint8_t *lpBytes = (uint8_t*)&wValue;
	for(int i = 0; i < (int)sizeof(uint16_t); i++) Add(lpBytes[i]);
}
void CChecksum::Add(uint32_t dwValue)
{
	uint8_t *lpBytes = (uint8_t*)&dwValue;
	for(int i = 0; i < (int)sizeof(uint32_t); i++) Add(lpBytes[i]);
}
void CChecksum::Add(uint8_t* lpByte, uint32_t nSize)
{
	for(uint32_t i = 0; i < nSize; i++) Add(lpByte[i]);
}
void CChecksum::Add(const char* lpStrA)
{
	uint32_t iSize = (uint32_t)strlen(lpStrA);
	Add((uint8_t*)lpStrA,iSize);
}

uint32_t CChecksum::FromFile( const char* lpFileName )
{
	FILE* f = fopen(lpFileName,"r");
	if (!f) return 0;
	CChecksum sum;
	char buf[16384];
	while (!feof(f)){
		int len = (int)fread(buf,1,16384,f);
		if (len > 0) sum.Add((uint8_t*)buf, (uint32_t)len);
	}
	fclose(f);
	return sum.GetChecksum();
}
uint32_t CChecksum::Get(uint8_t *buff, int len){
	uint32_t sum = 0;
	int swappem = 0;

	//if (1 & (unsigned long)buff) {
		sum = *buff << 8;
		buff++;
		len--;
		++swappem;
	//}

	while (len > 1) {
		sum += *(unsigned short *)buff;
		buff += 2;
		len -= 2;
	}

	if (len > 0)
		sum += *buff;

	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	if (swappem)
		sum = ((sum & 0xff00) >> 8) + ((sum & 0x00ff) << 8);

	return sum;
}
uint32_t CChecksum::Get(const char* str){
	if (!str) return 0;
    return CChecksum::Get((uint8_t*)str,strlen(str));
}
//////CHFT////////////////////////////////////
CHFT::CHFT(){
	m_pXOR = new CXOR4();
}
CHFT::~CHFT(){
	delete m_pXOR;
}
void CHFT::GenarateKey(){
	static const char HEX_CHARS[] = "abcdef0123456789ABCDEF";
	std::string szText = "";
	for (int i = 0; i < 4; ++i)
	{
		int k = (int)(rand() % 21 );		
		szText += HEX_CHARS[k];
	}
	m_pXOR->SetKey(szText);
}

char CHFT::GetDelimiter(const char* czEncode){
	static const char DELIMITER_CHARS[] = "=~!@#$^*()+{}[]|/<>.,";
	if (!czEncode) return DELIMITER_CHARS[0];
	int value = *(int*)czEncode;	
	int i = (value % 21);	
	return DELIMITER_CHARS[i];
}
std::string CHFT::Encode(uint8_t* lpBytes,size_t size){	  
	if (!lpBytes) return std::string("");
	GenarateKey();
	char* pKey = (char*)m_pXOR->GetKey().c_str();	
    m_pXOR->Encode(lpBytes,size);    
    std::string enCode = m_pXOR->ToBase64Url();	
	std::string szKey = StringFormat("%s%s",pKey,enCode.c_str());
	uint32_t uSum = CChecksum::Get(szKey.c_str());
	char delimiter = GetDelimiter(enCode.c_str());
	
	std::string szSum = StringFormat("%x",uSum);
    char* pSum = (char*)szSum.c_str();
    char ch = pKey[0];
    pKey[0] = pSum[0];
    pSum[0] = ch;

	return StringFormat("%s%c%s%s",enCode.c_str(),delimiter,pKey,pSum);
	/*if (uSum % 2 == 0)
		return StringFormat("%s%c%s%x",enCode.c_str(),delimiter,m_pXOR->GetKey(),uSum);	
	return StringFormat("%s%c%s%X",enCode.c_str(),delimiter,m_pXOR->GetKey(),uSum);*/
}
std::string CHFT::Encode(const char* czClear){
	if (!czClear) return std::string("");
	return Encode((uint8_t*)czClear,(uint32_t)strlen(czClear));
}

uint8_t* CHFT::Decode(const char* czEncode){
    if (!czEncode) return NULL;
	char delimiter[2];
	delimiter[0] = GetDelimiter(czEncode);
	delimiter[1] = 0;	
    std::vector<std::string> a = SplitString(czEncode,delimiter);
	if (a.size() != 2) return NULL;
	if (a[1].size() < 4) return NULL;
	char* pKey = (char*)a[1].c_str();
	char ch = pKey[0];
	pKey[0] = pKey[4];
	pKey[4] = ch;

    uint32_t uSum = strtol(&pKey[4],NULL,16);
	pKey[4] = 0;

	std::string szKey = StringFormat("%s%s",pKey,a[0].c_str());	
	if (uSum != CChecksum::Get(szKey.c_str())) return NULL;

    CBase64Url bs64;
	bs64.Decode((uint8_t*)a[0].c_str(),(int)a[0].size(),true);
	std::vector<uint8_t>* pBytes = bs64.GetBytes();  

    m_pXOR->SetKey(szKey);
    m_pXOR->Decode(pBytes->data(),(uint32_t)pBytes->size());    
    return m_pXOR->GetBytes();
}
std::string CHFT::ToString(){
	return m_pXOR->ToString();
}
size_t CHFT::Size(){
	return m_pXOR->GetSize();
}
