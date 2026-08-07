#pragma once

#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")

#include "draw.hpp"
#include "types.hpp"

extern Drawer g_draw;



void __stdcall flush()
{
	g_draw.Flush();
	printf("Surf\n");
}


namespace S
{
	void* addr = (void*)0x10034217;
	void* orig = nullptr;
	__declspec(naked) void hook()
	{
		__asm
		{
			pushad
			call flush
			popad
			jmp orig
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

bool installed = false;
void _stdcall Install()
{
	if (installed) return;

	MH_CreateHook(S::addr, (void*)S::hook, (void**)&S::orig);
	MH_EnableHook(S::addr);

	installed = true;
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
			call Install
			popad
			jmp orig
		}
	}
}
