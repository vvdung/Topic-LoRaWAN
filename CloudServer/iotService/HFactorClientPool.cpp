#include "main.h"
#include "HFactorClientPool.h"

CHFactorClientPool::CHFactorClientPool(const char *uri_string, const char *db_name):
CMongoClientPool(uri_string,db_name)
{

}

CHFactorClientPool::~CHFactorClientPool(){

}

CMongoClient* CHFactorClientPool::OnNewClient(mongoc_client_t* client){
    return new CHFactorClient(client,_dbName.c_str());
}

CHFactorClient* CHFactorClientPool::Pop(){
    return (CHFactorClient*)CMongoClientPool::Pop();
}