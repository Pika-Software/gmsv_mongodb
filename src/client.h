#ifndef CLIENT_H
#define CLIENT_H

#include "main.h"

#include <mongocxx/client.hpp>

namespace Client 
{
	extern int META;

	enum STATUS {
		DISCONNECTED = 1,
		CONNECTING = 2,
		CONNECTED = 3,
		FAILED = 4,
		DESTROYED = 5,
	};

	struct ClientStruct {
		mongocxx::client* client;
		int status;
	};

	// Destroying client
	int META_GC(lua_State* L);

	// Returning client status. 
	// See Client::STATUS enum.
	int Status(lua_State* L);

	// Disconnecting from server.
	// After calling this function client destroying.
	int Disconnect(lua_State* L);

	// Connecting to server
	int Connect(lua_State* L);

	// Returning list of databases on server
	int ListDatabases(lua_State* L);

	// Creating MongoDB Client
	int CreateClient(lua_State* L);

	// Initialization
	void Initialize(ILuaBase* LUA);
}

#endif // CLIENT_H