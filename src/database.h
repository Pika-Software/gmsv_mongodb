#ifndef DATABASE_H
#define DATABASE_H

#include "main.h"
#include "client.h"
#include <mongocxx/database.hpp>

class Database {
public:
	typedef SmartPointer<Database> Ptr;

	static int META;

	mongocxx::database db;
	Client::Ptr* client;

	~Database();
	static Ptr* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
	static int __gc(lua_State* L);
	static int __tostring(lua_State* L);
	static int Name(lua_State* L);
	static int Drop(lua_State* L);
	static int HasCollection(lua_State* L);
	static int ListCollections(lua_State* L);
	static int New(lua_State* L);

	static void Initialize(Lua::ILuaBase* LUA);
};

#endif // DATABASE_H