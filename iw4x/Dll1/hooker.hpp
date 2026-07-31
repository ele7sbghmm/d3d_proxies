
#include "../../vendor/minhook/include/MinHook.h"
#pragma comment(lib, "../../vendor/minhook/lib/libMinHook.x86.lib")

extern Drawer g_Drawer;

namespace Hooker
{
	void DrawR()
	{
		g_Drawer.Begin();
		g_Drawer.Line({ 0, -10, -10000 }, { 0, -10, 10000 }, 0xffff0000);
		g_Drawer.Flush();
	}
	void DrawG()
	{
		g_Drawer.Begin();
		g_Drawer.Line({ 0, 0, -10000 }, { 0, 0, 10000 }, 0xff00ff00);
		g_Drawer.Flush();
	}
	void DrawB()
	{
		g_Drawer.Begin();
		g_Drawer.Line({ 0, 10, -10000 }, { 0, 10, 10000 }, 0xff0000ff);
		g_Drawer.Flush();
	}

	namespace R
	{
		void* addr = (void*)0x555890;
		void* orig = nullptr;
		__declspec(naked) void Hook()
		{
			__asm
			{
				pushad
				call DrawR
				popad
				jmp orig
			}
		}
		void Create()
		{
			MH_CreateHook(addr, (void*)Hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}
	namespace G
	{
		void* addr = (void*)0x555be0;
		void* orig = nullptr;
		__declspec(naked) void Hook()
		{
			__asm
			{
				pushad
				call DrawG
				popad
				jmp orig
			}
		}
		void Create()
		{
			MH_CreateHook(addr, (void*)Hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}
	namespace B
	{
		void* addr = (void*)0x555a38;
		void* orig = nullptr;
		__declspec(naked) void Hook()
		{
			__asm
			{
				pushad
				call DrawB
				popad
				jmp orig
			}
		}
		void Create()
		{
			MH_CreateHook(addr, (void*)Hook, (void**)&orig);
			MH_EnableHook(addr);
		}
	}

	void Init()
	{
		MH_Initialize();
		R::Create();
		G::Create();
		B::Create();
	}
}
