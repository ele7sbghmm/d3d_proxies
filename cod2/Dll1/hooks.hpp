#pragma once

#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")

#include "draw.hpp"
#include "types.hpp"

extern Drawer g_draw;
extern GfxViewParms g_parms;

bool linerender = false;
namespace Hooks
{
	bool installed = false;

	void __stdcall draw(int* esp)
	{
		if (*esp == 0x1003210D) {
			g_draw.Flush();
			linerender = false;
		}
	}
	namespace A
	{
		void* addr = (void*)0x10038af6;
		void* orig = nullptr;
		__declspec(naked) void hook()
		{
			__asm
			{
				pushad
				mov eax, esp
				add eax, 0x7c + 0x20
				push eax
				call draw
				popad
				jmp orig
			}
		}
	}
	void __stdcall line()
	{
		linerender = true;
	}
	namespace L
	{
		void* addr = (void*)0x10031c10;
		void* orig = nullptr;
		__declspec(naked) void hook()
		{
			__asm
			{
				pushad
				call line
				popad
				jmp orig
			}
		}
	}
}

void __stdcall AddCustomLine()
{
	float p1[3] = { 0.0f, 0.0f, -10000.0f };
	float p2[3] = { 0.0f, 0.0f, 10000.0f };
	float p3[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	int p4 = 0;
	int p5 = 0;
	int p6 = 1;

	uintptr_t addr = 0x412300;
	__asm
	{
		push p6
		push p5
		push p4
		lea esi, p3
		lea edi, p2
		lea ebx, p1
		call addr
		add esp, 12
	}
}

void __stdcall install()
{
	if (Hooks::installed)
		return;
	MH_CreateHook(Hooks::A::addr, (void*)Hooks::A::hook, (void**)&Hooks::A::orig);
	MH_EnableHook(Hooks::A::addr);
	//MH_CreateHook(Hooks::L::addr, (void*)Hooks::L::hook, (void**)&Hooks::L::orig);
	//MH_EnableHook(Hooks::L::addr);
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
			call AddCustomLine
			call install
			popad
			jmp orig
		}
	}
}