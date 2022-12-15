#include "../inc/HFactor.h"
#include "../inc/MongoClientPool.h"

CMongoClientPool::CMongoClientPool(const char *uri_string, const char *db_name){
    
    _pool = NULL;
    _dbName = db_name;

    std::string szUri = uri_string;
    szUri += db_name;
    bson_error_t error;
    mongoc_uri_t *uri;
    
    uri = mongoc_uri_new_with_error (szUri.c_str(), &error);
    if (!uri) {
        fprintf (stderr,
               "failed to parse URI: %s\n"
               "error message:       %s\n",
               szUri.c_str(),
               error.message);
        return;
    }
    _pool = mongoc_client_pool_new (uri);
    mongoc_client_pool_set_error_api (_pool, 2);

}
CMongoClientPool::~CMongoClientPool(){
    if (_pool) mongoc_client_pool_destroy (_pool);
}

CMongoClient* CMongoClientPool::OnNewClient(mongoc_client_t* client){
    return new CMongoClient(client,_dbName.c_str());
}
void CMongoClientPool::Push(CMongoClient *client){
    if (!_pool || !client || !client->Get()) return;    
    mongoc_client_pool_push(_pool, client->Get());
}

CMongoClient* CMongoClientPool::Pop(){
    if (!_pool) return NULL;
    mongoc_client_t* client = mongoc_client_pool_pop(_pool);
    if (!client) return NULL;
    CMongoClient* pClient = _mapClient[(uint64_t)client];
    if (!pClient){
        pClient = OnNewClient(client);
        pClient->InitDatabase();
        _mapClient[(uint64_t)client] = pClient;
    }
    return pClient;
}
/*
const char* CMongoClientPool::GetDatabaseName(){
    return _dbName.c_str();
}*/