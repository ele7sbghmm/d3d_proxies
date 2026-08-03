#pragma once

#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")

#include "draw.hpp"
#include "types.hpp"

extern Drawer g_draw;
extern GfxViewParms g_parms;

namespace Hooks
{
	bool installed = false;

	void __stdcall draw()
	{
		g_draw.Flush();
	}
	namespace A
	{
		void* addr = (void*)0x100349c0;
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

}


void __stdcall install()
{
	if (Hooks::installed)
		return;
	MH_CreateHook(Hooks::A::addr, (void*)Hooks::A::hook, (void**)&Hooks::A::orig);
	MH_EnableHook(Hooks::A::addr);
	Hooks::installed = true;
}

namespace I
{
	void* addr = (void*)0x4148a0;
	void* orig = nullptr;
	__declspec(naked) void hook()
	{
		__asm
		{
			pushad
			call install
			popad
			jmp orig
		}
	}
}