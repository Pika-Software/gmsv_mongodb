#ifndef BSON_TYPES_OBJECTID
#define BSON_TYPES_OBJECTID

#include "../../main.h"
#include "../core.h"
#include <bsoncxx/types/bson_value/view.hpp>

namespace BSON
{
	namespace Types
	{
		namespace ObjectID
		{
			extern int META;

			struct ObjectIdStruct {
				bsoncxx::types::bson_value::view oid;
			};

			std::string StreamBytesToHex(const char* bytes, int size);

			int META__TOSTRING(lua_State* L);

			int Value(lua_State* L);

			void New(ILuaBase* LUA, bsoncxx::types::bson_value::view oid);

			void Initialize(ILuaBase* LUA);
		}
	}
}

#endif // BSON_TYPES_OBJECTID