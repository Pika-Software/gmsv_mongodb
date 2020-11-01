#include "core.h"

#include <bsoncxx/oid.hpp>
//#include <bsoncxx/json.hpp>
//#include <bsoncxx/types.hpp>
//#include <bsoncxx/builder/stream/array.hpp>
//#include <bsoncxx/builder/stream/document.hpp>
//#include <bsoncxx/builder/stream/helpers.hpp>

namespace BSON {
	void Core::ParseBSON(Lua::ILuaBase* LUA, bsoncxx::document::view view)
	{
		LUA->CreateTable();
		for (auto&& el : view) {
			LUA->PushString(el.key().data());
			ParseBSONValue(LUA, el.get_value());
			LUA->SetTable(-3);
		}
	}

	void Core::ParseBSON(Lua::ILuaBase* LUA, bsoncxx::array::view view)
	{
		int k = 1;
		LUA->CreateTable();
		for (auto&& el : view) {
			LUA->PushNumber(k++);
			ParseBSONValue(LUA, el.get_value());
			LUA->SetTable(-3);
		}
	}

	void Core::ParseBSONValue(Lua::ILuaBase* LUA, bsoncxx::types::bson_value::view view)
	{
		using bsoncxx::type;
		
		switch (view.type())
		{
			// Default types
		case type::k_bool: // Bool
			LUA->PushBool(view.get_bool().value);
			break;
		case type::k_utf8: // String
			LUA->PushString(view.get_utf8().value.data());
			break;
		case type::k_double: // Number (float)
			LUA->PushNumber(view.get_double().value);
			break;
		case type::k_int32: // Number
			LUA->PushNumber(view.get_int32().value);
			break;
		case type::k_int64: // Number
			LUA->PushNumber(view.get_int64().value);
			break;
			// Tables
		case type::k_document:
			ParseBSON(LUA, view.get_document().value);
			break;
		case type::k_array:
			ParseBSON(LUA, view.get_array().value);
			break;
			// Other types
		case type::k_oid: // ObjectID
			Type::ObjectID::New(LUA, view);
			break;
			// Unknown type or null
		default:
			LUA->PushNil();
			break;
		}
	}

	bool Core::TableIsArray(Lua::ILuaBase* LUA, int iStackPos)
	{
		LUA->Push(iStackPos);
		LUA->PushNil();

		bool isArray = true;
		while (LUA->Next(-2) != 0) {
			if (LUA->GetType(-2) != Lua::Type::Number)
				isArray = false;
		
			LUA->Pop();
		}
		
		LUA->Pop();
		return isArray;
	}

	bool Core::isInt(double val)
	{
		return (int)val == val;
	}

	// Iterating lua table as document at iStackPos.
	void Core::IterateTable(Lua::ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::document &builder)
	{
		using namespace bsoncxx::types;
		using bsoncxx::builder::stream::document;
		using bsoncxx::builder::stream::array;
		using bsoncxx::builder::stream::finalize;
		using bsoncxx::builder::concatenate;

		LUA->Push(iStackPos); // Table
		LUA->PushNil(); // Key

		while (LUA->Next(-2) != 0) {
			std::string key_str;
			if (LUA->IsType(-2, Lua::Type::Number)) {
				double num = LUA->GetNumber(-2);
				if (isInt(num)) key_str = std::to_string((int)num);
				else			key_str = std::to_string(num);
			}
			else
				key_str = LUA->GetString(-2);

			auto key = builder << key_str;

			switch (LUA->GetType(-1))
			{
			case Lua::Type::Bool: // Boolean value
				key << LUA->GetBool(-1);
				break;
			case Lua::Type::String: // String value
				key << LUA->GetString(-1);
				break;
			case Lua::Type::Number: // Double or integer value
			{
				double num = LUA->GetNumber(-1);
				key << (isInt(num) ? (int)num : num);
				break;
			}
			case Lua::Type::Table: // Document or Array
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
			default: // Unknown value.
				if (LUA->IsType(-1, BSON::Type::META_OBJECTID)) { // ObjectID.
					auto obj = BSON::Type::ObjectID::CheckSelf(LUA, -1);
					if (obj)
						key << obj->oid;
					else
						key << b_null{};
				} else
					key << b_null{};

				break;
			}

			LUA->Pop();
		}

		LUA->Pop();
	}

