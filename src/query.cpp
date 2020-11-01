// WARNING! SHITCODE!
#include "query.h"

int Query::META;
std::mutex Query::ListMutex;
std::deque<QueryData*> Query::DataList;
std::deque<std::pair<Query*, QueryData*>> Query::RemoveList;

void Query::Acquire(Lua::ILuaBase* LUA, QueryFunc func) 
{
	QueryData* data = NULL;
	if (!m_sync) {
		data = new QueryData;

		ListMutex.lock();
		DataList.push_back(data);
		ListMutex.unlock();
	}

	auto lambda = [LUA, data, func]() {
		if (data != NULL) {
			Global::DevMsg(1, "[Thread] Waiting main thread for sync...\n");
			std::unique_lock<std::mutex> lck(data->mtx);
			data->cv.wait(lck);
			Global::DevMsg(1, "[Thread] Main thread synced!\n");
		}

		func(LUA);
		LUA->Pop(LUA->Top()); // Clearing stack

		if (data != NULL) {
			Global::DevMsg(1, "[Thread] Desyncing main thread...\n");
			std::lock_guard<std::mutex> guard(data->mtx);
			data->done = true;
			data->cv.notify_one();
			Global::DevMsg(1, "[Thread] Main thread desynced!\n");
		}
	};

	if (data != NULL) {
		Global::DevMsg(1, "Starting thread for synching lua state...\n");
		std::thread(lambda).detach();
	}
	else 
		lambda();
		
}

bool Query::isUnblocked()
{
	return !m_done && !m_sync;
}

void Query::New(Lua::ILuaBase* LUA, QueryThreadFunc func)
{
	auto q = new Query;
	auto lambda = [](Lua::ILuaBase* LUA, Query* q, QueryThreadFunc func) {
		{
			std::unique_lock<std::mutex> lck(q->m_mutex);
			auto status = q->m_cvar.wait_for(lck, std::chrono::milliseconds(500), [q] { return q->m_sync; });
		}

		//{
		//	std::unique_lock<std::mutex> lck(q->m_mutex);
		//	while (!q->m_sync.load()) {
		//		auto status = q->m_cvar.wait_for(lck, std::chrono::milliseconds(10000));
		//	}

		//	if (q->m_sync.load())
		//		Global::DevMsg("[Thread] Thread is synched with main thread!\n");
		//}

		func(LUA, q);
		
		q->m_mutex.lock();
		q->m_done = true;
		q->m_mutex.unlock();

		q->m_cvar.notify_one();
	};

	std::thread(lambda, LUA, q, func).detach();
	LUA->PushUserType(q, META);
}

Query* Query::CheckSelf(Lua::ILuaBase* LUA, int iStackPos)
{
	LUA->CheckType(iStackPos, META);
	return LUA->GetUserType<Query>(iStackPos, META);
}

int Query::__tostring(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	auto obj = CheckSelf(LUA);
	std::string out = "Query: " + Global::PtrToStr(obj);

	LUA->PushString(out.c_str());
	return 1;
}

int Query::Wait(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);
	auto obj = CheckSelf(LUA);
	if (obj && obj->isUnblocked()) {
		Global::DevMsg(1, "Trying sync thread...\n");
		{
			std::lock_guard<std::mutex> guard(obj->m_mutex);
			obj->m_sync = true;
		}

		obj->m_cvar.notify_all();
		Global::DevMsg(1, "Thread synched!\n");

		{
			std::unique_lock<std::mutex> lck(obj->m_mutex);
			obj->m_cvar.wait(lck, [obj] { return obj->m_done; });
		}
	}

	return 0;
}

int Query::Think(lua_State* L)
{
	Lua::ILuaBase* LUA = L->luabase;
	LUA->SetState(L);

	std::lock_guard<std::mutex> guard(ListMutex);
	while (!RemoveList.empty()) {
		auto p = RemoveList.front();

		if (p.first)
			delete p.first;
		if (p.second)
			delete p.second;

		RemoveList.pop_front();
	}

	while (!DataList.empty()) {
		auto data = DataList.front();

		Global::DevMsg(1, "[Main Thread] Syncing thread...\n");
		std::unique_lock<std::mutex> lck(data->mtx);
		data->cv.notify_one();
		Global::DevMsg(1, "[Main Thread] Thread synced! Waiting for end...\n");
		while (!data->isDone()) data->cv.wait(lck);
		Global::DevMsg(1, "[Main Thread] Thread ended!\n");

		RemoveList.push_back(std::make_pair((Query*)NULL, data));
		DataList.pop_front();
	}

	return 0;
}

void Query::Initialize(Lua::ILuaBase* LUA)
{
	META = LUA->CreateMetaTable("Query");
		LUA->Push(-1); LUA->SetField(-2, "__index");
		LUA->PushCFunction(__tostring); LUA->SetField(-2, "__tostring");
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
}