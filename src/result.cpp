#include "result.h"
#include "bson/core.h"

#include <sstream>

std::string Result::DebugStack::Info::to_string()
{
	return std::string() + (name ? name : "<unknown>") + " - " + short_src + ':' + std::to_string(currentline);
}

Result::DebugStack::DebugStack(Lua::ILuaBase* LUA)
{
	for (int i = 1; i < 6; i++) {
		auto d = get_info(LUA, i);
		if (d)
			stack.push(d.value());
		else
			break;
	}
}

optional<Result::DebugStack::Info> Result::DebugStack::get_info(Lua::ILuaBase* LUA, int stack)
{
	Info d;
	LUA->PushSpecial(Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "debug");
	LUA->GetField(-1, "getinfo");

	LUA->PushNumber(stack);
	LUA->PushString("nSl");
	LUA->Call(2, 1);

	if (LUA->IsType(-1, Lua::Type::Nil)) {
		LUA->Pop(3);
		return optional<Info>();
	}

	LUA->GetField(-1, "name"); d.name = LUA->GetString(-1); LUA->Pop();
	LUA->GetField(-1, "short_src"); d.short_src = LUA->GetString(-1); LUA->Pop();
	LUA->GetField(-1, "currentline"); d.currentline = LUA->GetNumber(-1); LUA->Pop();

	LUA->Pop(3);

	return d;
}

optional<Result::DebugStack::Info> Result::DebugStack::get_info_func(Lua::ILuaBase* LUA, int iRef)
{
	Info d;
	LUA->PushSpecial(Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "debug");
	LUA->GetField(-1, "getinfo");

	LUA->ReferencePush(iRef);
	LUA->PushString("nSl");
	LUA->Call(2, 1);

	if (LUA->IsType(-1, Lua::Type::Nil)) {
		LUA->Pop(3);
		return optional<Info>();
	}

	LUA->GetField(-1, "name"); d.name = LUA->GetString(-1); LUA->Pop();
	LUA->GetField(-1, "short_src"); d.short_src = LUA->GetString(-1); LUA->Pop();
	LUA->GetField(-1, "currentline"); d.currentline = LUA->GetNumber(-1); LUA->Pop();

	LUA->Pop(3);

	return d;
}

std::string Result::DebugStack::BuildStack(Lua::ILuaBase* LUA)
{
	std::stringstream ss;

	int level = 1;
	auto info = get_info(LUA, 1);
	if (info)
		ss << ' ' << std::to_string(level++) << ". " << info.value().to_string() << "\n";

	while (!stack.empty()) {
		auto d = stack.front();
		for (int i = 0; i < level; i++)
			ss << ' ';
		
		ss << std::to_string(level) << ". " << d.to_string() << "\n";
		stack.pop();
	}

	return ss.str();
}

Result::Reference::Reference(Lua::ILuaBase* LUA)
{
	ref = LUA->ReferenceCreate();
}

void Result::Reference::Push(Lua::ILuaBase* LUA)
{
	LUA->ReferencePush(ref);
}

void Result::Reference::Free(Lua::ILuaBase* LUA)
{
	LUA->ReferenceFree(ref);
}

void Result::Value::Push(Lua::ILuaBase* LUA)
{
	switch (i_type)
	{
	case k_bool:
		LUA->PushBool(boolean);
		break;
	case k_number:
		LUA->PushNumber(number);
		break;
	case k_double:
		LUA->PushNumber(dnumber);
	case k_string:
		LUA->PushString(string.c_str());
		break;
	case k_ref:
		reference.Push(LUA);
		break;
	case k_oid:
		BSON::Type::ObjectID::New(LUA, oid);
		break;
	case k_oid_array: {
		int k = 1;
		LUA->CreateTable();
		for (auto&& oid : oid_array) {
			LUA->PushNumber(k++);
			BSON::Type::ObjectID::New(LUA, oid);
			LUA->SetTable(-3);
		}
		break;
	}

	default:
		LUA->PushNil();
		break;
	}
}

void Result::Value::Free(Lua::ILuaBase* LUA)
{
	switch (i_type)
	{
	case k_ref:
		reference.Free(LUA);
		break;
	default:
		break;
	}
}

