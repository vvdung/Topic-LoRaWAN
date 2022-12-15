#pragma once

#include "../inc/MongoClient.h"

#include "HFactorDatabase.h"

class CHFactorClient : public CMongoClient
{
public:
    CHFactorClient(mongoc_client_t* client,const char *db_name);
    virtual ~CHFactorClient();
    CHFactorDatabase*    GetDatabase();

protected:
    virtual CMongoDatabase* OnNewDatabase(const char *db_name);
};