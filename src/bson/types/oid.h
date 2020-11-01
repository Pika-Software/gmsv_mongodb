#ifndef BSON_TYPES_OBJECTID
#define BSON_TYPES_OBJECTID

#include "../../main.h"
#include "../core.h"
#include <bsoncxx/oid.hpp>

namespace BSON
{
	namespace Type
	{
		extern int META_OBJECTID;

		class ObjectID {
		public:
			bsoncxx::oid oid;

			static std::string BytesToHex(const char* bytes, int size);
			static ObjectID* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
			static int __gc(lua_State* L);
			static int __tostring(lua_State* L);
			static int Value(lua_State* L);
			static int IsValid(lua_State* L);
			static int New(lua_State* L);
			static void New(Lua::ILuaBase* LUA, bsoncxx::types::bson_value::view oid);
			static void New(Lua::ILuaBase* LUA, bsoncxx::oid oid);

			static void Initialize(Lua::ILuaBase* LUA);
		};
	}
}

#endif // BSON_TYPES_OBJECTID