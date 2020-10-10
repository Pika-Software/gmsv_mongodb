#include "document.h"

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>

namespace BSON
{
	namespace Types
	{
		namespace Document
		{
			int META;

			int META__TOSTRING(lua_State* L)
			{
				ILuaBase* LUA = L->luabase;
				LUA->SetState(L);

				LUA->PushString("BSON Document");
				return 1;
			}

			int Parse(lua_State* L)
			{
				ILuaBase* LUA = L->luabase;
				LUA->SetState(L);

				LUA->CheckType(1, META);
				DocumentStruct* data = LUA->GetUserType<DocumentStruct>(1, META);
				if (data == nullptr) {
					LUA->ArgError(1, "Invalid document!");
					return 0;
				}

				bsoncxx::document::value doc = data->doc;
				
				try {
					BSON::Core::ParseBSON(LUA, doc);
				} catch(mongocxx::exception err) {
					LUA->ThrowError(err.what());
					return 0;
				}

				return 1;
			}

			int ToJSON(lua_State* L)
			{
				ILuaBase* LUA = L->luabase;
				LUA->SetState(L);

				LUA->CheckType(1, META);
				DocumentStruct* data = LUA->GetUserType<DocumentStruct>(1, META);
				if (data == nullptr) {
					LUA->ArgError(1, "Invalid document!");
					return 0;
				}

				bsoncxx::document::value doc = data->doc;
				std::string json = bsoncxx::to_json(doc);
				LUA->PushString(json.c_str(), json.length());

				return 1;
			}

			void Initialize(ILuaBase* LUA)
			{
				META = LUA->CreateMetaTable("BSON Document");
					LUA->Push(-1); LUA->SetField(-2, "__index");
					LUA->PushCFunction(META__TOSTRING); LUA->SetField(-2, "__tostring");

					LUA->PushCFunction(Parse); LUA->SetField(-2, "Parse");
					LUA->PushCFunction(ToJSON); LUA->SetField(-2, "ToJSON");
					LUA->PushNumber(Type::document); LUA->SetField(-2, "Type");
				LUA->Pop();
			}
		}
	}
}