// WARNING! SHITCODE!
#include "query.h"

#include <thread>
#include <chrono>

int Query::__gc(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	LUA->CheckType(1, META);
	auto ptr = LUA->GetUserType<SmartPointer<Query>>(1, META);
	if (ptr)
		ptr->free();

	return 0;
}

 void Query::Acquire(std::function<void(Lua::ILuaBase*)> func)
{
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		m_cvar.wait_for(lck, std::chrono::milliseconds(500), [&] { return m_sync; });
	}

	if (m_sync) {
		func(lua_object);
	}
	else {
		std::lock_guard<std::mutex> guard(LuaMutex);
		LuaQueue.push(func);
	}
}

int Query::META;

std::mutex Query::LuaMutex;
std::queue<std::function<void(Lua::ILuaBase*)>> Query::LuaQueue;

std::mutex Query::ThreadMutex;
std::condition_variable Query::ThreadCVar;
bool Query::ThreadStopped;
std::queue<SmartPointer<Query>*> Query::ThreadQueue;

void Query::New(Lua::ILuaBase* LUA, std::function<void(Query*)> func)
{
	auto ptr = new SmartPointer<Query>;
	auto q = ptr->guard(); ptr->add();
	q->m_func = func;
	q->lua_object = LUA;

	std::thread([ptr] {
		std::lock_guard<std::mutex> guard(ThreadMutex);
		ThreadQueue.push(ptr);
		ThreadCVar.notify_one();
	}).detach();

	LUA->PushUserType(ptr, META);
}

int Query::Wait(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	LUA->CheckType(1, META);
	auto ptr = LUA->GetUserType<SmartPointer<Query>>(1, META);
	if (ptr) {
		auto obj = ptr->guard();
		if (obj->m_done)
			return 0;

		std::unique_lock<std::mutex> lck(obj->m_mtx);
		obj->m_sync = true;
		obj->m_cvar.notify_one();
		obj->m_cvar.wait(lck, [&obj] {
			return obj->m_done;
		});
	}

	return 0;
}

void Query::ThreadFunc()
{
	DevMsg(1, "Query thread started!");
	do {
		std::unique_lock<std::mutex> lck{ThreadMutex};
		ThreadCVar.wait(lck, [&] {
			return ThreadStopped || !ThreadQueue.empty();
		});

		while (!ThreadQueue.empty()) {
			auto ptr = ThreadQueue.front();
			ThreadQueue.pop();

			if (!ptr)
				continue;

			auto q = ptr->guard(false);
			if (!q->lua_object)
				continue;

			q->m_func(ptr->get());

			std::lock_guard<std::mutex> guard(q->m_mtx);
			q->m_done = true;
			q->m_cvar.notify_one();
		}

		if (ThreadStopped) {
			DevMsg(1, "Query thread stopped!");
			break;
		}
	} while(true);
}

int Query::Think(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	std::lock_guard<std::mutex> guard(LuaMutex);
	while (!LuaQueue.empty()) {
		LuaQueue.front()(LUA);
		LuaQueue.pop();
	}

	return 0;
} 

void Query::Initialize(Lua::ILuaBase* LUA)
{
	META = LUA->CreateMetaTable("Query");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		LUA->PushCFunction(__gc); LUA->SetField(-2, "__gc");
		//LUA->PushCFunction(__tostring); LUA->SetField(-2, "__tostring");
		LUA->PushCFunction(Wait); LUA->SetField(-2, "Wait");
	LUA->Pop();

	LUA->PushSpecial(Lua::SPECIAL_GLOB);
		LUA->GetField(-1, "timer");
			LUA->GetField(-1, "Create");

			LUA->PushString("__GMSV_MONGODB_THINK"); // identifier
			LUA->PushNumber(1 / 100); // delay
			LUA->PushNumber(0); // repetitions
			LUA->PushCFunction(Think); // func

			LUA->Call(4, 0); // timer.Create(identifier, delay, repetitions, func)
		LUA->Pop();
	LUA->Pop();

	std::thread(&Query::ThreadFunc).detach();
}

void Query::Deinitialize()
{
	std::lock_guard<std::mutex> guard(ThreadMutex);
	ThreadStopped = true;
	ThreadCVar.notify_one();
}