Result::Value Result::Value::operator=(bool b)
{
	i_type = k_bool;
	boolean = b;
	return *this;
}

Result::Value Result::Value::operator=(int num)
{
	i_type = k_number;
	number = num;
	return *this;
}

Result::Value Result::Value::operator=(double dnum)
{
	i_type = k_double;
	dnumber = dnum;
	return *this;
}

Result::Value Result::Value::operator=(std::string str)
{
	i_type = k_string;
	string = str;
	return *this;
}

Result::Value Result::Value::operator=(Reference ref)
{
	i_type = k_ref;
	reference = ref;
	return *this;
}

Result::Value Result::Value::operator=(bsoncxx::oid objectid)
{
	i_type = k_oid;
	oid = objectid;
	return *this;
}

Result::Value Result::Value::operator=(std::vector<bsoncxx::oid> objectid_array)
{
	i_type = k_oid_array;
	oid_array = objectid_array;
	return *this;
}

Result::Result()
{
	data["status"] = true;
}

void Result::Free(Lua::ILuaBase* LUA)
{
	for (auto&& p : data) {
		p.second.Free(LUA);
	}
	data.clear();
}

void Result::Push(Lua::ILuaBase* LUA)
{
	LUA->CreateTable();
	for (auto&& p : data) {
		p.second.Push(LUA);
		LUA->SetField(-2, p.first);
	}
}

void Result::Error(int code, std::string message)
{
	data["status"] = false;
	data["code"] = code;
	data["message"] = message;
}

void Result::Data(Reference ref)
{
	data["data"] = ref;
}

void Result::Data(Lua::ILuaBase* LUA)
{
	data["data"] = Reference(LUA);
}

void Result::SetResult(const mongocxx::result::insert_one& result)
{
	data["id"] = result.inserted_id().get_oid().value;
}

void Result::SetResult(const mongocxx::result::insert_many& result)
{
	data["inserted_count"] = result.inserted_count();

	auto id_map = result.inserted_ids();
	std::vector<bsoncxx::oid> oid_array;
	for (auto&& p : id_map)
		oid_array.push_back(p.second.get_oid().value);

	data["ids"] = oid_array;
}

void Result::SetResult(const mongocxx::result::update& result) 
{
	data["matched_count"] = result.matched_count();
	data["modified_count"] = result.modified_count();

	auto id = result.upserted_id();
	if (id)
		data["id"] = id.value().get_oid().value;
}

void Result::SetResult(const mongocxx::result::delete_result& result)
{
	data["deleted_count"] = result.deleted_count();
}

void Result::Call(Lua::ILuaBase* LUA, int iFunc)
{
	if (iFunc) {
		LUA->ReferencePush(iFunc);
		Push(LUA); Free(LUA);

		if (LUA->PCall(1, 1, 0) != 0) {
			LUA->PushSpecial(Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "ErrorNoHalt");
			LUA->PushString((std::string("[ERROR] ") + LUA->GetString(-3) + "\n").c_str());
			LUA->Call(1, 0); LUA->Pop();
		}

		LUA->Pop();

		LUA->ReferenceFree(iFunc);
	}
}

void Result::Call(Lua::ILuaBase* LUA, int iFunc, DebugStack& stack)
{
	if (iFunc) {
		LUA->ReferencePush(iFunc);
		Push(LUA); Free(LUA);

		if (LUA->PCall(1, 1, 0) != 0) {
			LUA->PushSpecial(Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "ErrorNoHalt");
			auto err = std::string("[ERROR] ") + LUA->GetString(-3) + "\n" + stack.BuildStack(LUA);
			LUA->PushString(err.c_str());
			LUA->Call(1, 0); LUA->Pop();
		}

		LUA->Pop();
		LUA->ReferenceFree(iFunc);
	}
}

bool Result::KeyValue(Lua::ILuaBase* LUA, const char* key, const char* expectedKey, int valueType, int valuePos)
{
	return strcmp(key, expectedKey) == 0 && LUA->IsType(valuePos, valueType);
}

