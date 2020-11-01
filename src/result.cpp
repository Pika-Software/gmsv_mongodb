#include "result.h"
#include "bson/core.h"

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
			Global::Run(LUA, "print", 1, 0);
			LUA->PushSpecial(Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "debug");
			LUA->GetField(-1, "Trace");

			if (LUA->IsType(-1, Lua::Type::Function))
				LUA->Call(0, 0);
			else
				LUA->Pop();

			LUA->Pop(2);
		}
		else
			LUA->Pop();

		LUA->ReferenceFree(iFunc);
	}
}