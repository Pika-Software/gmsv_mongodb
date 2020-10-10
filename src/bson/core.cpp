#include "core.h"

#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

namespace BSON 
{
	namespace Core
	{
		// Parse a BSON Document to Lua table
		void ParseBSON(ILuaBase* LUA, bsoncxx::document::view view)
		{
			using bsoncxx::type;

			LUA->CreateTable();
			for (auto&& el : view) {
				LUA->PushString(el.key().data());

				switch (el.type())
				{
					// Default types
				case type::k_bool: // Bool
					LUA->PushBool(el.get_bool().value);
					break;
				case type::k_utf8: // String
					LUA->PushString(el.get_utf8().value.data());
					break;
				case type::k_double: // Number (float)
					LUA->PushNumber(el.get_double().value);
					break;
				case type::k_int32: // Number
					LUA->PushNumber(el.get_int32().value);
					break;
				case type::k_int64: // Number
					LUA->PushNumber(el.get_int64().value);
					break;
					// Tables
				case type::k_document:
					ParseBSON(LUA, el.get_document().value);
					break;
				case type::k_array:
					ParseBSON(LUA, el.get_array().value);
					break;
					// Unknown type or null
				default:
					LUA->PushNil();
					break;
				}

				LUA->SetTable(-3);
			}
		}

		// Parse a BSON Array to Lua table
		void ParseBSON(ILuaBase* LUA, bsoncxx::array::view view)
		{
			using bsoncxx::type;

			int k = 1;
			LUA->CreateTable();
			for (auto&& el : view) {
				LUA->PushNumber(k++);

				switch (el.type())
				{
					// Default types
				case type::k_bool: // Bool
					LUA->PushBool(el.get_bool().value);
					break;
				case type::k_utf8: // String
					LUA->PushString(el.get_utf8().value.data());
					break;
				case type::k_double: // Number (float)
					LUA->PushNumber(el.get_double().value);
					break;
				case type::k_int32: // Number
					LUA->PushNumber(el.get_int32().value);
					break;
				case type::k_int64: // Number
					LUA->PushNumber(el.get_int64().value);
					break;
					// Tables
				case type::k_document:
					ParseBSON(LUA, el.get_document().value);
					break;
				case type::k_array:
					ParseBSON(LUA, el.get_array().value);
					break;
					// Unknown type or null
				default:
					LUA->PushNil();
					break;
				}

				LUA->SetTable(-3);
			}
		}

		// Iterate table at iStackPos.
		// True if every table keys is number.
		bool TableIsArray(ILuaBase* LUA, int iStackPos)
		{
			LUA->Push(iStackPos);
			LUA->PushNil();
			bool isArray = true;

			while (LUA->Next(-2) != 0) {
				if (LUA->GetType(-2) != Type::Number)
					isArray = false;

				LUA->Pop();
			}

			LUA->Pop();
			return isArray;
		}

		// Checks, if lua number is integer.
		bool isInt(double val)
		{
			return (int)val == val;
		}

		// Iterating lua table as document at iStackPos.
		void IterateTable(ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::document &builder)
		{
			using namespace bsoncxx::types;
			using bsoncxx::builder::stream::document;
			using bsoncxx::builder::stream::array;
			using bsoncxx::builder::stream::finalize;
			using bsoncxx::builder::concatenate;

			LUA->Push(iStackPos);
			LUA->PushNil();

			while (LUA->Next(-2) != 0) {
				std::string key_str;
				if (LUA->GetType(-2) == Type::Number) {
					double num = LUA->GetNumber(-2);
					if (isInt(num)) key_str = std::to_string((int)num);
					else			key_str = std::to_string(num);
				}
				else {
					key_str = LUA->GetString(-2);
				}

				auto key = builder << key_str;

				switch (LUA->GetType(-1))
				{
				case Type::Bool: // Boolean value
					key << LUA->GetBool(-1);
					break;
				case Type::String: // String value
					key << LUA->GetString(-1);
					break;
				case Type::Number: // Double or integer value
				{
					double num = LUA->GetNumber(-1);
					key << (isInt(num) ? (int)num : num);
					break;
				}
				case Type::Table: // Document or Array
				{
					bool isArray = TableIsArray(LUA, -1);
					if (isArray) { // Array
						array sub_arr{};
						IterateTable(LUA, -1, sub_arr);
						auto doc = sub_arr << finalize;
						key << concatenate(doc.view());
					}
					else { // Document
						document sub_doc{};
						IterateTable(LUA, -1, sub_doc);
						auto arr = sub_doc << finalize;
						key << concatenate(arr.view());
					}

					break;
				}
				default: // Unknown value. Adding "key = null"
					key << b_null{};
					break;
				}

				LUA->Pop();
			}

			LUA->Pop();
		}

		// Iterating lua table as array at iStackPos.
		void IterateTable(ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::array &builder)
		{
			using namespace bsoncxx::types;
			using bsoncxx::builder::stream::document;
			using bsoncxx::builder::stream::array;
			using bsoncxx::builder::stream::finalize;
			using bsoncxx::builder::concatenate;

			LUA->Push(iStackPos);
			LUA->PushNil();

			while (LUA->Next(-2) != 0) {
				switch (LUA->GetType(-1))
				{
				case Type::Bool: // Boolean value
					builder << LUA->GetBool(-1);
					break;
				case Type::String: // String value
					builder << LUA->GetString(-1);
					break;
				case Type::Number: // Double or integer value
				{
					double num = LUA->GetNumber(-1);
					builder << (isInt(num) ? (int)num : num);
					break;
				}
				case Type::Table: // Document or Array
				{
					bool isArray = TableIsArray(LUA, -1);
					if (isArray) { // Array
						array sub_arr{};
						IterateTable(LUA, -1, sub_arr);
						auto doc = sub_arr << finalize;
						builder << concatenate(doc.view());
					}
					else { // Document
						document sub_doc{};
						IterateTable(LUA, -1, sub_doc);
						auto arr = sub_doc << finalize;
						builder << concatenate(arr.view());
					}

					break;
				}
				default: // Unknown value. Adding "key = null"
					builder << b_null{};
					break;
				}

				LUA->Pop();
			}

			LUA->Pop();
		}

		// Parsing lua table at iStackPos
		bsoncxx::document::value ParseTable(ILuaBase* LUA, int iStackPos)
		{
			bsoncxx::builder::stream::document doc{};

			IterateTable(LUA, iStackPos, doc);
			return doc << bsoncxx::builder::stream::finalize;
		}

		// Creating BSON document from JSON
		int FromJSON(lua_State* L)
		{
			ILuaBase* LUA = L->luabase;
			LUA->SetState(L);

			const char* json = LUA->CheckString(1);

			bsoncxx::document::value doc = bsoncxx::from_json(json);

			//Types::Document::DocumentStruct data{ doc };
			//LUA->PushUserType_Value(data, Types::Document::META);
			ParseBSON(LUA, doc.view());

			return 1;
		}

		// Initialization
		void Initialize(ILuaBase* LUA)
		{
			Types::Initialize(LUA);
		}
	}
}
