#pragma once
#include "MongoDatabase.h"
#include "MongoCollection.h"

class CMongoClient {
public:
    CMongoClient(const char *uri_string, const char *db_name);
    CMongoClient(mongoc_client_t* client,const char *db_name);
    virtual ~CMongoClient();
    void                InitDatabase();
    mongoc_client_t*    Get();

    CMongoDatabase*     GetDatabase();
    CMongoCollection*   GetCollection(const char* colName);
    
protected:
    virtual CMongoDatabase* OnNewDatabase(const char *db_name);
    
    mongoc_client_t *_client;
    CMongoDatabase* _database;   
    std::string     _dbName; 
};