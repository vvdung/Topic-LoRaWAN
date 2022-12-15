#include "main.h"
#include "HFactorDatabase.h"

CHFactorDatabase::CHFactorDatabase(const char* dbName, CMongoClient* client)
:CMongoDatabase(dbName,client)
{

}
CHFactorDatabase::~CHFactorDatabase(){

}

/*CMongoCollection* CHFactorDatabase::OnNewCollection(mongoc_collection_t *col, const char* colName){

    if (strcmp(colName,DB_NAME_CONFIG) == 0) return new CConfigCollection(col,_client);
    if (strcmp(colName,DB_NAME_GROUP)  == 0) return new CGroupCollection(col,_client);
    if (strcmp(colName,DB_NAME_USERS)  == 0) return new CUsersCollection(col,_client);
    
    return CMongoDatabase::OnNewCollection(col,colName);
}*/

void CHFactorDatabase::InitIndexes(){
    //GROUP
    CMongoCollection*  pCol_Group = GetCollection(DB_NAME_GROUP);
    CBSONObject oKey;
	oKey.AddMember("gid",1);
	pCol_Group->CreateIndexes(&oKey,true);
	oKey.Reset();
	oKey.AddMember("name",1);
	oKey.AddMember("gid",-1);
	pCol_Group->CreateIndexes(&oKey);

    //USERS
    CMongoCollection*  pCol_Users = GetCollection(DB_NAME_USERS);
    oKey.Reset();
	oKey.AddMember("uid",1);
	pCol_Users->CreateIndexes(&oKey,true);

    oKey.Reset();
	oKey.AddMember("kind",1);
	oKey.AddMember("uid",1);
	pCol_Users->CreateIndexes(&oKey);
}

bool CHFactorDatabase::AddGroup(const char* name, const char* desc){
    CMongoCollection* pCol_config_test = GetCollection(DB_NAME_CONFIG);
	uint64_t gid;
    if (!pCol_config_test->IncreaseField("nextGroupId",gid)) return false;    
    CMongoCollection*  pCol_group_test = GetCollection(DB_NAME_GROUP);
    CBSONObject oDoc;
    oDoc.AddMember("gid",(int64_t)gid);
	oDoc.AddMember("name",name);
	oDoc.AddMember("desc",desc);
	return pCol_group_test->InsertOne(&oDoc);
}

bool CHFactorDatabase::GetHFUser(const char* email,HFUser  &user){
    //return GetUsersCollection()->GetHFUser(email, user);
    CMongoCollection* pCollection = GetCollection(DB_NAME_USERS);
    CBSONObject oQuery;
    oQuery.AddMember("hfactor.email",email);
    CBSONObject oProjection;
    oProjection.AddMember("_id",0);
    oProjection.AddMember("__v",0);
    CBSONObject oOption;	
	oOption.AddMember("projection",&oProjection);
	oOption.AddMember("limit",1);

    mongoc_cursor_t *cursor;
    cursor = mongoc_collection_find_with_opts (
      pCollection->Get(),
      oQuery.Get(),
      oOption.Get(),  /* additional options */
      NULL); /* read prefs, NULL for default */
    
    printf("[%p] cursor\n",cursor);
    if (!cursor) return false;
    
    bool bFound = false;
    const bson_t *doc;
    if (mongoc_cursor_next (cursor, &doc)) {
        bFound = true;
        CBSONObject oDoc((bson_t*)doc);         
        bson_t bHFactor;
        oDoc.GetObject("hfactor",&bHFactor);
        CBSONObject oHFactor(&bHFactor);            
        user.szUsername = oHFactor.GetUTF8("username");
        user.szFullname = oHFactor.GetUTF8("fullname");
        user.szEmail = oHFactor.GetUTF8("email");
        user.uid = oDoc.GetInt64("uid");        
        user.gid = oDoc.GetInt64("gid");
    }    
    mongoc_cursor_destroy (cursor);
    return bFound;
}
bool CHFactorDatabase::IsGatewayBelongUser(uint64_t gid,uint64_t uid){
    CMongoCollection* pCollection = GetCollection(DB_NAME_GATES);
    CBSONObject oQuery;
    oQuery.AddMember("uid",(int64_t)uid);
    oQuery.AddMember("gid",(int64_t)gid);
    CBSONObject oProjection;
    oProjection.AddMember("_id",0);
    oProjection.AddMember("__v",0);
    CBSONObject oOption;	
	oOption.AddMember("projection",&oProjection);
	oOption.AddMember("limit",1);

    mongoc_cursor_t *cursor;
    cursor = mongoc_collection_find_with_opts (
      pCollection->Get(),
      oQuery.Get(),
      oOption.Get(),  /* additional options */
      NULL); /* read prefs, NULL for default */
    
    
    if (!cursor) return false;
    
    const bson_t *doc;
    bool  bFound = mongoc_cursor_next (cursor, &doc);
    mongoc_cursor_destroy (cursor); 

    return bFound;
}
bool CHFactorDatabase::IsNodeBelongGateway(uint64_t nid,uint64_t gid){
    CMongoCollection* pCollection = GetCollection(DB_NAME_NODES);
    CBSONObject oQuery;
    oQuery.AddMember("nid",(int64_t)nid);
    oQuery.AddMember("gid",(int64_t)gid);
    CBSONObject oProjection;
    oProjection.AddMember("_id",0);
    oProjection.AddMember("__v",0);
    CBSONObject oOption;	
	oOption.AddMember("projection",&oProjection);
	oOption.AddMember("limit",1);

    mongoc_cursor_t *cursor;
    cursor = mongoc_collection_find_with_opts (
      pCollection->Get(),
      oQuery.Get(),
      oOption.Get(),  /* additional options */
      NULL); /* read prefs, NULL for default */
    
    
    if (!cursor) return false;
    
    const bson_t *doc;
    bool  bFound = mongoc_cursor_next (cursor, &doc);
    mongoc_cursor_destroy (cursor); 

    return bFound;
}
bool CHFactorDatabase::GetGateways(uint64_t uid,std::vector<HFGate>& lstGate,int start, int limit){
    CMongoCollection* pCollection = GetCollection(DB_NAME_GATES);
    CBSONObject oQuery;
    oQuery.AddMember("uid",(int64_t)uid);
    CBSONObject oProjection;
    oProjection.AddMember("_id",0);
    oProjection.AddMember("__v",0);
    CBSONObject oOption;	
	oOption.AddMember("projection",&oProjection);
    if (start < 0) start = 0;
    oOption.AddMember("skip",start);
    if (limit < 1) limit = 1;
	oOption.AddMember("limit",limit);

    mongoc_cursor_t *cursor;
    cursor = mongoc_collection_find_with_opts (
      pCollection->Get(),
      oQuery.Get(),
      oOption.Get(),  /* additional options */
      NULL); /* read prefs, NULL for default */
    
    
    if (!cursor) return false;
    lstGate.clear();

    const bson_t *doc;
    while (mongoc_cursor_next (cursor, &doc)) {        
        CBSONObject oDoc((bson_t*)doc);         
        //printf("%s\n",oDoc.ToJson().c_str());
        HFGate hfGate;
		hfGate.gid = oDoc.GetInt64("gid");
		hfGate.uid = oDoc.GetInt64("uid");
		hfGate.szGateId = oDoc.GetUTF8("gwid");
		hfGate.szDesc = oDoc.GetUTF8("description");
		hfGate.date_created = oDoc.GetInt64("date_created") / 1000;
		hfGate.szGateKey = oDoc.GetUTF8("gwkey");
		lstGate.push_back(hfGate);    
    }    
    mongoc_cursor_destroy (cursor);
    
    return true;
}

