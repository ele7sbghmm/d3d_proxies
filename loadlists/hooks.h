#pragma once

#include <cstdio>

auto CreateHooks() -> void;

#define DEFINE_HOOK(addr,name)\
	namespace name {\
		inline uintptr_t Fn = (uintptr_t)addr;\
		inline uintptr_t O = NULL;\
		auto Hk(void) -> void;\
		inline auto Create() -> void {\
			MH_CreateHook((LPVOID)Fn,\
						  (LPVOID)Hk,\
						  reinterpret_cast<LPVOID*>(&O));\
		}\
	}

/// WorldScene
DEFINE_HOOK(0x0049cb40, WorldScene_RenderOpaque);
DEFINE_HOOK(0x0049cbd0, WorldScene_RenderTranslucents);
/// WorldRenderLayer
DEFINE_HOOK(0x004aadc2, WorldRenderLayer_Render_SHOWTREE);

/// pddi
inline uintptr_t PDDI_BASE = (uintptr_t)GetModuleHandleA("pddidx8r.dll");
DEFINE_HOOK(PDDI_BASE + 0x85a4, Pddi_Call_CreateDevice);
DEFINE_HOOK(PDDI_BASE + 0x84fe, Pddi_esi_ppReturnedDeviceInterface);
DEFINE_HOOK(PDDI_BASE + 0x85ab, Pddi_esi_pReturnedDeviceInterface);
