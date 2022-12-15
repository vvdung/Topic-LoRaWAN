#pragma once
#include "mongoc.h"

class CMongoClient;
class CMongoDatabase;

class CMongoCollection {
public:
    //CMongoCollection(const char* colName, CMongoClient* client);
    CMongoCollection(mongoc_collection_t *collection, CMongoClient* client);

    virtual ~CMongoCollection();
    mongoc_collection_t* Get();
    uint32_t             GetErrorCode();
    uint32_t             GetErrorDomain();   
    const char*          GetErrorMessage();
    void                 ShowError(const char* name);

    const char*          GetName();
    bool    CreateIndexes(CBSONObject* oKey, bool bUnique = false);

    uint64_t Count(CBSONObject* oQuery);
    uint64_t Count(const char* czQuery);
    
    bool InsertOne(CBSONObject* oDoc,CBSONObject* oOption = NULL,CBSONObject* oReply = NULL);
    bool IncreaseField(const char* fieldName, uint64_t& count);
protected:
    mongoc_collection_t *_collection;  
    bson_error_t        _error; 
    CMongoDatabase*     _database;
    
};