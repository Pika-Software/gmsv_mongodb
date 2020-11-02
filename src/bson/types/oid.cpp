#include "oid.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <system_error>

namespace BSON
{
	namespace Type
	{
		int META_OBJECTID;

		ObjectID* ObjectID::CheckSelf(Lua::ILuaBase* LUA, int iStackPos)
		{
			LUA->CheckType(iStackPos, META_OBJECTID);
			return LUA->GetUserType<ObjectID>(iStackPos, META_OBJECTID);
		}

		std::string ObjectID::BytesToHex(const char* bytes, int size)
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			for (int i = 0; i < size; i++) {
				ss << std::setw(2) << (unsigned)static_cast<unsigned char>(bytes[i]);
			}
			return ss.str();
		}

		int ObjectID::__gc(lua_State* L)
		{
			Lua::ILuaBase* LUA = L->luabase;
			LUA->SetState(L);

			auto obj = CheckSelf(LUA);
			if (obj)
				delete obj;

			return 0;
		}

		int ObjectID::__tostring(lua_State* L)
		{
			Lua::ILuaBase* LUA = L->luabase;
			LUA->SetState(L);

			auto obj = CheckSelf(LUA);
			std::string hex;
			if (obj != nullptr)
				hex = BytesToHex(obj->oid.bytes(), obj->oid.k_oid_length);
			else
				hex = "invalid";

			std::string out = "ObjectID(\"" + hex + "\")";
			LUA->PushString(out.c_str());

			return 1;
		}

		int ObjectID::Value(lua_State* L)
		{
			Lua::ILuaBase* LUA = L->luabase;
			LUA->SetState(L);

			auto obj = CheckSelf(LUA);
			if (obj == nullptr) {
				LUA->ArgError(1, "Invalid ObjectID");
				return 0;
			}

			std::string hex = BytesToHex(obj->oid.bytes(), obj->oid.k_oid_length);

			LUA->PushString(hex.c_str());
			return 1;
		}

		int ObjectID::IsValid(lua_State* L)
		{
			Lua::ILuaBase* LUA = L->luabase;
			LUA->SetState(L);

			LUA->PushBool(CheckSelf(LUA) != nullptr);
			return 1;
		}

		int ObjectID::New(lua_State* L)
		{
			Lua::ILuaBase* LUA = L->luabase;
			LUA->SetState(L);

			auto id = LUA->CheckString(1);

			try {
				auto oid = bsoncxx::oid(id);
				New(LUA, oid);
			}
			catch (std::system_error err) {
				LUA->ThrowError(err.what());
				return 0;
			}

			return 1;
		}

		void ObjectID::New(Lua::ILuaBase* LUA, bsoncxx::types::bson_value::view oid)
		{
			New(LUA, oid.get_oid().value);
		}

		void ObjectID::New(Lua::ILuaBase* LUA, bsoncxx::oid oid)
		{
			auto obj = new ObjectID;
			obj->oid = oid;

			LUA->PushUserType(obj, META_OBJECTID);
		}

		void ObjectID::Initialize(Lua::ILuaBase* LUA)
		{
			META_OBJECTID = LUA->CreateMetaTable("ObjectID");
				LUA->Push(-1); LUA->SetField(-2, "__index");
				LUA->PushCFunction(__gc); LUA->SetField(-2, "__gc");
				LUA->PushCFunction(__tostring); LUA->SetField(-2, "__tostring");

				LUA->PushCFunction(Value); LUA->SetField(-2, "Value");
				LUA->PushCFunction(IsValid); LUA->SetField(-2, "IsValid");
			LUA->Pop();
		}
	}
}