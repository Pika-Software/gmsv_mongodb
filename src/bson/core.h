#ifndef BSON_CORE_H
#define BSON_CORE_H

#include "../main.h"
#include "types.h"

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/array/view.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

namespace BSON 
{
	namespace Core
	{
		// Parse a BSON Document to Lua table
		void ParseBSON(ILuaBase* LUA, bsoncxx::document::view view);

		// Parse a BSON Array to Lua table
		void ParseBSON(ILuaBase* LUA, bsoncxx::array::view view);

		// Iterate table at iStackPos.
		// True if every table keys is number.
		bool TableIsArray(ILuaBase* LUA, int iStackPos);

		// Checks, if lua number is integer.
		bool isInt(double val);

		// Iterating lua table as document at iStackPos.
		void IterateTable(ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::document &doc);

		// Iterating lua table as array at iStackPos.
		void IterateTable(ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::array& builder);

		// Parsing lua table at iStackPos
		bsoncxx::document::value ParseTable(ILuaBase* LUA, int iStackPos);

		// Creating BSON document from JSON
		int FromJSON(lua_State* L);

		// Initialization
		void Initialize(ILuaBase* LUA);
	}
}

#endif // BSON_CORE_H