	// Iterating lua table as array at iStackPos.
	void Core::IterateTable(Lua::ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::array &builder)
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
			case Lua::Type::Bool: // Boolean value
				builder << LUA->GetBool(-1);
				break;
			case Lua::Type::String: // String value
				builder << LUA->GetString(-1);
				break;
			case Lua::Type::Number: // Double or integer value
			{
				double num = LUA->GetNumber(-1);
				builder << (isInt(num) ? (int)num : num);
				break;
			}
			case Lua::Type::Table: // Document or Array
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
	bsoncxx::document::value Core::ParseTable(Lua::ILuaBase* LUA, int iStackPos)
	{
		bsoncxx::builder::stream::document doc{};

		IterateTable(LUA, iStackPos, doc);
		return doc << bsoncxx::builder::stream::finalize;
	}

	void Core::Initialize(Lua::ILuaBase* LUA)
	{
		Types::Initialize(LUA);
	}
}

//namespace BSON 
//{
//	namespace Core
//	{
//		// Parse a BSON Document to Lua table
//		void ParseBSON(Lua::ILuaBase* LUA, bsoncxx::document::view view)
//		{
//			using bsoncxx::type;
//
//			LUA->CreateTable();
//			for (auto&& el : view) {
//				LUA->PushString(el.key().data());
//				ParseBSONValue(LUA, el.get_value());
//				LUA->SetTable(-3);
//			}
//		}
//
//		// Parse a BSON Array to Lua table
//		void ParseBSON(Lua::ILuaBase* LUA, bsoncxx::array::view view)
//		{
//			int k = 1;
//			LUA->CreateTable();
//			for (auto&& el : view) {
//				LUA->PushNumber(k++);
//				ParseBSONValue(LUA, el.get_value());
//				LUA->SetTable(-3);
//			}
//		}
//
//		// Parse BSON value
//		void ParseBSONValue(Lua::ILuaBase* LUA, bsoncxx::types::bson_value::view view)
//		{
//			using bsoncxx::type;
//
//			switch (view.type())
//			{
//				// Default types
//			case type::k_bool: // Bool
//				LUA->PushBool(view.get_bool().value);
//				break;
//			case type::k_utf8: // String
//				LUA->PushString(view.get_utf8().value.data());
//				break;
//			case type::k_double: // Number (float)
//				LUA->PushNumber(view.get_double().value);
//				break;
//			case type::k_int32: // Number
//				LUA->PushNumber(view.get_int32().value);
//				break;
//			case type::k_int64: // Number
//				LUA->PushNumber(view.get_int64().value);
//				break;
//				// Tables
//			case type::k_document:
//				ParseBSON(LUA, view.get_document().value);
//				break;
//			case type::k_array:
//				ParseBSON(LUA, view.get_array().value);
//				break;
//				// Other types
//			case type::k_oid: // ObjectID
//				Types::ObjectID::New(LUA, view);
//				break;
//				// Unknown type or null
//			default:
//				LUA->PushNil();
//				break;
//			}
//		}
//
//		// Iterate table at iStackPos.
//		// True if every table keys is number.
//		bool TableIsArray(Lua::ILuaBase* LUA, int iStackPos)
//		{
//			LUA->Push(iStackPos);
//			LUA->PushNil();
//			bool isArray = true;
//
//			while (LUA->Next(-2) != 0) {
//				if (LUA->GetType(-2) != Lua::Type::Number)
//					isArray = false;
//
//				LUA->Pop();
//			}
//
//			LUA->Pop();
//			return isArray;
//		}
//
//		// Checks, if lua number is integer.
//		bool isInt(double val)
//		{
//			return (int)val == val;
//		}
//
//		// Iterating lua table as document at iStackPos.
//		void IterateTable(Lua::ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::document &builder)
//		{
//			using namespace bsoncxx::types;
//			using bsoncxx::builder::stream::document;
//			using bsoncxx::builder::stream::array;
//			using bsoncxx::builder::stream::finalize;
//			using bsoncxx::builder::concatenate;
//
//			LUA->Push(iStackPos);
//			LUA->PushNil();
//
//			while (LUA->Next(-2) != 0) {
//				std::string key_str;
//				if (LUA->GetType(-2) == Lua::Type::Number) {
//					double num = LUA->GetNumber(-2);
//					if (isInt(num)) key_str = std::to_string((int)num);
//					else			key_str = std::to_string(num);
//				}
//				else {
//					key_str = LUA->GetString(-2);
//				}
//
//				auto key = builder << key_str;
//
//				switch (LUA->GetType(-1))
//				{
//				case Lua::Type::Bool: // Boolean value
//					key << LUA->GetBool(-1);
//					break;
//				case Lua::Type::String: // String value
//					key << LUA->GetString(-1);
//					break;
//				case Lua::Type::Number: // Double or integer value
//				{
//					double num = LUA->GetNumber(-1);
//					key << (isInt(num) ? (int)num : num);
//					break;
//				}
//				case Lua::Type::Table: // Document or Array
//				{
//					bool isArray = TableIsArray(LUA, -1);
//					if (isArray) { // Array
//						array sub_arr{};
//						IterateTable(LUA, -1, sub_arr);
//						auto doc = sub_arr << finalize;
//						key << concatenate(doc.view());
//					}
//					else { // Document
//						document sub_doc{};
//						IterateTable(LUA, -1, sub_doc);
//						auto arr = sub_doc << finalize;
//						key << concatenate(arr.view());
//					}
//
//					break;
//				}
//				default: // Unknown value. Adding "key = null"
//					key << b_null{};
//					break;
//				}
//
//				LUA->Pop();
//			}
//
//			LUA->Pop();
//		}
//
//		// Iterating lua table as array at iStackPos.
//		void IterateTable(Lua::ILuaBase* LUA, int iStackPos, bsoncxx::builder::stream::array &builder)
//		{
//			using namespace bsoncxx::types;
//			using bsoncxx::builder::stream::document;
//			using bsoncxx::builder::stream::array;
//			using bsoncxx::builder::stream::finalize;
//			using bsoncxx::builder::concatenate;
//
//			LUA->Push(iStackPos);
//			LUA->PushNil();
//
//			while (LUA->Next(-2) != 0) {
//				switch (LUA->GetType(-1))
//				{
//				case Lua::Type::Bool: // Boolean value
//					builder << LUA->GetBool(-1);
//					break;
//				case Lua::Type::String: // String value
//					builder << LUA->GetString(-1);
//					break;
//				case Lua::Type::Number: // Double or integer value
//				{
//					double num = LUA->GetNumber(-1);
//					builder << (isInt(num) ? (int)num : num);
//					break;
//				}
//				case Lua::Type::Table: // Document or Array
//				{
//					bool isArray = TableIsArray(LUA, -1);
//					if (isArray) { // Array
//						array sub_arr{};
//						IterateTable(LUA, -1, sub_arr);
//						auto doc = sub_arr << finalize;
//						builder << concatenate(doc.view());
//					}
//					else { // Document
//						document sub_doc{};
//						IterateTable(LUA, -1, sub_doc);
//						auto arr = sub_doc << finalize;
//						builder << concatenate(arr.view());
//					}
//
//					break;
//				}
//				default: // Unknown value. Adding "key = null"
//					builder << b_null{};
//					break;
//				}
//
//				LUA->Pop();
//			}
//
//			LUA->Pop();
//		}
//
//		// Parsing lua table at iStackPos
//		bsoncxx::document::value ParseTable(Lua::ILuaBase* LUA, int iStackPos)
//		{
//			bsoncxx::builder::stream::document doc{};
//
//			IterateTable(LUA, iStackPos, doc);
//			return doc << bsoncxx::builder::stream::finalize;
//		}
//
//		// Initialization
//		void Initialize(Lua::ILuaBase* LUA)
//		{
//			Types::Initialize(LUA);
//		}
//	}
//}
