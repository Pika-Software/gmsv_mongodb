// WARNING! SHITCODE!

#ifndef QUERY_H
#define QUERY_H

#include "main.h"
#include "pointer.h"

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

class Query {
	std::function<void(Query*)> m_func;
	std::mutex m_mtx;
	std::condition_variable m_cvar;
	bool m_sync = false;
	bool m_done = false;
	Lua::ILuaBase* lua_object; 
public:
	static int __gc(lua_State* L);
	void Acquire(std::function<void(Lua::ILuaBase*)> func);

	static int META;

	static std::mutex LuaMutex;
	static std::queue<std::function<void(Lua::ILuaBase*)>> LuaQueue;

	static std::mutex ThreadMutex;
	static std::condition_variable ThreadCVar;
	static bool ThreadStopped;
	static std::queue<SmartPointer<Query>*> ThreadQueue;

	static void New(Lua::ILuaBase* LUA, std::function<void(Query*)> func);
	static int Wait(lua_State* L);

	static void ThreadFunc();
	static int Think(lua_State* L);

	static void Initialize(Lua::ILuaBase* LUA);
	static void Deinitialize();
};

// #include <condition_variable>
// #include <atomic>
// #include <thread>
// #include <mutex>
// #include <queue>

// class Query;

// typedef std::function<void(Lua::ILuaBase* LUA, Query* q)> QueryThreadFunc;
// typedef std::function<void(Lua::ILuaBase* LUA)> QueryFunc;

// struct QueryData {
// 	bool done = false;
// 	std::mutex mtx;
// 	std::condition_variable cv;
// 	bool isDone() { return done; };
// };

// class Query {
// public:
// 	static int META;

// 	static std::mutex ListMutex;
// 	static std::queue<QueryData*> DataList;
// 	static std::queue<std::pair<Query*, QueryData*>> RemoveList;

// 	std::mutex m_mutex;
// 	std::condition_variable m_cvar;
// 	bool m_done = false;
// 	bool m_sync = false;

// 	template<typename Fn>
// 	void Acquire(Lua::ILuaBase* LUA, Fn func);
// 	bool isUnblocked();

// 	template<typename Fn>
// 	static void New(Lua::ILuaBase* LUA, Fn func);
// 	//static void New(Lua::ILuaBase* LUA, QueryThreadFunc func);
// 	static Query* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
// 	static int __tostring(lua_State* L);
// 	static int Wait(lua_State* L);
// 	static int Think(lua_State* L);
// 	static void Initialize(Lua::ILuaBase* LUA);
// };

// template<typename Fn>
// inline void Query::Acquire(Lua::ILuaBase* LUA, Fn func)
// {
// 		QueryData* data = NULL;
// 	if (!m_sync) {
// 		data = new QueryData;

// 		ListMutex.lock();
// 		DataList.push(data);
// 		ListMutex.unlock();
// 	}

// 	auto lambda = [LUA, data, func]() mutable {
// 		if (data != NULL) {
// 			DevMsg(1, "[Thread] Waiting main thread for sync...\n");
// 			std::unique_lock<std::mutex> lck(data->mtx);
// 			data->cv.wait(lck);
// 			DevMsg(1, "[Thread] Main thread synced!\n");
// 		}

// 		func(LUA);
// 		LUA->Pop(LUA->Top()); // Clearing stack

// 		if (data != NULL) {
// 			DevMsg(1, "[Thread] Desyncing main thread...\n");
// 			std::lock_guard<std::mutex> guard(data->mtx);
// 			data->done = true;
// 			data->cv.notify_all();
// 			DevMsg(1, "[Thread] Main thread desynced!\n");
// 		}
// 	};

// 	if (data != NULL) {
// 		DevMsg(1, "Starting thread for synching lua state...\n");
// 		std::thread(lambda).detach();
// 	}
// 	else 
// 		lambda();
// }

// template<typename Fn>
// inline void Query::New(Lua::ILuaBase* LUA, Fn func)
// {
// 	auto q = new Query;
// 	auto lambda = [](Lua::ILuaBase* LUA, Query* q, Fn func) {
// 		{
// 			std::unique_lock<std::mutex> lck(q->m_mutex);
// 			q->m_cvar.wait_for(lck, std::chrono::milliseconds(500), [q] { return q->m_sync; });
// 		}

// 		func(LUA, q);
		
// 		q->m_mutex.lock();
// 		q->m_done = true;
// 		q->m_mutex.unlock();

// 		q->m_cvar.notify_one();
// 	};

// 	std::thread(lambda, LUA, q, func).detach();
// 	LUA->PushUserType(q, META);
// }

#endif // QUERY_H