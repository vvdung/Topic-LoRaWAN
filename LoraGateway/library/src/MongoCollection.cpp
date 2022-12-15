#include "../inc/HFactor.h"
#include "../inc/MongoClient.h"
#include "../inc/MongoCollection.h"
/*
CMongoCollection::CMongoCollection(const char* colName, CMongoClient* client){
    _collection = NULL; 
    
    memset(&_error,0,sizeof(_error));
    if (!client) return;
    _collection = client->GetCollection(colName)->Get();
    _database = client->GetDatabase();
}*/

CMongoCollection::CMongoCollection(mongoc_collection_t *collection, CMongoClient* client){
    memset(&_error,0,sizeof(_error));
    _collection = NULL;
    if (!collection || !client) return;
    _collection = collection;
    _database = client->GetDatabase();
}

CMongoCollection::~CMongoCollection(){
    if (_collection){
        mongoc_collection_destroy (_collection);        
    }    
}

mongoc_collection_t * CMongoCollection::Get(){
    return _collection;
}
const char* CMongoCollection::GetName(){
    return mongoc_collection_get_name(_collection);
}
uint32_t CMongoCollection::GetErrorCode() {return _error.code;}
uint32_t CMongoCollection::GetErrorDomain() {return _error.domain;}
const char* CMongoCollection::GetErrorMessage() {return _error.message;}
void CMongoCollection::ShowError(const char* name){
    printf("[?] %s Error: %d (%s)\n",name,_error.code,_error.message);
}
bool CMongoCollection::CreateIndexes(CBSONObject* oKey, bool bUnique){
    if (!oKey) return false;

    char* index_name = mongoc_collection_keys_to_index_string (oKey->Get());
	bson_t* bIndexes = BCON_NEW ("createIndexes",
                              BCON_UTF8 (GetName()),
                              "indexes",
                              "[",
                              "{",
                              "key",
                              BCON_DOCUMENT (oKey->Get()),
                              "name",
                              BCON_UTF8 (index_name),
                              "unique",
                              BCON_BOOL(bUnique),
                              "}",
                              "]");	
    bson_free (index_name);

    CBSONObject oIndexes(bIndexes);
    //printf("[+] %s\n",oIndexes.ToJson().c_str());

    return _database->WriteCommand(&oIndexes);    
}

uint64_t CMongoCollection::Count(CBSONObject* oQuery){
    return mongoc_collection_count_documents (_collection, oQuery->Get(), NULL, NULL, NULL, &_error);    
}
uint64_t CMongoCollection::Count(const char* czQuery){
    bson_error_t error;
	bson_t* b = CBSONObject::Parse(czQuery, &error);
	if (!b){
        printf("%d [%s]\n",error.code,error.message);
        return 0;
    }
    CBSONObject oQuery(b);
    return Count(&oQuery);
}

bool CMongoCollection::InsertOne(CBSONObject* oDoc,CBSONObject* oOption, CBSONObject* oReply){
    bson_t *opts = NULL;
    bson_t *reply = NULL;
    if (oOption) opts = oOption->Get();
    if (oReply) reply = oReply->Get();
    bool r = mongoc_collection_insert_one(
        _collection,
        oDoc->Get(),
        opts,
        reply,
        &_error);

    if (!r) ShowError("InsertOne()");

    return r;
}

bool CMongoCollection::IncreaseField(const char* fieldName, uint64_t& count){    
    bson_t *update = BCON_NEW ("$inc", "{", fieldName, BCON_INT64 (1), "}");
    bson_t query = BSON_INITIALIZER;

    CBSONObject oReply;

    mongoc_find_and_modify_opts_t *opts = mongoc_find_and_modify_opts_new();
	mongoc_find_and_modify_opts_set_update (opts, update);
	/* Create the document if it didn't exist, and return the updated document */
   	mongoc_find_and_modify_opts_set_flags (
      opts, (mongoc_find_and_modify_flags_t)
      (MONGOC_FIND_AND_MODIFY_UPSERT | MONGOC_FIND_AND_MODIFY_RETURN_NEW));

    bool bOk = mongoc_collection_find_and_modify_with_opts (
      _collection, &query, opts, oReply.Get(), &_error);

    if (bOk){
        if (oReply.HasKey("value")){
            bson_t bValue;
            oReply.GetObject("value",&bValue);
            CBSONObject oValue(&bValue);
            count = oValue.GetInt64(fieldName);
        }
    }
    else{
        ShowError(StringFormat("IncreaseField(%s)",fieldName).c_str());
    }

    //printf("REPLY [%s]\n",oReply.ToJson().c_str());

    bson_destroy (update);
    bson_destroy (&query);
    mongoc_find_and_modify_opts_destroy (opts);
    return bOk;
}