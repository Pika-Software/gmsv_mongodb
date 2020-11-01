#include "types.h"

namespace BSON
{
	void Types::Initialize(Lua::ILuaBase* LUA)
	{
		Type::ObjectID::Initialize(LUA);
	}
}
