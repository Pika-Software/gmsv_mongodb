#ifndef DATABASE_H
#define DATABASE_H

#include "main.h"

namespace Database 
{
	extern int META;

	// Destroying database
	int META__GC(lua_State* L);

	// Returning database name
	int Name(lua_State* L);

	// Drop database
	int Drop(lua_State* L);

	// Check if database has collection
	int HasCollection(lua_State* L);

	// Returning list of collections in database
	int ListCollections(lua_State* L);

	// Creating database object.
	int Database(lua_State* L);

	// Initialization
	void Initialize(ILuaBase* LUA);
}

#endif // DATABASE_H