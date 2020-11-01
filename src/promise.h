
#ifndef PROMISE_H
#define PROMISE_H

#include "main.h"

#include <condition_variable>
#include <mutex>

extern int META_PROMISE;

class Promise {
public:
	bool m_done = false;
	std::mutex m_mtx;
	std::condition_variable m_cv;

	static Promise* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
template<class Fn>
	static void New(Lua::ILuaBase* LUA, Fn func);
	static int __gc(lua_State* L);
	static int __tostring(lua_State* L);
	static int Wait(lua_State* L);
	static int IsValid(lua_State* L);

	static void Initialize(Lua::ILuaBase* LUA);
};

#endif // PROMISE_H