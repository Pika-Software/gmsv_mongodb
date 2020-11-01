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
	class Core {
	public:
		// Parse a BSON Document to Lua table
		static void ParseBSON(Lua::ILuaBase* LUA, bsoncxx::document::view view);
		// Parse a BSON Array to Lua table
		static void ParseBSON(Lua::ILuaBase* LUA, bsoncxx::array::view view);
		// Parse BSON value
		static void ParseBSONValue(Lua::ILuaBase* LUA, bsoncxx::types::bson_value::view view);

		// Iterate table at iStackPos.
		// True if every table keys is number.
		static bool TableIsArray(Lua::ILuaBase* LUA, int iStackPos);
		// Checks, if lua number is integer.
		static bool isInt(double val);

		// Iterating lua table as document at iStackPos.
		static void IterateTable(Lua::ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::document &doc);
		// Iterating lua table as array at iStackPos.
		static void IterateTable(Lua::ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::array &builder);
		// Parsing lua table at iStackPos
		static bsoncxx::document::value ParseTable(Lua::ILuaBase* LUA, int iStackPos);

		static void Initialize(Lua::ILuaBase* LUA);
	};
}

#endif // BSON_CORE_H