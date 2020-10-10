#ifndef BSON_TYPES_H
#define BSON_TYPES_H

#include "../main.h"

//#include "types/document.h"
#include "types/oid.h"

namespace BSON
{
	namespace Types
	{
		enum Type {
			oid = 1,
			document = 2,
		};

		// Initialization
		void Initialize(ILuaBase* LUA);
	}
}

#endif // BSON_TYPES_H