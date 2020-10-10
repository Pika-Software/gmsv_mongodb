#include "database.h"
#include "client.h"
#include "collection.h"
#include "bson/core.h"

#include <mongocxx/database.hpp>
#include <mongocxx/exception/exception.hpp>

namespace Database
{
	int META;

	// Destroying database
	int META__GC(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto db = LUA->GetUserType<mongocxx::database>(1, META);
		db->~database();

		return 0;
	}

	// Returning database name
	int Name(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto db = LUA->GetUserType<mongocxx::database>(1, META);
		if (db == nullptr) {
			LUA->ArgError(1, "Invalid database object!");
			return 0;
		}

		LUA->PushString(db->name().data());
		return 1;
	}

	// Drop database
	int Drop(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto db = LUA->GetUserType<mongocxx::database>(1, META);
		if (db == nullptr) {
			LUA->ArgError(1, "Invalid database object!");
			return 0;
		}

		try {
			db->drop();
		} catch (mongocxx::exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		return 1;
	}

	// Check if database has collection
	int HasCollection(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto db = LUA->GetUserType<mongocxx::database>(1, META);
		if (db == nullptr) {
			LUA->ArgError(1, "Invalid database object!");
			return 0;
		}

		const char* name = LUA->CheckString(2);

		try {
			bool has = db->has_collection(name);
			LUA->PushBool(has);
		} catch (mongocxx::exception err) {
			LUA->ThrowError(err.what());
			return 0;
		}

		return 1;
	}

	// Returning list of collections in database
	int ListCollections(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		auto db = LUA->GetUserType<mongocxx::database>(1, META);
		if (db == nullptr) {
			LUA->ArgError(1, "Invalid database object!");
			return 0;
		}

		mongocxx::cursor cur = db->list_collections();
		LUA->CreateTable();
		int key = 1;
		for (auto&& doc : cur) {
			LUA->PushNumber(key++);
			try {
				BSON::Core::ParseBSON(LUA, doc);
			} catch(mongocxx::exception err) {
				LUA->ThrowError(err.what());
				return 0;
			}
			LUA->SetTable(-3);
		}

		return 1;
	}

	// Creating database object.
	int Database(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, Client::META);
		Client::ClientStruct* data = LUA->GetUserType<Client::ClientStruct>(1, Client::META);
		if (data == nullptr || data->status == Client::STATUS::DESTROYED || data->client == nullptr) {
			LUA->ArgError(1, "Invalid client!");
			return 0;
		}

		const char* db_name = LUA->CheckString(2);
		auto db = data->client->database(db_name);

		LUA->PushUserType_Value(db, META);
		return 1;
	}

	// Initialization
	void Initialize(ILuaBase* LUA)
	{
		META = LUA->CreateMetaTable("MongoDB Database");
			LUA->Push(-1); LUA->SetField(-2, "__index");
			LUA->PushCFunction(META__GC); LUA->SetField(-2, "__gc");
			
			LUA->PushCFunction(Name); LUA->SetField(-2, "Name");
			LUA->PushCFunction(Drop); LUA->SetField(-2, "Drop");
			LUA->PushCFunction(HasCollection); LUA->SetField(-2, "HasCollection");
			LUA->PushCFunction(ListCollections); LUA->SetField(-2, "ListCollections");
			LUA->PushCFunction(Collection::Collection); LUA->SetField(-2, "Collection");
		LUA->Pop();
	}
}