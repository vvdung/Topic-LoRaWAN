#include "../inc/HFactor.h"
#include "../inc/MongoClient.h"
#include "../inc/MongoDatabase.h"

CMongoDatabase::CMongoDatabase(const char* dbName, CMongoClient* client){
    _database = NULL;    
    _client = client;
    if (!client) return;
    _database = mongoc_client_get_database (client->Get(), dbName);

}

CMongoDatabase::~CMongoDatabase(){
    if (_database){
        mongoc_database_destroy (_database);
    }    
}

mongoc_database_t * CMongoDatabase::Get(){
    return _database;
}

const char* CMongoDatabase::GetName(){
    return mongoc_database_get_name(_database);
}

CMongoCollection* CMongoDatabase::OnNewCollection(mongoc_collection_t *col,const char* colName){
    return new CMongoCollection(col,_client);
}
CMongoCollection* CMongoDatabase::GetCollection(const char* colName){
    CMongoCollection* pCollection = _mapCollection[colName];
    if (!pCollection){
        mongoc_collection_t *col = mongoc_client_get_collection (_client->Get(), GetName(), colName);
        if (!col) return NULL;
        pCollection = OnNewCollection(col,colName);//new CMongoCollection(col,_client);
        _mapCollection[colName] = pCollection;
    }    
    return pCollection;
}

bool CMongoDatabase::WriteCommand(CBSONObject* oCommand, CBSONObject* oOption){
    if (!oCommand) return false;
    //printf("[>] COMMAND (%s)\n",oCommand->ToJson().c_str());
    
    bson_t* opts = NULL;
    if (oOption) opts = oOption->Get();

    CBSONObject oReply;
    bson_error_t error;
    bool r = mongoc_database_write_command_with_opts(_database,oCommand->Get(),opts,oReply.Get(),&error);
    if (!r){
        printf("[?] Error: %d (%s)\n",error.code,error.message);
        return false;
    }

    //printf("[#] REPLY (%s)\n",oReply.ToJson().c_str());
    return true;
}
