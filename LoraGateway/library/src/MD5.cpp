#include "../inc/HFactor.h"
#include "../inc/openssl/md5.h"

#include "MD5.h"

std::string CMD5::FromString(uint8_t* buff,size_t uSize){
	if (!buff || uSize <= 0) return std::string("");
	unsigned char digest[16];
	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, buff, uSize);
	MD5_Final(digest, &ctx);
	char mdString[33]; mdString[32] = 0;	
	for (int i = 0; i < 16; i++)
		sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
	return std::string(mdString);
}
std::string CMD5::FromString(const char* lpText){
    if (!lpText) return std::string("");
    return FromString((uint8_t*)lpText,strlen(lpText));
}

std::string CMD5::FromFile(const char* lpFileName){
    FILE* f = fopen(lpFileName,"r");	
	if (!f) return std::string("");

	MD5_CTX ctx;
	MD5_Init(&ctx);
	char buf[16384];
	while (!feof(f)) {
		size_t r = fread(buf,1, 16384,f);
		MD5_Update(&ctx, buf, r);
	}
	fclose(f);
	unsigned char digest[16];
	MD5_Final(digest, &ctx);
	char mdString[33]; mdString[32] = 0;	
	for (int i = 0; i < 16; i++)
		sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
	return std::string(mdString);
}