void Result::ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::insert &options)
{
	LUA->Push(iStackPos);
	LUA->PushNil();
	while (LUA->Next(iStackPos) != 0) {
		if (LUA->IsType(-2, Lua::Type::String)) {
			auto key = LUA->GetString(-2);
			
			if (KeyValue(LUA, key, "bypass_document_validation", Lua::Type::Bool))
				options.bypass_document_validation(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "ordered", Lua::Type::Bool))
				options.ordered(LUA->GetBool(-1));
		}

		LUA->Pop();
	}
	LUA->Pop();
}

void Result::ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::find &options)
{
	LUA->Push(iStackPos);
	LUA->PushNil();
	while (LUA->Next(-2) != 0) {
		if (LUA->IsType(-2, Lua::Type::String)) {
			auto key = LUA->GetString(-2);
			
			if (KeyValue(LUA, key, "allow_disk_use", Lua::Type::Bool))
				options.allow_disk_use(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "allow_partial_results", Lua::Type::Bool))
				options.allow_partial_results(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "batch_size", Lua::Type::Number))
				options.batch_size(LUA->GetNumber(-1));
			else if (KeyValue(LUA, key, "collation", Lua::Type::Table))
				options.collation(BSON::Core::ParseTable(LUA, -1));
			else if (KeyValue(LUA, key, "comment", Lua::Type::String))
				options.comment(LUA->GetString(-1));
			else if (KeyValue(LUA, key, "limit", Lua::Type::Number))
				options.limit(LUA->GetNumber(-1));
			else if (KeyValue(LUA, key, "max", Lua::Type::Table))
				options.max(BSON::Core::ParseTable(LUA, -1));
			else if (KeyValue(LUA, key, "max_await_time", Lua::Type::Number))
				options.max_await_time(std::chrono::milliseconds((int)LUA->GetNumber(-1)));
			else if (KeyValue(LUA, key, "max_time", Lua::Type::Number))
				options.max_time(std::chrono::milliseconds((int)LUA->GetNumber(-1)));
			else if (KeyValue(LUA, key, "min", Lua::Type::Table))
				options.min(BSON::Core::ParseTable(LUA, -1));
			else if (KeyValue(LUA, key, "no_cursor_timeout", Lua::Type::Bool))
				options.no_cursor_timeout(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "projection", Lua::Type::Table))
				options.projection(BSON::Core::ParseTable(LUA, -1));
			else if (KeyValue(LUA, key, "return_key", Lua::Type::Bool))
				options.return_key(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "show_record_id", Lua::Type::Bool))
				options.show_record_id(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "skip ", Lua::Type::Number))
				options.skip(LUA->GetNumber(-1));
			else if (KeyValue(LUA, key, "sort ", Lua::Type::Table))
				options.sort(BSON::Core::ParseTable(LUA, -1));
		}

		LUA->Pop();
	}
	LUA->Pop();
}

void Result::ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::update &options)
{
	LUA->Push(iStackPos);
	LUA->PushNil();
	while (LUA->Next(iStackPos) != 0) {
		if (LUA->IsType(-2, Lua::Type::String)) {
			auto key = LUA->GetString(-2);
			
			if (KeyValue(LUA, key, "bypass_document_validation", Lua::Type::Bool))
				options.bypass_document_validation(LUA->GetBool(-1));
			else if (KeyValue(LUA, key, "collation", Lua::Type::Table))
				options.collation(BSON::Core::ParseTable(LUA, -1));
			else if (KeyValue(LUA, key, "upsert", Lua::Type::Bool))
				options.upsert(LUA->GetBool(-1));
		}

		LUA->Pop();
	}
	LUA->Pop();
}

void Result::ParseOptions(Lua::ILuaBase* LUA, int iStackPos, mongocxx::options::delete_options &options)
{
	LUA->Push(iStackPos);
	LUA->PushNil();
	while (LUA->Next(iStackPos) != 0) {
		if (LUA->IsType(-2, Lua::Type::String)) {
			auto key = LUA->GetString(-2);
			
			if (KeyValue(LUA, key, "collation", Lua::Type::Table))
				options.collation(BSON::Core::ParseTable(LUA, -1));
		}

		LUA->Pop();
	}
	LUA->Pop();
}