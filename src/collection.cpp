#include "collection.h"
#include "database.h"
#include "query.h"
#include "result.h"
#include "bson/core.h"

#include <vector>
#include <system_error>
#include <bsoncxx/json.hpp>
//#include <mongocxx/stdx.hpp>

int Collection::META;

Collection::~Collection()
{
	client->free();
}

Collection::Ptr* Collection::CheckSelf(Lua::ILuaBase* LUA, int iStackPos)
{
	LUA->CheckType(iStackPos, META);
	return LUA->GetUserType<Ptr>(iStackPos, META);
}

int Collection::__gc(lua_State* L) 
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (ptr)
		ptr->free();

	return 0;
}

int Collection::__tostring(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	std::string out = "MongoDB Collection: " + Global::PtrToStr(ptr);

	LUA->PushString(out.c_str());
	return 1;
}

int Collection::InsertOne(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto doc = BSON::Core::ParseTable(LUA, 2);

	int func = 0;
	if (LUA->IsType(3, Lua::Type::Function)) {
		LUA->Push(3);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, doc, func](Lua::ILuaBase* LUA, Query* q) {
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto res = obj->coll.insert_one(doc.view());
			if (res)
				r.SetResult(res.value());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
		});
	});

	return 1;
}

int Collection::InsertMany(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	std::vector<bsoncxx::document::value> docs;
	LUA->PushNil();
	while (LUA->Next(2)) {
		docs.push_back(BSON::Core::ParseTable(LUA, -1));
		LUA->Pop();
	}

	int func = 0;
	if (LUA->IsType(3, Lua::Type::Function)) {
		LUA->Push(3);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, docs, func](Lua::ILuaBase* LUA, Query* q) {
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto res = obj->coll.insert_many(docs);
			if (res)
				r.SetResult(res.value());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
		});
	});

	return 1;
}

int Collection::FindOne(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto filter = BSON::Core::ParseTable(LUA, 2);

	int func = 0;
	if (LUA->IsType(3, Lua::Type::Function)) {
		LUA->Push(3);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, filter, func](Lua::ILuaBase* LUA, Query* q) {
		bsoncxx::stdx::optional<bsoncxx::document::value> doc;
		auto obj = ptr->guard(false);
		Result r;

		try {
			doc = obj->coll.find_one(filter.view());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, doc, func](Lua::ILuaBase* LUA) mutable {
			if (doc) {
				BSON::Core::ParseBSON(LUA, doc.value());
				r.Data(LUA);
			}

			r.Call(LUA, func);
		});
	});

	return 1;
}

int Collection::Find(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto filter = BSON::Core::ParseTable(LUA, 2);

	int func = 0;
	if (LUA->IsType(3, Lua::Type::Function)) {
		LUA->Push(3);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, filter, func](Lua::ILuaBase* LUA, Query* q) {
		std::vector<bsoncxx::document::value> docs;
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto cur = obj->coll.find(filter.view());
			for (auto&& doc : cur) {
				docs.emplace_back(doc);
			}
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, docs, func](Lua::ILuaBase* LUA) mutable {
			if (!docs.empty()) {
				int k = 1;
				LUA->CreateTable();
				for (auto&& doc : docs) {
					LUA->PushNumber(k++);
					BSON::Core::ParseBSON(LUA, doc.view());
					LUA->SetTable(-3);
				}

				r.Data(LUA);
			}

			r.Call(LUA, func);
		});
	});

	return 1;
}

int Collection::UpdateOne(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto filter = BSON::Core::ParseTable(LUA, 2);

	LUA->CheckType(3, Lua::Type::Table);
	auto update = BSON::Core::ParseTable(LUA, 3);

	int func = 0;
	if (LUA->IsType(4, Lua::Type::Function)) {
		LUA->Push(4);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, filter, update, func](Lua::ILuaBase* LUA, Query* q) {
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto res = obj->coll.update_one(filter.view(), update.view());
			if (res)
				r.SetResult(res.value());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
			});
		});

	return 1;
}

int Collection::UpdateMany(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto filter = BSON::Core::ParseTable(LUA, 2);

	LUA->CheckType(3, Lua::Type::Table);
	auto update = BSON::Core::ParseTable(LUA, 3);

	int func = 0;
	if (LUA->IsType(4, Lua::Type::Function)) {
		LUA->Push(4);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, filter, update, func](Lua::ILuaBase* LUA, Query* q) {
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto res = obj->coll.update_many(filter.view(), update.view());
			if (res)
				r.SetResult(res.value());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
		});
	});

	return 1;
}

int Collection::DeleteOne(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto filter = BSON::Core::ParseTable(LUA, 2);

	int func = 0;
	if (LUA->IsType(3, Lua::Type::Function)) {
		LUA->Push(3);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, filter, func](Lua::ILuaBase* LUA, Query* q) {
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto res = obj->coll.delete_one(filter.view());
			if (res)
				r.SetResult(res.value());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
		});
	});

	return 1;
}

int Collection::DeleteMany(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad collection object");
		return 0;
	}

	LUA->CheckType(2, Lua::Type::Table);
	auto filter = BSON::Core::ParseTable(LUA, 2);

	int func = 0;
	if (LUA->IsType(3, Lua::Type::Function)) {
		LUA->Push(3);
		func = LUA->ReferenceCreate();
	}

	ptr->add();
	Query::New(LUA, [ptr, filter, func](Lua::ILuaBase* LUA, Query* q) {
		auto obj = ptr->guard(false);
		Result r;

		try {
			auto res = obj->coll.delete_many(filter.view());
			if (res)
				r.SetResult(res.value());
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
			});
		});

	return 1;
}

int Collection::New(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = Database::CheckSelf(LUA);
	if (!ptr) {
		LUA->ArgError(1, "Bad database object");
		return 0;
	}

	const char* name = LUA->CheckString(2);
	if (std::string(name).empty()) {
		LUA->ArgError(2, "Empty collection name");
		return 0;
	}

	ptr->add();
	auto db = ptr->get();
	auto obj = new Collection;
	obj->client = db->client;
	obj->coll = db->db.collection(name);

	LUA->PushUserType(new Ptr(obj), META);
	return 1;
}

void Collection::Initialize(Lua::ILuaBase* LUA)
{
	META = LUA->CreateMetaTable("MongoDB Collection");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		LUA->PushCFunction(__tostring); LUA->SetField(-2, "__tostring");
		LUA->PushCFunction(__gc); LUA->SetField(-2, "__gc");

		LUA->PushCFunction(InsertOne); LUA->SetField(-2, "InsertOne");
		LUA->PushCFunction(InsertMany); LUA->SetField(-2, "InsertMany");
		LUA->PushCFunction(FindOne); LUA->SetField(-2, "FindOne");
		LUA->PushCFunction(Find); LUA->SetField(-2, "Find");
		LUA->PushCFunction(UpdateOne); LUA->SetField(-2, "UpdateOne");
		LUA->PushCFunction(UpdateMany); LUA->SetField(-2, "UpdateMany");
		LUA->PushCFunction(DeleteOne); LUA->SetField(-2, "DeleteOne");
		LUA->PushCFunction(DeleteMany); LUA->SetField(-2, "DeleteMany");
	LUA->Pop();
}