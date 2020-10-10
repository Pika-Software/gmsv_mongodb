#include "types.h"

namespace BSON
{
	namespace Types
	{
		// Initialization
		void Initialize(ILuaBase* LUA)
		{
			//Document::Initialize(LUA);
			ObjectID::Initialize(LUA);
		}
	}
}
