#ifndef CLIENT_H
#define CLIENT_H

#include "pointer.h"
#include "main.h"

#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>

class Client {
public:
	typedef SmartPointer<Client> Ptr;

	static int META;
	enum STATUS {
		DISCONNECTED = 1,
		CONNECTING = 2,
		CONNECTED = 3,
		FAILED = 4,
		DESTROYED = 5,
	};

	std::atomic<int> status = DISCONNECTED;
	mongocxx::pool* pool;

	~Client();
	int Status(); // Get status
	void Status(int s); // Set status

	static Ptr* CheckSelf(Lua::ILuaBase* LUA, int iStackPos = 1);
	static int __tostring(lua_State* L) noexcept;
	static int __gc(lua_State* L) noexcept;
	static int New(lua_State* L) noexcept;
	static int Connect(lua_State* L) noexcept;
	static int Status(lua_State* L) noexcept;
	static int ListDatabases(lua_State* L) noexcept;

	static void Initialize(Lua::ILuaBase* LUA);
	static void Deinitialize(Lua::ILuaBase* LUA);
};

#endif // CLIENT_H