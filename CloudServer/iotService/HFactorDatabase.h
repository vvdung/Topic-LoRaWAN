#pragma once

#include "../inc/MongoDatabase.h"

#define DB_NAME_CONFIG  "config"//_test"
#define DB_NAME_GROUP   "groups"//_test"
#define DB_NAME_USERS   "users"//_test"
#define DB_NAME_GATES   "gates"//_test"
#define DB_NAME_NODES   "nodes"//_test"
#define DB_NAME_SENSORS "sensors"//_test"

class CHFactorDatabase : public CMongoDatabase
{
public:
    CHFactorDatabase(const char* dbName, CMongoClient* client);
    virtual ~CHFactorDatabase();

    void    InitIndexes();
    bool    AddGroup(const char* name, const char* desc);

    bool    GetHFUser(const char* email,HFUser  &user);
    bool    IsGatewayBelongUser(uint64_t gid,uint64_t uid);
    bool    IsNodeBelongGateway(uint64_t nid,uint64_t gid);
    bool    GetGateways(uint64_t uid,std::vector<HFGate>& lstGate,int start = 0, int limit = 10);
    bool    GetNodes(uint64_t uid,uint64_t gid,std::vector<HFNode>& lstNode,int start = 0, int limit = 10);
    bool    AddSensor(uint64_t gid,uint64_t nid,CBSONObject& oSensor,uint64_t& sid);
protected:
    //virtual CMongoCollection*   OnNewCollection(mongoc_collection_t *col,const char* colName);
    
};