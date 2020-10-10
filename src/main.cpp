#include "main.h"
#include "client.h"
#include "database.h"
#include "collection.h"
#include "bson/core.h"

#include <mongocxx/instance.hpp>

namespace Global
{
	// Running global function
	// On the top stack should be arguments
	// This function don't pop arguments from stack. You should do this manually
	void Run(ILuaBase* LUA, const char* func, int iArgs, int iResults)
	{
		LUA->PushSpecial(SPECIAL_GLOB);
			LUA->GetField(-1, func);
			for (int i = 0; i < iArgs; i++)
				LUA->Push(-2 - iArgs);
			LUA->Call(iArgs, iResults);
		LUA->Pop();
	}

#ifdef _DEBUG
	int Test(lua_State* L)
	{
		ILuaBase* LUA = L->luabase;
		LUA->SetState(L);

		return 1;
	}
#endif // _DEBUG

	// Garry's mod initialization
	int Initialize(ILuaBase* LUA)
	{
		// MongoDB initialization
		mongocxx::instance inst{}; // This should be done only once.
		
		// Modules initialization
		Client::Initialize(LUA); // Client
		Database::Initialize(LUA); // Database
		Collection::Initialize(LUA); // Collection
		BSON::Core::Initialize(LUA); // BSON

		// Global MongoDB table
		LUA->PushSpecial(SPECIAL_GLOB);
			LUA->CreateTable();
				LUA->PushString(VERSION); LUA->SetField(-2, "__VERSION"); // Version
				LUA->PushCFunction(Client::CreateClient); LUA->SetField(-2, "CreateClient"); // Creating client object
				LUA->PushCFunction(BSON::Core::FromJSON); LUA->SetField(-2, "FromJSON"); // Creating bson document from json

#ifdef _DEBUG
				LUA->PushCFunction(Test); LUA->SetField(-2, "Test"); // Ignore this
#endif // _DEBUG
			LUA->SetField(-2, "mongodb");
		LUA->Pop();

		return 0;
	}

	// Shutdown
	int Deinitialize(ILuaBase* LUA)
	{
		//MongoDB_Instance->~instance(); // Crashing gmod when server shutdown. Maybe fix it tommorow
		return 0;
	}
}

// Initalization Garry's mod
GMOD_MODULE_OPEN()
{
	return Global::Initialize(LUA);
}

GMOD_MODULE_CLOSE()
{
	return Global::Deinitialize(LUA);
}