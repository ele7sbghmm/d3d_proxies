#include "pch.h"

#include <d3dx8.h>
#include "hooks.h"
#include "srr2_types.h"
#include "globals.h"

auto CreateHooks() -> void {
	WorldScene_RenderOpaque::Create();
	WorldScene_RenderTranslucents::Create();
	WorldRenderLayer_Render_SHOWTREE::Create();
	Pddi_esi_pReturnedDeviceInterface::Create();
	MH_EnableHook(MH_ALL_HOOKS);
}

auto DrawFences() { fenceCache.Draw(); }

auto __declspec(naked) WorldScene_RenderOpaque::Hk(void) -> void { __asm { jmp O } }
auto __declspec(naked) WorldScene_RenderTranslucents::Hk(void) -> void { __asm { jmp O } }
auto __declspec(naked) WorldRenderLayer_Render_SHOWTREE::Hk(void) -> void {
	__asm {
		pushad
		pushfd

		call DrawFences

		popfd
		popad

		jmp O
	}
}
auto __declspec(naked) Pddi_esi_pReturnedDeviceInterface::Hk(void) -> void {
	__asm {
		pushad
		pushfd

		mov eax, [esi]
		mov Device, eax

		popfd
		popad

		jmp O
	}
}
