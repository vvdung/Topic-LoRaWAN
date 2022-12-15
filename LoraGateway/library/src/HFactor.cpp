#include "../inc/HFactor.h"  

uint64_t GetTickCount(){
    struct timespec tTemp = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &tTemp);	
	uint64_t lRunMS = tTemp.tv_sec*1000 + tTemp.tv_nsec/1000000 ;
	return lRunMS ;
}
uint64_t GetNanoCount(){
    struct timespec tTemp = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &tTemp);	
	uint64_t tNano = tTemp.tv_sec*1000000000 + tTemp.tv_nsec;
	return tNano ;
}
int Sleep(uint32_t miliseconds){
    struct timespec timeout;
    timeout.tv_nsec =  (miliseconds % 1000) * 1000000;  //nanoseconds
    timeout.tv_sec  =  (miliseconds / 1000); //miliseconds
    return nanosleep(&timeout,NULL);
}
bool IsReady(uint32_t miliseconds, uint64_t& tStarted){
    uint64_t t = GetTickCount();
    if (t - tStarted < miliseconds) return false;
    tStarted = t;
    return true;
}
/*char* TimeString (const struct tm *tp, bool bWithMiliseconds){
    static char buffer [32];
    strftime(buffer, 24, "%Y-%m-%d %H:%M:%S", tp);
    return buffer;
}*/
/*std::string GetTimeString(bool bWithMiliseconds){
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    //std::string s1(TimeString(localtime(&curTime.tv_sec)));
    char buffer [32];
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    if (!bWithMiliseconds) return (buffer);//std::string(s1);//
    char currentTime[64];
    sprintf(currentTime, "%s.%03d", buffer, milli);
    return std::string(currentTime); 
}*/
uint64_t GetMiliseconds(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (uint64_t)(tp.tv_sec * 1000L + tp.tv_usec / 1000);
}
char* TimeStringUTC (UTC_TIME& tmUTC, bool bWithMiliseconds){
    static char buffer [32];
    buffer[0] = 0;
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", &tmUTC.tmUTC);
    if (!bWithMiliseconds) return buffer;    
    int len = (int)strlen(buffer);
    sprintf(&buffer[len],".%03d",tmUTC.milisec);
    return buffer;
}
char* GetTimeUTC(int h, int m,UTC_TIME* tmUTC,bool bWithMiliseconds){
    uint64_t msSec = GetMiliseconds();
    int h_mili = h * 3600000;
    int m_mili = m * 60000;
    msSec = msSec + h_mili + m_mili;
    
    time_t   tLocal = msSec / 1000;
    int milli       = msSec % 1000;
    struct tm tmLocal;
    struct tm tmUTC_0;
    localtime_r(&tLocal,&tmLocal);
    gmtime_r(&tLocal,&tmUTC_0);      
    UTC_TIME utcTemp;
    utcTemp.milisec = milli;
    memcpy(&utcTemp.tmUTC,&tmUTC_0,sizeof(tmUTC_0));
    if (tmUTC) memcpy(tmUTC,&utcTemp,sizeof(UTC_TIME));        
    return TimeStringUTC(utcTemp,bWithMiliseconds);
}
char* GetTimeVN(UTC_TIME* tmVN, bool bWithMiliseconds){
    UTC_TIME utc;
    char* p = GetTimeUTC(7,0,&utc,bWithMiliseconds);
    if (tmVN) memcpy(tmVN,&utc,sizeof(UTC_TIME));    
    return p;
}
char*  GetTimeLocal(UTC_TIME* tmPC, bool bWithMiliseconds){
    uint64_t msSec = GetMiliseconds();
    time_t   tLocal = msSec / 1000;
    int milli       = msSec % 1000;
    struct tm tmLocal;
    localtime_r(&tLocal,&tmLocal);
    UTC_TIME utcTemp;
    utcTemp.milisec = milli;
    memcpy(&utcTemp.tmUTC,&tmLocal,sizeof(tmLocal));
    if (tmPC) memcpy(tmPC,&utcTemp,sizeof(UTC_TIME));        
    return TimeStringUTC(utcTemp,bWithMiliseconds);;
}

char* GetTimeString(bool bWithMiliseconds){
    static char buffer [32];
    struct timeval curTime;
    gettimeofday(&curTime, NULL);    
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    if (!bWithMiliseconds) return buffer;
    int milli = curTime.tv_usec / 1000;    
    int len = (int)strlen(buffer);
    sprintf(&buffer[len],".%03d",milli);
    return buffer;
}
std::vector<std::string> SplitString(const char* src,const char * delimiters,bool useBuffer){
    std::vector<std::string> a;
    char* str  = (char*)src;
    if (useBuffer){
        size_t size = strlen(src) + 1;
        str = (char*)malloc(size);
        strcpy(str,src);
    }
    char * pch;
    pch = strtok (str,delimiters);
    while (pch != NULL)
    {
        a.push_back(std::string(pch));
        pch = strtok (NULL, delimiters);
    }
    if (useBuffer) free(str);
    return a;
}
std::string StringFormat(const char* fmt, ...){
    va_list vaArgs;
    va_start(vaArgs, fmt);
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = std::vsnprintf(NULL, 0, fmt, vaArgsCopy);
    va_end(vaArgsCopy);
    std::vector<char> zc(iLen + 1);
    std::vsnprintf(zc.data(), zc.size(), fmt, vaArgs);
    va_end(vaArgs);
    return std::string(zc.data(), iLen); 
}

CPoolThread* GetPoolThread(){
    CPoolThread* _poolThread = NULL;
    if (_poolThread) return _poolThread;
    _poolThread = new CPoolThread();
    return _poolThread;
}

double      RandomMinMax(int min, int max){
    return (double)(min + rand() % (max + 1 - min));
}