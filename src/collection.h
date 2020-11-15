#ifndef COLLECTION_H
#define COLLECTION_H

#include "main.h"
#include "client.h"
#include "pointer.h"
#include <mutex>
#include <mongocxx/collection.hpp>

class Collection {
public:
	typedef SmartPointer<Collection> Ptr;

	static int META;
	mongocxx::collection coll;
	Client::Ptr* client;
	~Collection();

	static Ptr* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
	static int __gc(lua_State* L);
	static int __tostring(lua_State* L);
	static int InsertOne(lua_State* L);
	static int InsertMany(lua_State* L);
	static int FindOne(lua_State* L);
	static int Find(lua_State* L);
	static int UpdateOne(lua_State* L);
	static int UpdateMany(lua_State* L);
	static int DeleteOne(lua_State* L);
	static int DeleteMany(lua_State* L);
	static int New(lua_State* L);
	static void Initialize(Lua::ILuaBase* LUA);
};

#endif // COLLECTION_H