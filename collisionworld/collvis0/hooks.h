#pragma once

#include "MinHook.h"
#include "shar.h"
#include "mod.h"

auto draw() -> void {
	g_collviscontext.commit();
	g_collviscontext.draw();
}
auto drawfe() -> void {
	g_collviscontext.end_scene();
}

#define CREATE_HOOK(addr,name)\
namespace name {\
	void* address = (void*)addr;\
	void* original = nullptr;\
	void hook();\
	void inject() {\
		if (MH_CreateHook(address, (void*)hook, reinterpret_cast<void**>(&original)) != MH_OK)\
			throw;\
		MH_EnableHook(address);\
	}\
}

CREATE_HOOK(0x49cb40, RenderOpaque)
CREATE_HOOK(0x49cbd0, RenderTranslucent)
CREATE_HOOK(0x4a7dc0, FrontEndRenderLayer_Render)
CREATE_HOOK(0x4aadc2, WRL_Render_Call_IsCheatEntered_TREE)

void __declspec(naked) RenderOpaque::hook() { __asm jmp original }
void __declspec(naked) RenderTranslucent::hook() {
	__asm pushad
	__asm call draw
	__asm popad
	__asm jmp original
}
void __declspec(naked) WRL_Render_Call_IsCheatEntered_TREE::hook() { __asm jmp original }
void __declspec(naked) FrontEndRenderLayer_Render::hook() {
	__asm pushad
	__asm call drawfe
	__asm popad
	__asm jmp original
}

auto inject_hooks() -> void {
	RenderOpaque::inject();
	RenderTranslucent::inject();
	FrontEndRenderLayer_Render::inject();
	WRL_Render_Call_IsCheatEntered_TREE::inject();
}
