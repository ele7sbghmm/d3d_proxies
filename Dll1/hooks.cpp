#include "pch.h"

#include "hooks.h"
#include "context.h"

//extern RenderContext<XYZNormalColor> g_context;
extern RenderContext g_context;

auto Draw() -> void {
	g_context.PopulateTransient();
	g_context.Draw();
}
auto RenderOpaque_detour() -> void {
	Draw();
}
auto DoPostDynaLoad_detour() -> void {

}

auto CreateHooks() -> void {
	Hooks::RenderOpaque::Create();
	//Hooks::DoPostDynaLoad::Create();

	MH_EnableHook(MH_ALL_HOOKS);
}

void __declspec(naked) Hooks::RenderOpaque::Hook() {
	__asm {
		call RenderOpaque_detour

		jmp Original
	}
}

void __declspec(naked) Hooks::DoPostDynaLoad::Hook()  {
	__asm {
		call DoPostDynaLoad_detour

		jmp Original
	}
}
