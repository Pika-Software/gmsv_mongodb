#ifndef BSON_TYPES_DOCUMENT
#define BSON_TYPES_DOCUMENT

#include "../../main.h"
#include "../core.h"
#include <bsoncxx/document/value.hpp>

namespace BSON 
{
	namespace Types 
	{
		namespace Document 
		{
			extern int META;

			struct DocumentStruct {
				bsoncxx::document::value doc;
			};

			int META__TOSTRING(lua_State* L);

			int Parse(lua_State* L);

			int ToJSON(lua_State* L);

			void Initialize(ILuaBase* LUA);
		}
	}
}

#endif // BSON_TYPES_DOCUMENT