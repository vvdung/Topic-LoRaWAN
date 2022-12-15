#pragma once

#include "../inc/MongoClientPool.h"

#include "HFactorClient.h"

class CHFactorClientPool : public CMongoClientPool
{
public:
    CHFactorClientPool(const char *uri_string, const char *db_name);
    virtual ~CHFactorClientPool();
    CHFactorClient*   Pop();
protected:
    virtual CMongoClient*   OnNewClient(mongoc_client_t* client);
};