#include "main.h"
#include "client.h"
#include "database.h"
#include "collection.h"
#include "query.h"
#include "bson/core.h"

#include <sstream>
#include <algorithm>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>

#ifdef WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#endif

f_DevMsg DevMsg;

int Test(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	return 1;
}

void Global::LoadEngine()
{
#ifdef WIN32
	HINSTANCE hGetProcIDDLL = LoadLibrary("tier0.dll");

	if (!hGetProcIDDLL)
		abort();

	f_DevMsg func = (f_DevMsg)GetProcAddress(hGetProcIDDLL, "DevMsg");
	if (!func)
		abort();

	DevMsg = func;
#elif __linux__
	void *HANDLE;

	HANDLE = dlopen("libtier0_srv.so", RTLD_LAZY);
	if (!HANDLE) 
	{
		HANDLE = dlopen("libtier0.so", RTLD_LAZY);
		if (!HANDLE)
			abort();
	}

	f_DevMsg fn = (f_DevMsg)dlsym(HANDLE, "DevMsg");
	if (dlerror() != NULL)  
		abort();

	DevMsg = fn;
#endif
}

void Global::Run(Lua::ILuaBase* LUA, const char* func, int iArgs, int iResults)
{
	int x = -1 - iArgs;
	LUA->PushSpecial(Lua::SPECIAL_GLOB);
		LUA->Insert(x);
		LUA->GetField(x, func);
		LUA->Insert(x);
		LUA->PCall(iArgs, iResults, Lua::Type::Nil);
	LUA->Pop();
}

void Global::Print(Lua::ILuaBase* LUA, const char* message)
{
	LUA->PushString(message);
	Global::Run(LUA, "print", 1, 0);
}

std::string Global::PtrToStr(const void* addr)
{
	std::stringstream ss;
	ss << "0x" << addr;
	std::string str = ss.str();
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
	return str;
}

int Global::Initialize(Lua::ILuaBase* LUA)
{
	// MongoDB initialization
	mongocxx::instance inst{}; // This should be done only once.

	LoadEngine(); // For developer messages

	DevMsg(1, "Initializing MongoDB...\n");
		
	// Modules initialization
	Client::Initialize(LUA); // Client
	Database::Initialize(LUA); // Database
	Collection::Initialize(LUA); // Collection
	Query::Initialize(LUA);
	BSON::Core::Initialize(LUA); // BSON

	// Global MongoDB metatable
	int meta = LUA->CreateMetaTable("MongoDB");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		LUA->PushCFunction(Client::New); LUA->SetField(-2, "__call"); // Creating client object
	LUA->Pop();

	// Global MongoDB table
	LUA->PushSpecial(Lua::SPECIAL_GLOB);
		LUA->CreateTable();
			LUA->PushMetaTable(meta); LUA->SetMetaTable(-2);
			LUA->PushString(VERSION); LUA->SetField(-2, "VERSION"); // Version
			LUA->PushCFunction(BSON::Type::ObjectID::New); LUA->SetField(-2, "ObjectID");
			LUA->PushCFunction(Test); LUA->SetField(-2, "Test");
		LUA->SetField(-2, "mongodb");
	LUA->Pop();

	return 0;
}

int Global::Deinitialize(Lua::ILuaBase* LUA)
{
	DevMsg(1, "Unloading MongoDB...\n");
	Client::Deinitialize(LUA);
	Query::Deinitialize();

	return 0;
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