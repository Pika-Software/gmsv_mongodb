#include "client.h"
#include "database.h"
#include "result.h"
#include "query.h"
#include "bson/core.h"

#include <thread>
#include <queue>
#include <system_error>
#include <mongocxx/uri.hpp>

int Client::META;
std::queue<Client*> clients;

Client::~Client()
{
	Global::DevMsg(1, "Disconnecting from server... (or just destroying pool)\n");

	if (pool)
		delete pool;
}

int Client::Status()
{
	return status.load();
}

void Client::Status(int s)
{
	status.store(s);
}

Client::Ptr* Client::CheckSelf(Lua::ILuaBase* LUA, int iStackPos)
{
	LUA->CheckType(iStackPos, META);
	return LUA->GetUserType<Ptr>(iStackPos, META);
}

int Client::__gc(lua_State* L) noexcept
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	if (ptr)
		ptr->free();

	return 0;
}

int Client::__tostring(lua_State* L) noexcept
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);
	std::string out = "MongoDB Client: " + Global::PtrToStr(ptr);

	LUA->PushString(out.c_str());
	return 1;
}

int Client::New(lua_State* L) noexcept
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto uri = LUA->CheckString(2);

	try {
		Global::DevMsg(1, "Creating client object\n");
		auto obj = new Client;
		obj->status.store(DISCONNECTED);
		obj->pool = new mongocxx::pool{ mongocxx::uri(uri) };

		LUA->PushUserType(new Ptr(obj), META);
		return 1;
	}
	catch (std::system_error err) {
		LUA->ArgError(2, "Invalid mongodb connection string!");
		return 0;
	}
}

int Client::Connect(lua_State* L) noexcept
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
	ptr->get()->Status(CONNECTING);
	Query::New(LUA, [ptr, func](Lua::ILuaBase* LUA, Query* q) {
		Result r;
		auto obj = ptr->guard(false);

		try {
			auto c = obj->pool->acquire();
			Global::DevMsg(1, "Connecting to the server... (uri: `%s`)\n", c->uri().to_string().c_str());
			c->start_session();
			obj->Status(CONNECTED);
		}
		catch (std::system_error err) {
			Global::DevMsg(1, "Failed to connect. Reason: %s\n", err.what());
			obj->Status(FAILED);
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func](Lua::ILuaBase* LUA) mutable {
			r.Call(LUA, func);
		});
	});

	return 1;
}

int Client::Status(lua_State* L) noexcept
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto ptr = CheckSelf(LUA);

	int s = ptr ? ptr->get()->Status() : DESTROYED;
	LUA->PushNumber(s);

	return 1;
}

int Client::ListDatabases(lua_State* L) noexcept
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
	Query::New(LUA, [ptr, func](Lua::ILuaBase* LUA, Query* q) {
		Result r;
		auto obj = ptr->guard(false);
		std::vector<bsoncxx::document::value> docs;

		try {
			auto c = obj->pool->acquire();
			auto cur = c->list_databases();

			for (auto&& doc : cur) {
				docs.push_back(bsoncxx::document::value(doc));
			}
		}
		catch (std::system_error err) {
			r.Error(err.code().value(), err.what());
		}

		q->Acquire(LUA, [r, func, docs](Lua::ILuaBase* LUA) mutable {
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

void Client::Initialize(Lua::ILuaBase* LUA)
{
	META = LUA->CreateMetaTable("MongoDB Client");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		LUA->PushCFunction(__tostring); LUA->SetField(-2, "__tostring");
		LUA->PushCFunction(__gc); LUA->SetField(-2, "__gc");
		LUA->PushCFunction(Database::New); LUA->SetField(-2, "__call");

		LUA->PushNumber(DISCONNECTED); LUA->SetField(-2, "STATUS_DISCONNECTED");
		LUA->PushNumber(CONNECTING); LUA->SetField(-2, "STATUS_CONNECTING");
		LUA->PushNumber(CONNECTED); LUA->SetField(-2, "STATUS_CONNECTED");
		LUA->PushNumber(FAILED); LUA->SetField(-2, "STATUS_FAILED");
		LUA->PushNumber(DESTROYED); LUA->SetField(-2, "STATUS_DESTROYED");

		LUA->PushCFunction(Connect); LUA->SetField(-2, "Connect");
		LUA->PushCFunction(Status); LUA->SetField(-2, "Status");
		LUA->PushCFunction(ListDatabases); LUA->SetField(-2, "ListDatabases");
		LUA->PushCFunction(Database::New); LUA->SetField(-2, "Database");
	LUA->Pop();
}

void Client::Deinitialize(Lua::ILuaBase* LUA)
{
	Global::DevMsg(1, "Clients should be destroyed now!\n");
}