#include "oid.h"

#include <string>
#include <sstream>
#include <iomanip>

namespace BSON
{
	namespace Types
	{
		namespace ObjectID
		{
			int META;

			std::string StreamBytesToHex(const char* bytes, int size)
			{
				std::stringstream ss;
				ss << std::hex << std::setfill('0');
				for (int i = 0; i < size; i++) {
					ss << std::setw(2) << (unsigned)static_cast<unsigned char>(bytes[i]);
				}
				return ss.str();
			}

			int META__TOSTRING(lua_State* L)
			{
				ILuaBase* LUA = L->luabase;
				LUA->SetState(L);

				LUA->CheckType(1, META);
				ObjectIdStruct* data = LUA->GetUserType<ObjectIdStruct>(1, META);
				if (data == nullptr || data->oid.type() != bsoncxx::type::k_oid) {
					LUA->PushString("BSON ObjectID(\"invalid\")");
					return 1;
				}

				auto oid = data->oid.get_oid().value;

				std::string hex = StreamBytesToHex(oid.bytes(), oid.k_oid_length);
				std::string out = "BSON ObjectID(\"" + hex + "\")";

				LUA->PushString(out.c_str());
				
				return 1;
			}

			int Value(lua_State* L)
			{
				ILuaBase* LUA = L->luabase;
				LUA->SetState(L);

				LUA->CheckType(1, META);
				ObjectIdStruct* data = LUA->GetUserType<ObjectIdStruct>(1, META);
				if (data == nullptr || data->oid.type() != bsoncxx::type::k_oid) {
					LUA->ArgError(1, "Invalid Object ID");
					return 0;
				}

				auto oid = data->oid.get_oid().value;
				std::string hex = StreamBytesToHex(oid.bytes(), oid.k_oid_length);

				LUA->PushString(hex.c_str());

				return 1;
			}

			void New(ILuaBase* LUA, bsoncxx::types::bson_value::view oid)
			{
				ObjectIdStruct data{ oid };
				LUA->PushUserType_Value(data, META);
			}

			void Initialize(ILuaBase* LUA)
			{
				META = LUA->CreateMetaTable("BSON Document");
					LUA->Push(-1); LUA->SetField(-2, "__index");
					LUA->PushCFunction(META__TOSTRING); LUA->SetField(-2, "__tostring");

					LUA->PushCFunction(Value); LUA->SetField(-2, "Value");
					LUA->PushNumber(Type::oid); LUA->SetField(-2, "Type");
				LUA->Pop();
			}
		}
	}
}