bool CHFactorDatabase::GetNodes(uint64_t uid,uint64_t gid,std::vector<HFNode>& lstNode,int start, int limit){
    if (!IsGatewayBelongUser(gid,uid)){
        printf("Error - gid:%lu not belong uid:%lu\n",gid,uid);
        return false;
    }
    CMongoCollection* pCollection = GetCollection(DB_NAME_NODES);
    CBSONObject oQuery;
    //oQuery.AddMember("uid",(int64_t)uid);
    oQuery.AddMember("gid",(int64_t)gid);
    CBSONObject oProjection;
    oProjection.AddMember("_id",0);
    oProjection.AddMember("__v",0);
    CBSONObject oSort;
    oSort.AddMember("date_created",1);

    CBSONObject oOption;	
	oOption.AddMember("projection",&oProjection);
    if (start < 0) start = 0;
    oOption.AddMember("skip",start);
    if (limit < 1) limit = 1;
	oOption.AddMember("limit",limit);
    oOption.AddMember("sort",&oSort);
    

    mongoc_cursor_t *cursor;
    cursor = mongoc_collection_find_with_opts (
      pCollection->Get(),
      oQuery.Get(),
      oOption.Get(),  /* additional options */
      NULL); /* read prefs, NULL for default */
    
    lstNode.clear();
    if (!cursor) return false;
    const bson_t *doc;
    while (mongoc_cursor_next (cursor, &doc)) {        
        CBSONObject oDoc((bson_t*)doc);         
        //printf("%s\n",oDoc.ToJson().c_str()); 
        HFNode hfNode;
		if (oDoc.HasKey("nid")) hfNode.nid = oDoc.GetInt64("nid");
		if (oDoc.HasKey("gid")) hfNode.gid = oDoc.GetInt64("gid");
		if (oDoc.HasKey("nodeid")) hfNode.szNodeId = oDoc.GetUTF8("nodeid");
		if (oDoc.HasKey("description")) hfNode.szDesc = oDoc.GetUTF8("description");
		if (oDoc.HasKey("date_created")) hfNode.date_created = oDoc.GetInt64("date_created")/1000;
		if (oDoc.HasKey("nkey")) hfNode.szNodeKey = oDoc.GetUTF8("nkey");
        lstNode.push_back(hfNode);
    }    
    mongoc_cursor_destroy (cursor);    
    return true;
}

bool CHFactorDatabase::AddSensor(uint64_t gid,uint64_t nid,CBSONObject& oSensor,uint64_t& sid){
    if (!IsNodeBelongGateway(nid,gid)){
        printf("Node [%lu] not belongto Gate [%lu]\n",nid,gid);
        return false;
    }
    CMongoCollection* pConfig = GetCollection(DB_NAME_CONFIG);
    uint64_t uInc;
    if (!pConfig->IncreaseField("nextSensorId",uInc)){
        printf("Error - IncreaseField(nextSensorId)\n");
        return false;
    }
    sid = uInc;
    
    CBSONObject oData;
    oData.AddMember("sid",(int64_t)uInc);
    oData.AddMember("nid",(int64_t)nid);
    oData.AddMember("gid",(int64_t)gid);

    double tEpoch = (double)GetMiliseconds();
    oData.AddMember("date_created",tEpoch);
    oData.AddMember("sensors",&oSensor);
    CMongoCollection* pSensors = GetCollection(DB_NAME_SENSORS);
    if (!pSensors->InsertOne(&oData)){
        printf("SENSORS - Error - InsertOne()\n");
        return false;
    }
    //printf("[=>] gid:%lu nid:%lu nextSensorId [%lu] \n",gid,nid,uInc);
    return true;
}