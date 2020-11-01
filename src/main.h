#ifndef MAIN_H
#define MAIN_H

#include <string>
#include "config.h"
#include "GarrysMod/Lua/Interface.h"

using namespace GarrysMod;

class Global {
public:
	static constexpr const char* DEVMSG_PREFIX = "[MongoDB Debug] ";

	static void LoadEngine();

	static void DevMsg(int level, const char* pMsg, ...);

	// Running global function
	// On the top stack should be arguments
	// This function don't pop arguments from stack. You should do this manually
	static void Run(Lua::ILuaBase* LUA, const char* func, int iArgs, int iResults);

	static void Print(Lua::ILuaBase* LUA, const char* message);

	// Converting pointer address to string
	static std::string PtrToStr(const void* addr);

	// Garry's mod initialization
	static int Initialize(Lua::ILuaBase* LUA);
	static int Deinitialize(Lua::ILuaBase* LUA);
};

#endif // MAIN_H