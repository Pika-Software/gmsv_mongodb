#include "client.h"
#include "database.h"
#include "bson/core.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>

namespace Client
{
	int META;

	// Destroying client
	int META_GC(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		ClientStruct* data = LUA->GetUserType<ClientStruct>(1, META);
		if (data != nullptr && data->status != STATUS::DESTROYED && data->client != nullptr) {
			data->status = STATUS::DESTROYED;
			delete data->client;
		}

		return 0;
	}

	// Returning client status. 
	// See Client::STATUS enum.
	int Status(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);
		
		int status = STATUS::DESTROYED; // Default, client status is destroyed

		LUA->CheckType(1, META);
		ClientStruct* data = LUA->GetUserType<ClientStruct>(1, META);
		if (data != nullptr) {
			status = data->status;
		}

		LUA->PushNumber(status);
		return 1;
	}

	// Disconnecting from server.
	// After calling this function client destroying.
	int Disconnect(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		ClientStruct* data = LUA->GetUserType<ClientStruct>(1, META);
		if (data == nullptr || data->status == STATUS::DESTROYED || data->client == nullptr) {
			LUA->ArgError(1, "Invalid client!");
			return 0;
		}

		// Sorry, but i didn't found disconnect method :(
		data->status = STATUS::DESTROYED;
		delete data->client; // Destroying client

		return 1;
	}

	// Connecting to server
	int Connect(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		ClientStruct* data = LUA->GetUserType<ClientStruct>(1, META);
		if (data == nullptr || data->status == STATUS::DESTROYED || data->client == nullptr) {
			LUA->ArgError(1, "Invalid client!");
			return 0;
		}

		data->status = STATUS::CONNECTING; // Maybe i do threaded version of this wrapper. But not at this version
		try {
			data->client->start_session();
			data->status = STATUS::CONNECTED;
		} catch (mongocxx::exception err) {
			data->status = STATUS::FAILED;
			LUA->ThrowError(err.what()); // I don't know mongodb errors :(
			return 0;
		}
		
		return 1;
	}

	// Returning list of databases on server
	int ListDatabases(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		LUA->CheckType(1, META);
		ClientStruct* data = LUA->GetUserType<ClientStruct>(1, META);
		if (data == nullptr || data->status == STATUS::DESTROYED || data->client == nullptr) {
			LUA->ArgError(1, "Invalid client!");
			return 0;
		}

		try {
			mongocxx::cursor cur = data->client->list_databases();

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

			// cur.~cursor(); // this is crash gmod
		} catch (mongocxx::exception err) {
			std::string buf("Failed to get list of databases: "); // Dummy way to concat two strings
			buf.append(err.what());

			LUA->ThrowError(buf.c_str());
			return 0;
		}

		return 1;
	}

	// Creating MongoDB Client
	int CreateClient(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		const char* connect_str = LUA->CheckString(1); // MongoDB Connection string

		// Creating client
		mongocxx::uri uri(connect_str);
		auto client = new mongocxx::client(uri);

		// Creating client structure
		ClientStruct data;
		data.client = client;
		data.status = STATUS::DISCONNECTED;

		LUA->PushUserType_Value(data, META);
		return 1;
	}

	// Initialization
	void Initialize(ILuaBase* LUA)
	{
		META = LUA->CreateMetaTable("MongoDB Client");
			LUA->Push(-1); LUA->SetField(-2, "__index");
			LUA->PushCFunction(META_GC); LUA->SetField(-2, "__gc");

			LUA->PushNumber(STATUS::DISCONNECTED); LUA->SetField(-2, "STATUS_DISCONNECTED");
			LUA->PushNumber(STATUS::CONNECTING); LUA->SetField(-2, "STATUS_CONNECTING");
			LUA->PushNumber(STATUS::CONNECTED); LUA->SetField(-2, "STATUS_CONNECTED");
			LUA->PushNumber(STATUS::FAILED); LUA->SetField(-2, "STATUS_FAILED");
			LUA->PushNumber(STATUS::DESTROYED); LUA->SetField(-2, "STATUS_DESTROYED");

			LUA->PushCFunction(Disconnect); LUA->SetField(-2, "Disconnect");
			LUA->PushCFunction(Connect); LUA->SetField(-2, "Connect");
			LUA->PushCFunction(Status); LUA->SetField(-2, "Status");
			LUA->PushCFunction(ListDatabases); LUA->SetField(-2, "ListDatabases");
			LUA->PushCFunction(Database::Database); LUA->SetField(-2, "Database");
		LUA->Pop();
	}
}