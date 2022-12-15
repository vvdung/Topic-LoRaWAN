#include "../inc/HFactor.h"
#include "../inc/MongoClient.h"


CMongoClient::CMongoClient(const char *uri_string, const char *db_name){
    _client = NULL;
    _database = NULL;    
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

   	_client = mongoc_client_new_from_uri (uri);
   	if (!_client) {
		printf("Error - mongoc_client_new_from_uri\n");
        mongoc_uri_destroy (uri);
      	return;
   	}
       
    mongoc_uri_destroy (uri);

    //_database = OnNewDatabase(db_name);//new CMongoDatabase(db_name,this);

}
CMongoClient::CMongoClient(mongoc_client_t* client,const char *db_name){
    _client = client;
    _database = NULL;    
    _dbName = db_name;
    //_database = OnNewDatabase(db_name);//new CMongoDatabase(db_name,this);
}

CMongoClient::~CMongoClient(){

    if (_database) delete _database;
    if (_client){        
        mongoc_client_destroy (_client);
    }
}
void CMongoClient::InitDatabase(){
    _database = OnNewDatabase(_dbName.c_str());
}
CMongoDatabase* CMongoClient::OnNewDatabase(const char *db_name){
    //printf("CMongoClient::OnNewDatabase(%s)\n",db_name);
    return new CMongoDatabase(db_name,this);
}
mongoc_client_t *CMongoClient::Get(){
    return _client;
}

CMongoDatabase* CMongoClient::GetDatabase(){
    return _database;
}

CMongoCollection* CMongoClient::GetCollection(const char* colName){
    return _database->GetCollection(colName);
}