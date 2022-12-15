#pragma once
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <math.h>
#include <stdarg.h>
#include <dlfcn.h>

#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <arpa/inet.h>

#include <vector>
#include <queue>
#include <mutex>

#include "EThread.h"
#include "PoolThread.h"
#include "ETimer.h"
#include "ESocket.h"
#include "EConnect.h"
#include "EQueueConnect.h"
#include "EServer.h"
#include "EClient.h"
#include "MongoClientPool.h"
//#include "HttpClient.h"

#include "ECrypto.h"
#include "Ebson.h"
#include "MD5.h"
#include "AES.h"
#include "HS256.h"
#include "JWT.h"

#define MAKE64(h,l) ((uint64_t) (((uint64_t)h << 32) | (uint64_t)l))
#define HIGH32(v) ((int)((uint64_t)v >> 32))
#define LOW32(v) ((int)v)

typedef struct _utc_time{
    struct tm  tmUTC;
    int        milisec;
}UTC_TIME;

uint64_t    GetTickCount();
uint64_t    GetNanoCount();
int         Sleep(uint32_t miliseconds);
bool        IsReady(uint32_t miliseconds, uint64_t& tStarted);
uint64_t    GetMiliseconds();
char*       TimeStringUTC (UTC_TIME& tmUTC, bool bWithMiliseconds = false);
char*       GetTimeUTC(int h = 0, int m = 0,UTC_TIME* tmUTC = NULL,bool bWithMiliseconds = false);//h=-12 -> 12; m: 0/30
char*       GetTimeString(bool bWithMiliseconds = false);
char*       GetTimeVN(UTC_TIME* tmVN = NULL, bool bWithMiliseconds = false);
char*       GetTimeLocal(UTC_TIME* tmVN = NULL, bool bWithMiliseconds = false);
std::vector<std::string> SplitString(const char* src,const char * delimiters,bool useBuffer = false);//useBuffer = true: dont change src
std::string StringFormat(const char* fmt, ...);
CPoolThread*    GetPoolThread();

double      RandomMinMax(int min, int max);