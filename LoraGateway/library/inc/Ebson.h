#pragma once
#include "bson.h"

class CBSONIter : public bson_iter_t {
public:
	CBSONIter();
	~CBSONIter();
	 
	bool	Next();
	bool	IsArray();
	bool	Recurse(CBSONIter* it);

	std::string	GetKey();
	
	bool		GetValue(const char* key, int& value);    
	bool		GetValue(const char* key, int64_t& value);
	bool		GetValue(const char* key, std::string& value);	

	int			GetInt();
	int64_t		GetInt64();
	double		GetDouble();
	std::string	GetUTF8();
	std::string	GetSubDocument();
	bool		GetObject(bson_t* b);

};

class CBSONObject {
public:
	CBSONObject(int type = BSON_TYPE_DOCUMENT);
	CBSONObject(bson_t* p);
	virtual ~CBSONObject();
	
	static bson_t* Parse(const char* szJson, bson_error_t* error = NULL);

	void 	Reset();
	bson_t* Get();	

	bool	AddMember(const char* key, int value);
	bool	AddMember(const char* key, int64_t value);
	bool	AddMember(const char* key, double value);
	bool	AddMember(const char* key, const char* value);
	bool	AddMember(const char* key, bson_t* doc);
	bool	AddMember(const char* key, CBSONObject* oDoc);

	bool	ArrayBegin(const char* name, CBSONObject* arrBson);
	bool	ArrayEnd(CBSONObject* arrBson);
	
	bool 	HasKey(const char* key);
	bool	KeyFirst(CBSONIter* it);				//key first of doc
	bool	KeyExist(const char* key, CBSONIter* it);	//
	
	bool	GetMember(const char* key, CBSONIter* it);

	int 		GetInt(const char* key);
	uint64_t	GetInt64(const char* key);
	double		GetDouble(const char* key);
	const char*	GetUTF8(const char* key);
	bool 		GetObject(const char* key,bson_t* b);
    std::string ToJson();
    std::string ToJsonRelaxed();
    std::string ToJsonCanonical();
	void	Save(const char* fName);
protected:    
	bson_t*	m_bson;	
};

class CBSONArray : public CBSONObject {
public:
	CBSONArray();
	virtual ~CBSONArray();

	bool	AddElement(int idx, CBSONObject* b);
};