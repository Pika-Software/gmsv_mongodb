#include "promise.h"
#include <thread>
#include <system_error>

int META_PROMISE;

Promise* Promise::CheckSelf(Lua::ILuaBase* LUA, int iStackPos)
{
	LUA->CheckType(iStackPos, META_PROMISE);
	return LUA->GetUserType<Promise>(iStackPos, META_PROMISE);
}

int Promise::__gc(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto obj = CheckSelf(LUA);
	if (obj)
		delete obj;

	return 0;
}

int Promise::__tostring(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto obj = CheckSelf(LUA);
	std::string status;
	if (obj)
		status = obj->m_done ? "done" : "pending";
	else
		status = "invalid";

	status = "Promise [" + status + "]: " + Global::PtrToStr(obj);

	LUA->PushString(status.c_str());
	return 1;
}

template<class Fn>
void Promise::New(Lua::ILuaBase* LUA, Fn func)
{
	//auto lambda = [](Promise* obj, Lua::ILuaBase* LUA, std::function<T(T, U)> func) {
	//	func(LUA);
	//	std::unique_lock<std::mutex> lk(obj->m_mtx);
	//	obj->m_done = true;
	//	obj->m_cv.notify_one();
	//	delete obj;
	//};

	auto obj = new Promise;
	func();
	//std::thread(lambda, obj, LUA, func).detach();
	LUA->PushUserType(obj, META_PROMISE);
}

int Promise::Wait(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto obj = CheckSelf(LUA);
	if (obj && !obj->m_done) {
		std::unique_lock<std::mutex> lk(obj->m_mtx);
		while (!obj->m_done) obj->m_cv.wait(lk);
	}

	return 0;
}

int Promise::IsValid(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);
	
	auto obj = CheckSelf(LUA);
	LUA->PushBool(obj && !obj->m_done);
	return 1;
}

void Promise::Initialize(Lua::ILuaBase* LUA)
{
	META_PROMISE = LUA->CreateMetaTable("Promise");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		//LUA->PushCFunction(__gc); LUA->SetField(-2, "__gc");
		LUA->PushCFunction(__tostring); LUA->SetField(-2, "__tostring");

		LUA->PushCFunction(Wait); LUA->SetField(-2, "Wait");
		LUA->PushCFunction(IsValid); LUA->SetField(-2, "IsValid");
	LUA->Pop();
}