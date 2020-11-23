#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include "config.h"
#include "GarrysMod/Lua/Interface.h"

using namespace GarrysMod;
using bsoncxx::stdx::optional;
using bsoncxx::stdx::make_optional;
using bsoncxx::stdx::string_view;

#ifdef WIN32
typedef void(__stdcall *f_DevMsg)(int level, const char* pMsg, ...);
#elif __linux
typedef void(*f_DevMsg)(int level, const char* pMsg, ...);
#endif

extern f_DevMsg DevMsg;

class Global {
public:
	static constexpr const char* DEVMSG_PREFIX = "[MongoDB Debug] ";

	static void LoadEngine();

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