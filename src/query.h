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

	//static std::mutex ThreadMutex;
	//static std::condition_variable ThreadCVar;
	//static bool ThreadStopped;
	//static std::queue<SmartPointer<Query>*> ThreadQueue;
	//static std::promise<SmartPointer<Query>*> ThreadPromise;
	//static std::future<SmartPointer<Query>*> ThreadFuture;

	static void New(Lua::ILuaBase* LUA, std::function<void(Query*)> func);
	static int Wait(lua_State* L);

	//static void ThreadFunc();
	static int Think(lua_State* L);

	static void Initialize(Lua::ILuaBase* LUA);
	static void Deinitialize();
};

#endif // QUERY_H