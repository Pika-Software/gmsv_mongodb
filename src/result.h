#ifndef RESULT_H
#define RESULT_H

#include "main.h"
#include <string>
#include <map>
#include <queue>
#include <mongocxx/result/insert_one.hpp>
#include <mongocxx/result/insert_many.hpp>
#include <mongocxx/result/update.hpp>
#include <mongocxx/result/delete.hpp>
#include <mongocxx/options/insert.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/update.hpp>
#include <mongocxx/options/delete.hpp>

class Result {
public:
	class DebugStack {
	public:
		struct Info {
			const char* name;
			const char* short_src;
			int currentline;
			std::string to_string();
		};

		std::queue<Info> stack;

		DebugStack(Lua::ILuaBase* LUA);
		optional<Info> get_info(Lua::ILuaBase* LUA, int stack);
		static optional<Info> get_info_func(Lua::ILuaBase* LUA, int iRef);
		std::string BuildStack(Lua::ILuaBase* LUA);
	};

	enum Types {
		k_bool = 0,
		k_number = 1,
		k_double = 2,
		k_string = 3,
		k_ref = 4,
		k_oid = 5,
		k_oid_array = 6,
	};

	class Reference {
		int ref;
	public:
		Reference() {}; // Need default constructor for `Value` struct
		Reference(Lua::ILuaBase* LUA);
		void Push(Lua::ILuaBase* LUA);
		void Free(Lua::ILuaBase* LUA);
	};

	struct Value {
		int i_type;
		bool boolean;
		int number;
		double dnumber;
		std::string string;
		Reference reference;

		bsoncxx::oid oid;
		std::vector<bsoncxx::oid> oid_array;
		
		void Push(Lua::ILuaBase* LUA);
		void Free(Lua::ILuaBase* LUA);
		Value operator=(bool b);
		Value operator=(int num);
		Value operator=(double dnum);
		Value operator=(std::string str);
		Value operator=(Reference ref);
		Value operator=(bsoncxx::oid objectid);
		Value operator=(std::vector<bsoncxx::oid> objectid_array);
	};

	std::map<const char*, Value> data;
	Result();

	void Free(Lua::ILuaBase* LUA);
	void Push(Lua::ILuaBase* LUA);
	void Error(int code, std::string message);
	void Data(Reference ref);
	void Data(Lua::ILuaBase* LUA);
	void SetResult(const mongocxx::result::insert_one& result);
	void SetResult(const mongocxx::result::insert_many& result);
	void SetResult(const mongocxx::result::update& result);
	void SetResult(const mongocxx::result::delete_result& result);
	void Call(Lua::ILuaBase* LUA, int iFunc);
	void Call(Lua::ILuaBase* LUA, int iFunc, DebugStack& stack);

	static bool KeyValue(Lua::ILuaBase* LUA, const char* key, const char* expectedKey, int valueType, int valuePos = -1);
	static void ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::insert &options);
	static void ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::find &options);
	static void ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::update &options);
	static void ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::delete_options &options);
};

#endif // RESULT_H