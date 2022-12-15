#include "../inc/HFactor.h"
#include "../inc/Ebson.h"

CBSONIter::CBSONIter() {

}
CBSONIter::~CBSONIter() {

}

bool CBSONIter::Next()
{
	return bson_iter_next(this);
}

bool CBSONIter::IsArray()
{
	return BSON_ITER_HOLDS_ARRAY(this);
}

bool CBSONIter::Recurse(CBSONIter * it)
{
	return bson_iter_recurse(this, it);
}

std::string CBSONIter::GetKey()
{
	return std::string(bson_iter_key(this));
}

bool CBSONIter::GetValue(const char* key, int & value)
{	
	bson_iter_t tmp = *this;
	bson_iter_t it;
	if (!bson_iter_find_descendant(&tmp, key, &it)) return false;
	value = bson_iter_int32(&it);
	return true;
}

bool CBSONIter::GetValue(const char* key, int64_t & value)
{
	bson_iter_t tmp = *this;
	bson_iter_t it;
	if (!bson_iter_find_descendant(&tmp, key, &it)) return false;
	value = bson_iter_as_int64(&it);
	return true;
}

bool CBSONIter::GetValue(const char* key, std::string & value)
{
	bson_iter_t tmp = *this;
	bson_iter_t it;
	if (!bson_iter_find_descendant(&tmp, key, &it)) return false;
	value = StringFormat("%s", bson_iter_utf8(&it, nullptr));
	return true;
}

int CBSONIter::GetInt()
{
	return bson_iter_int32(this);
}

int64_t CBSONIter::GetInt64()
{
	return bson_iter_as_int64(this);
}

double CBSONIter::GetDouble(){
	return bson_iter_as_double(this);
}

std::string CBSONIter::GetUTF8()
{
	return std::string(bson_iter_utf8(this, nullptr));
}

bool CBSONIter::GetObject(bson_t* b){
	if (!b) return false;
	if (!BSON_ITER_HOLDS_DOCUMENT(this)) return false;
	uint32_t len = 0;
	const uint8_t *mDoc = NULL;
	bson_iter_document(this, &len, &mDoc);
	if (len <= 0) return false;	
	if (!bson_init_static(b, mDoc, len)) return false;
	return true;
}
std::string	CBSONIter::GetSubDocument(){
	bson_t b;
	if (!GetObject(&b)) return std::string("");
	CBSONObject o(&b);
	return std::string(o.ToJson());
}

////////////////////////////////////////////
CBSONObject::CBSONObject(int type) {	
	m_bson = bson_new();
	if (type == BSON_TYPE_DOCUMENT) bson_init(m_bson);
}

CBSONObject::CBSONObject(bson_t * p)
{
	m_bson = p;		
}

CBSONObject::~CBSONObject() {
	bson_clear(&m_bson);
}

bson_t * CBSONObject::Parse(const char* szJson, bson_error_t * error)
{
	return bson_new_from_json((const uint8_t*)szJson, -1, error);
}
void CBSONObject::Reset(){
	bson_clear(&m_bson);
	m_bson = bson_new();
	bson_init(m_bson);
}

bson_t * CBSONObject::Get()
{
	return m_bson;
}

bool CBSONObject::AddMember(const char* key, int value)
{
	return BSON_APPEND_INT32(m_bson, key, value);
}

bool CBSONObject::AddMember(const char* key, int64_t value)
{
	return BSON_APPEND_INT64(m_bson, key, value);
}

bool CBSONObject::AddMember(const char* key, double value){
	return BSON_APPEND_DOUBLE(m_bson, key, value);
}
bool CBSONObject::AddMember(const char* key, const char* value)
{
	return BSON_APPEND_UTF8(m_bson, key, value);
}

bool CBSONObject::AddMember(const char* key, bson_t* doc){
	return BSON_APPEND_DOCUMENT(m_bson, key, doc);
}
bool CBSONObject::AddMember(const char* key, CBSONObject* oDoc){
	return AddMember(key,oDoc->Get());
}

bool CBSONObject::ArrayBegin(const char* name, CBSONObject* arrBson)
{
	return BSON_APPEND_ARRAY_BEGIN(m_bson, name, arrBson->Get());
}

bool CBSONObject::ArrayEnd(CBSONObject *arrBson)
{
	return bson_append_array_end(m_bson, arrBson->Get());
}

bool CBSONObject::HasKey(const char* key){
	return bson_has_field(m_bson,key);
}
bool CBSONObject::KeyFirst(CBSONIter * it)
{
	return bson_iter_init(it, m_bson);
}

bool CBSONObject::KeyExist(const char* key, CBSONIter* it)
{
	return bson_iter_init_find(it, m_bson, key);
}

bool CBSONObject::GetMember(const char* key, CBSONIter * it)
{
	if (!KeyExist(key,it)) return false;
	if (BSON_ITER_HOLDS_DOCUMENT(it) || 
		BSON_ITER_HOLDS_ARRAY(it))
	 it->Recurse(it);
	return true;
}

int CBSONObject::GetInt(const char* key){
	CBSONIter it;
	KeyExist(key,&it);
	return it.GetInt();
}
uint64_t CBSONObject::GetInt64(const char* key){
	CBSONIter it;
	KeyExist(key,&it);
	return (uint64_t)it.GetInt64();
}
double CBSONObject::GetDouble(const char* key){
	CBSONIter it;
	KeyExist(key,&it);
	return it.GetDouble();
}
const char*	CBSONObject::GetUTF8(const char* key){
	CBSONIter it;
	KeyExist(key,&it);
	return bson_iter_utf8(&it, nullptr);
}
bool CBSONObject::GetObject(const char* key,bson_t* b){
	CBSONIter it;
	if (!KeyExist(key,&it)) return false;
	return it.GetObject(b);
}
std::string CBSONObject::ToJson(){
    if (!m_bson) return std::string("");
    char* p = bson_as_json (m_bson, NULL);
    std::string szRet(p);
    bson_free (p);
    return std::string(szRet);
}
std::string CBSONObject::ToJsonRelaxed(){
    if (!m_bson) return std::string("");
    char* p = bson_as_relaxed_extended_json (m_bson, NULL);
    std::string szRet(p);
    bson_free (p);
    return std::string(szRet);
}
std::string CBSONObject::ToJsonCanonical(){
    if (!m_bson) return std::string("");
    char* p = bson_as_canonical_extended_json (m_bson, NULL);
    std::string szRet(p);
    bson_free (p);
    return std::string(szRet);
}

void CBSONObject::Save(const char* fName)
{
	size_t len;
	char *str = bson_as_json(m_bson, &len);
	FILE* f = fopen(fName,"rw+");
	if (f) {
		fwrite(str, 1,len,f);
		fclose(f);
	}
	bson_free(str);
}

/////////////////////////////////////////
CBSONArray::CBSONArray():CBSONObject(BSON_TYPE_ARRAY) {

}

CBSONArray::~CBSONArray() {

}
bool CBSONArray::AddElement(int idx, CBSONObject * b)
{
	std::string szIndex = StringFormat("%d", idx);
	return BSON_APPEND_DOCUMENT(m_bson, szIndex.c_str(), b->Get());
}
