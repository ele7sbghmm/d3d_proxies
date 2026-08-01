#pragma once

#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")

#include "draw.hpp"

extern Drawer g_draw;

namespace Hooks
{
	void draw()
	{
		g_draw.Flush();
	}
	namespace A
	{
		void* addr = (void*)0x004cfe8f;
		void* orig = nullptr;
		__declspec(naked) void hook()
		{
			__asm
			{
				pushad
				call draw
				popad
				jmp orig
			}
		}
	}
	void install()
	{
		MH_Initialize();
		MH_CreateHook(A::addr, (void*)A::hook, (void**)&A::orig);
		MH_EnableHook(A::addr);
	}
}
