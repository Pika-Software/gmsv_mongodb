#ifndef BSON_TYPES_H
#define BSON_TYPES_H

#include "../main.h"

#include <bsoncxx/types/bson_value/view.hpp>
#include "types/oid.h"

namespace BSON
{
	class Types {
	public:
		static void Initialize(Lua::ILuaBase* LUA);
	};
}

#endif // BSON_TYPES_H