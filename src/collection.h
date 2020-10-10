#ifndef COLLECTION_H
#define COLLECTION_H

#include "main.h"

namespace Collection {
	extern int META;

	int META__GC(lua_State* L);

	int InsertOne(lua_State* L);

	int InsertMany(lua_State* L);

	int FindOne(lua_State* L);

	// Creating collection object
	int Collection(lua_State* L);

	// Initialization
	void Initialize(ILuaBase* LUA);
}

#endif // COLLECTION_H