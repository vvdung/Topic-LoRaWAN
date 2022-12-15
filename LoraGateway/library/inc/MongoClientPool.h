#pragma once
#include "mongoc.h"

#include "MongoClient.h"

class CMongoClientPool {
public:
    CMongoClientPool(const char *uri_string, const char *db_name);
    virtual ~CMongoClientPool();
    void            Push(CMongoClient *client);
    CMongoClient*   Pop();
    //const char*     GetDatabaseName();
protected:
    virtual CMongoClient*   OnNewClient(mongoc_client_t* client);

    mongoc_client_pool_t *  _pool;
    std::string             _dbName;

    std::map<uint64_t,CMongoClient*> _mapClient;
};