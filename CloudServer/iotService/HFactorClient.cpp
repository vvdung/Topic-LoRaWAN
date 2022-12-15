#include "main.h"
#include "HFactorClient.h"


CHFactorClient::CHFactorClient(mongoc_client_t* client,const char *db_name)
:CMongoClient(client,db_name)
{

}
CHFactorClient::~CHFactorClient(){

}

CHFactorDatabase* CHFactorClient::GetDatabase(){
    return (CHFactorDatabase*)_database;
}

CMongoDatabase* CHFactorClient::OnNewDatabase(const char *db_name){
    return new CHFactorDatabase(db_name,this);
}

