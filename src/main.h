#ifndef MAIN_H
#define MAIN_H

//#define VERSION "1.0.0"
#include "config.h"
#include "GarrysMod/Lua/Interface.h"
using namespace GarrysMod::Lua;

namespace Global 
{
	// Running global function
	// On the top stack should be arguments
	// This function don't pop arguments from stack. You should do this manually
	void Run(ILuaBase* LUA, const char* func, int iArgs, int iResults);

	// Garry's mod initialization
	int Initialize(ILuaBase* LUA);

	// Shutdown
	//int Deinitialize(ILuaBase* LUA);
}

#endif // MAIN_H