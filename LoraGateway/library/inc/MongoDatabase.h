#pragma once
#include "mongoc.h"

class CMongoClient;
class CMongoCollection;
class CBSONObject;

class CMongoDatabase {
public:
    CMongoDatabase(const char* dbName, CMongoClient* client);
    virtual ~CMongoDatabase();
    mongoc_database_t *     Get();    
    const char*             GetName();
    CMongoCollection*       GetCollection(const char* colName);    
    bool    WriteCommand(CBSONObject* oCommand, CBSONObject* oOption = NULL);
protected:
 
    virtual CMongoCollection*   OnNewCollection(mongoc_collection_t *col,const char* colName);

    CMongoClient*   _client;
    mongoc_database_t *_database;
    std::map<const char*,CMongoCollection*> _mapCollection;
};