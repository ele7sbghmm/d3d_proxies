
#include "../../vendor/minhook/include/MinHook.h"
#pragma comment(lib, "../../vendor/minhook/lib/libMinHook.x86.lib")

extern Drawer g_Drawer;

namespace Hooker
{
	void Draw()
	{
		g_Drawer.Begin();
		g_Drawer.Flush();
	}

	namespace A
	{
		void* addr = (void*)0x555be0;
		void* orig = nullptr;
		__declspec(naked) void Hook()
		{
			__asm
			{
				pushad
				call Draw
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
		A::Create();
	}
}
