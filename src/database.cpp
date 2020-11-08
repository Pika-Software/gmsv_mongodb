#include "database.h"
#include "client.h"
#include "collection.h"
#include "result.h"
#include "query.h"
#include "bson/core.h"

#include <system_error>

int Database::META;

Database::~Database()
{
	client->free();
}

Database::Ptr* Database::CheckSelf(Lua::ILuaBase* LUA, int iStackPos)
{
	LUA->CheckType(iStackPos, META);
	return LUA->GetUserType<Ptr>(iStackPos, META);
}

int Database::__gc(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto obj = CheckSelf(LUA);
	if (obj)
		obj->free();

	return 0;
}

int Database::__tostring(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto obj = CheckSelf(LUA);
	std::string out = "MongoDB Database: " + Global::PtrToStr(obj);

	LUA->PushString(out.c_str());
	return 1;
}

int Database::Name(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad database object");
		return 0;
	}

	LUA->PushString(ptr->get()->db.name().data());
	return 1;
}

int Database::Drop(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad database object");
		return 0;
	}

	Result r;
	auto obj = ptr->guard();

	try {
		obj->db.drop();
	}
	catch (std::system_error err) {
		r.Error(err.code().value(), err.what());
	}

	r.Push(LUA); r.Free(LUA);
	return 1;
}

int Database::HasCollection(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad database object");
		return 0;
	}

	auto name = LUA->CheckString(2);
	auto obj = ptr->guard();
	Result r;

	try {
		bool has = obj->db.has_collection(name);
		r.data["data"] = has;
	}
	catch (std::system_error err) {
		r.Error(err.code().value(), err.what());
	}

	r.Push(LUA); r.Free(LUA);
	return 1;
}

int Database::ListCollections(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad client");
		return 0;
	}

	int func = 0;
	if (LUA->IsType(2, Lua::Type::Function)) {
		LUA->Push(2);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, func](Query* q) {
		Result r;
		auto obj = ptr->guard(false);
		std::vector<bsoncxx::document::value> docs;

		try {
			auto cur = obj->db.list_collections();

			for (auto&& doc : cur) {
				docs.push_back(bsoncxx::document::value(doc));
			}
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire([r, func, docs](Lua::ILuaBase* LUA) mutable {
			int k = 1;
			LUA->CreateTable();
			for (auto&& doc : docs) {
				LUA->PushNumber(k++);
				BSON::Core::ParseBSON(LUA, doc.view());
				LUA->SetTable(-3);
			}
			r.Data(LUA);

			r.Call(LUA, func);
		});
	});

	return 1;
}

int Database::New(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);
	
	auto client = Client::CheckSelf(LUA);
	if (!client) {
		LUA->ArgError(1, "Bad client");
		return 0;
	}

	const char* name = LUA->CheckString(2);
	if (std::string(name).empty()) {
		LUA->ArgError(2, "Empty database name");
		return 0;
	}

	client->add();

	auto obj = new Database;
	obj->client = client;
	
	{
		auto c = client->get()->pool->acquire();
		obj->db = c->database(name);
	}

	LUA->PushUserType(new Ptr(obj), META);
	return 1;
}

void Database::Initialize(Lua::ILuaBase* LUA)
{
	META = LUA->CreateMetaTable("MongoDB Database");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		LUA->PushCFunction(__gc); LUA->SetField(-2, "__gc");
		LUA->PushCFunction(Collection::New); LUA->SetField(-2, "__call");

		LUA->PushCFunction(Name); LUA->SetField(-2, "Name");
		LUA->PushCFunction(Drop); LUA->SetField(-2, "Drop");
		LUA->PushCFunction(HasCollection); LUA->SetField(-2, "HasCollection");
		LUA->PushCFunction(ListCollections); LUA->SetField(-2, "ListCollections");
		LUA->PushCFunction(Collection::New); LUA->SetField(-2, "Collection");
	LUA->Pop();
}