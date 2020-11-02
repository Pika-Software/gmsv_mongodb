// WARNING! SHITCODE!

#ifndef QUERY_H
#define QUERY_H

#include "client.h"
#include "main.h"

#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>
#include <deque>

class Query;

typedef std::function<void(Lua::ILuaBase* LUA, Query* q)> QueryThreadFunc;
typedef std::function<void(Lua::ILuaBase* LUA)> QueryFunc;

struct QueryData {
	bool done = false;
	std::mutex mtx;
	std::condition_variable cv;
	bool isDone() { return done; };
};

class Query {
public:
	static int META;

	static std::mutex ListMutex;
	static std::deque<QueryData*> DataList;
	static std::deque<std::pair<Query*, QueryData*>> RemoveList;

	std::mutex m_mutex;
	std::condition_variable m_cvar;
	bool m_done = false;
	bool m_sync = false;

	void Acquire(Lua::ILuaBase* LUA, QueryFunc func);
	bool isUnblocked();

	static void New(Lua::ILuaBase* LUA, QueryThreadFunc func);
	static Query* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
	static int __tostring(lua_State* L);
	static int Wait(lua_State* L);
	static int Think(lua_State* L);
	static void Initialize(Lua::ILuaBase* LUA);
};

#endif // QUERY_H