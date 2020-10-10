#include "collection.h"
#include "database.h"
#include "bson/core.h"

#include <vector>
#include <mongocxx/stdx.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>


#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/logic_error.hpp>

namespace Collection 
{
	int META;

	int META__GC(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);

		if (coll != nullptr)
			coll->~collection();

		return 0;
	}

	int InsertOne(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		auto doc = BSON::Core::ParseTable(LUA, 2);

		mongocxx::stdx::optional<mongocxx::result::insert_one> result;
		try {
			result = coll->insert_one(doc.view());
		}
		catch (mongocxx::bulk_write_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();
			auto oid = val.inserted_id();

			LUA->CreateTable();
			BSON::Types::ObjectID::New(LUA, oid); LUA->SetField(-2, "id");
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}

	int InsertMany(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		int args = LUA->Top() - 1;

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);

		std::vector<bsoncxx::document::value> docs;
		for (int i = 0; i < args; i++) {
			LUA->CheckType(2, Type::Table);
			auto doc = BSON::Core::ParseTable(LUA, 2 + i);
			docs.push_back(doc);
		}

		mongocxx::stdx::optional<mongocxx::result::insert_many> result;
		try {
			result = coll->insert_many(docs);
		}
		catch (mongocxx::bulk_write_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();
			auto id_map = val.inserted_ids();
			
			LUA->CreateTable();
			LUA->PushNumber(val.inserted_count()); LUA->SetField(-2, "inserted_count");
			LUA->CreateTable();
				int k = 1;
				for (auto it = id_map.begin(); it != id_map.end(); it++) {
					auto el = it->second;
					LUA->PushNumber(k++);
					BSON::Types::ObjectID::New(LUA, el.get_value());
					LUA->SetTable(-3);
				}
			LUA->SetField(-2, "id_list");
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}

	int FindOne(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		auto doc = BSON::Core::ParseTable(LUA, 2);

		mongocxx::stdx::optional<bsoncxx::document::value> result;
		try {
			result = coll->find_one(doc.view());
		}
		catch (mongocxx::query_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();
				try {
					BSON::Core::ParseBSON(LUA, val.view());
				} catch(mongocxx::exception err) {
					LUA->ThrowError(err.what());
					return 0;
				}
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}

	int Find(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		auto doc = BSON::Core::ParseTable(LUA, 2);

		try {
			auto cur = coll->find(doc.view());

			LUA->CreateTable();
			int k = 1;
			for (auto doc : cur) {
				LUA->PushNumber(k++);
				try {
					BSON::Core::ParseBSON(LUA, doc);
				} catch(mongocxx::exception err) {
					LUA->ThrowError(err.what());
					return 0;
				}
				LUA->SetTable(-3);
			}
		}
		catch (mongocxx::query_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		return 1;
	}

	int UpdateOne(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		LUA->CheckType(3, Type::Table);

		auto filter = BSON::Core::ParseTable(LUA, 2);
		auto update = BSON::Core::ParseTable(LUA, 3);

		mongocxx::stdx::optional<mongocxx::result::update> result;
		try {
			result = coll->update_one(filter.view(), update.view());
		}
		catch (mongocxx::logic_error err) {
			LUA->ThrowError(err.what());
			return 0;
		}
		catch (mongocxx::bulk_write_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();
			auto id = val.upserted_id();

			LUA->CreateTable();
			LUA->PushNumber(val.matched_count()); LUA->SetField(-2, "matched_count");
			LUA->PushNumber(val.modified_count()); LUA->SetField(-2, "modified_count");
			if (id) {
				auto el = &id.value();
				BSON::Types::ObjectID::New(LUA, el->get_value()); LUA->SetField(-2, "id");
			}
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}

	int UpdateMany(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		LUA->CheckType(3, Type::Table);

		auto filter = BSON::Core::ParseTable(LUA, 2);
		auto update = BSON::Core::ParseTable(LUA, 3);

		mongocxx::stdx::optional<mongocxx::result::update> result;
		try {
			result = coll->update_many(filter.view(), update.view());
		}
		catch (mongocxx::logic_error err) {
			LUA->ThrowError(err.what());
			return 0;
		}
		catch (mongocxx::bulk_write_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();
			auto id = val.upserted_id();

			LUA->CreateTable();
			LUA->PushNumber(val.matched_count()); LUA->SetField(-2, "matched_count");
			LUA->PushNumber(val.modified_count()); LUA->SetField(-2, "modified_count");
			if (id) {
				auto el = &id.value();
				BSON::Types::ObjectID::New(LUA, el->get_value()); LUA->SetField(-2, "id");
			}
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}

	int DeleteOne(lua_State* L) 
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		auto doc = BSON::Core::ParseTable(LUA, 2);

		mongocxx::stdx::optional<mongocxx::result::delete_result> result;
		try {
			result = coll->delete_one(doc.view());
		}
		catch (mongocxx::bulk_write_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();

			LUA->CreateTable();
			LUA->PushNumber(val.deleted_count()); LUA->SetField(-2, "deleted_count");
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}

	int DeleteMany(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		auto doc = BSON::Core::ParseTable(LUA, 2);

		mongocxx::stdx::optional<mongocxx::result::delete_result> result;
		try {
			result = coll->delete_many(doc.view());
		}
		catch (mongocxx::bulk_write_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		if (result) {
			auto val = result.value();

			LUA->CreateTable();
			LUA->PushNumber(val.deleted_count()); LUA->SetField(-2, "deleted_count");
		}
		else {
			LUA->PushNil();
		}

		return 1;
	}


	int CreateIndex(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto coll = LUA->GetUserType<mongocxx::collection>(1, META);
		if (coll == nullptr) {
			LUA->ArgError(1, "Invalid collection object!");
			return 0;
		}

		LUA->CheckType(2, Type::Table);
		auto keys = BSON::Core::ParseTable(LUA, 2);

		bsoncxx::document::value options = bsoncxx::builder::basic::document().extract();
		if (LUA->IsType(3, Type::Table))
			options = BSON::Core::ParseTable(LUA, 3);

		try {
			auto doc = coll->create_index(keys.view(), options.view());
				try {
					BSON::Core::ParseBSON(LUA, doc.view());
				} catch(mongocxx::exception err) {
					LUA->ThrowError(err.what());
					return 0;
				}
		}
		catch (mongocxx::operation_exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		return 1;
	}

	// Creating collection object
	int Collection(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);


		LUA->CheckType(1, Database::META);
		auto db = LUA->GetUserType<mongocxx::database>(1, Database::META);
		if (db == nullptr) {
			LUA->ArgError(1, "Invalid database object!");
			return 0;
		}

		const char* name = LUA->CheckString(2);
		mongocxx::collection coll = db->collection(name);

		LUA->PushUserType_Value(coll, META);
		return 1;
	}

	// Initialization
	void Initialize(ILuaBase* LUA)
	{
		META = LUA->CreateMetaTable("MongoDB Collection");
			LUA->Push(-1); LUA->SetField(-2, "__index");
			LUA->PushCFunction(META__GC); LUA->SetField(-2, "__gc");

			LUA->PushCFunction(InsertOne); LUA->SetField(-2, "InsertOne");
			LUA->PushCFunction(InsertMany); LUA->SetField(-2, "InsertMany");
			LUA->PushCFunction(FindOne); LUA->SetField(-2, "FindOne");
			LUA->PushCFunction(Find); LUA->SetField(-2, "Find");
			LUA->PushCFunction(UpdateOne); LUA->SetField(-2, "UpdateOne");
			LUA->PushCFunction(UpdateMany); LUA->SetField(-2, "UpdateMany");
			LUA->PushCFunction(DeleteOne); LUA->SetField(-2, "DeleteOne");
			LUA->PushCFunction(DeleteMany); LUA->SetField(-2, "DeleteMany");
			LUA->PushCFunction(CreateIndex); LUA->SetField(-2, "CreateIndex");
		LUA->Pop();
	}
}