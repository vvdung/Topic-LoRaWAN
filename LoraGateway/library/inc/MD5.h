#pragma once

class CMD5
{
public:
	static std::string FromString(uint8_t* buff,size_t uSize);
    static std::string FromString(const char* lpText);
	static std::string FromFile(const char* lpFileName);
protected:
	CMD5();
	~CMD5(void);	
};