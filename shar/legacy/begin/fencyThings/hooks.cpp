#include "pch.h"

#include "hooks.h"
#include "globals.h"
#include "render.h"

auto SetFillmode_Wireframe() -> DWORD;
auto SetFillmode_Point() -> DWORD;
auto Alpha() -> void;

enum : uintptr_t {
	RENDER_OPAQUE                       = 0x0049cb40,
	RENDER_SIMPLE_SHADOWS               = 0x0049cab0,
	RENDER_TRANSLUCENT                  = 0x0049cbd0,
	EVENTMANAGER_HANDLEEVENT            = 0x00432b0d + 3,
	FIND_FENCE_ELEMS_CHECK              = 0x004b4bb0,
	SUBMIT_FENCE_PIECES_PSEUDO_CALLBACK = 0x004de220,
	ADD_GUTS_FENCEENTITYDSG             = 0x004aaf50,
	WORLDRENDERLAYER_RENDER             = 0x004aac00,
	BILLBOARDQUADMANAGER_DISPLAYALL     = 0x0058e070,

	LUCAS_DRAW_FENCE_TRIGGER            = 0x00506507
};

auto __stdcall DrawFenceCache() -> void {
	GameCache::fenceCache.Draw();
}
auto __stdcall UpdateFenceCache() -> void {
	GameCache::fenceCache.Update();
}
auto __stdcall UpdateCaiCache(D3DXVECTOR3* pos, float radius, int cai, FenceEntityDSG* f, uint32_t flags) -> void {
	#define X86_ZF (1 << 6)
	bool isEqual = flags & X86_ZF;
	if (cai != 15 && cai != 0) return;
	if (!GameCache::fenceCache.inited) return;
	if (-1 < cai && cai < FenceCache::MAX_CAIS)
		GameCache::fenceCache.cais[cai].Update(pos, radius, f, isEqual);
}
auto __stdcall AddFence(FenceEntityDSG* fence) -> void {
	GameCache::fenceCache.Add(fence);
}

namespace Hooks::Lucas::DrawFenceTrigger {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)LUCAS_DRAW_FENCE_TRIGGER;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			cmp enabled, 0
			je Skip

			jmp O

		Skip:
			ret
		}
	}
};

namespace Hooks::WorldRenderLayer::AddGuts_FenceEntityDSG {
	static LPVOID Fn = (LPVOID)ADD_GUTS_FENCEENTITYDSG;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			pushad
			pushfd

			push [esp + 4 + 0x24]  // FenceEntityDSG*
			call AddFence
			
			popfd
			popad

			jmp O
		}
	}
}

namespace Hooks::IntersectManager::FindFenceElems {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)FIND_FENCE_ELEMS_CHECK;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			pushad
			pushfd
			
			lea edi, [esp + 0x24]
			push [esp]            //                 flags           

			mov eax, [edx + 0x50]
			push [eax + ecx * 4]  // FenceEntityDSG*

			push [edi + 0xa8]     // int             collision area index
			push [edi + 0xa4]     // float           radius
			push [edi + 0xa0]     // rmt::Vector*    position

			call UpdateCaiCache

			popfd
			popad

			jmp O
		}
	}
}

namespace Hooks::WorldRenderLayer::Render {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)WORLDRENDERLAYER_RENDER;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			cmp enabled, 0
			je Skip

			jmp O

			Skip :
			ret
		}
	}
};


namespace Hooks::BillboardQuadManagerDisplayAll {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)BILLBOARDQUADMANAGER_DISPLAYALL;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			cmp enabled, 0
			je Skip
			
			jmp O

			Skip :
			ret
		}
	}
};

namespace Hooks::RenderOpaque {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)RENDER_OPAQUE;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			cmp enabled, 0
			je Skip

			jmp O

			Skip :
			ret
		}
	}
};

namespace Hooks::RenderSimpleShadows {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)RENDER_SIMPLE_SHADOWS;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			pushad
			pushfd

			call DrawFenceCache

			popfd
			popad

			cmp enabled, 0
			je Skip

			jmp O

			Skip :
			ret
		}
	}
};

namespace Hooks::RenderTranslucent {
	bool enabled = true;
	static LPVOID Fn = (LPVOID)RENDER_TRANSLUCENT;
	inline uintptr_t O = NULL;
	void __declspec(naked) Hk(void) {
		__asm {
			cmp enabled, 0
			je Skip

			jmp O

		Skip:
			ret
		}
	}
};

void Hooks::Create(void) {
	MH_CreateHook(RenderOpaque::Fn, 
		          (LPVOID)RenderOpaque::Hk, 
		          reinterpret_cast<LPVOID*>(&RenderOpaque::O));
	MH_CreateHook(RenderSimpleShadows::Fn, 
		          (LPVOID)RenderSimpleShadows::Hk, 
		          reinterpret_cast<LPVOID*>(&RenderSimpleShadows::O));
	MH_CreateHook(RenderTranslucent::Fn, 
		          (LPVOID)RenderTranslucent::Hk, 
		          reinterpret_cast<LPVOID*>(&RenderTranslucent::O));
	MH_CreateHook(IntersectManager::FindFenceElems::Fn, 
		          (LPVOID)IntersectManager::FindFenceElems::Hk, 
		          reinterpret_cast<LPVOID*>(&IntersectManager::FindFenceElems::O));
	MH_CreateHook(WorldRenderLayer::AddGuts_FenceEntityDSG::Fn, 
		          (LPVOID)WorldRenderLayer::AddGuts_FenceEntityDSG::Hk, 
		          reinterpret_cast<LPVOID*>(&WorldRenderLayer::AddGuts_FenceEntityDSG::O));
	MH_CreateHook(WorldRenderLayer::Render::Fn, 
		          (LPVOID)WorldRenderLayer::Render::Hk, 
		          reinterpret_cast<LPVOID*>(&WorldRenderLayer::Render::O));
	MH_CreateHook(BillboardQuadManagerDisplayAll::Fn,
	              (LPVOID)BillboardQuadManagerDisplayAll::Hk,
		          reinterpret_cast<LPVOID*>(&BillboardQuadManagerDisplayAll::O));
	MH_CreateHook(Lucas::DrawFenceTrigger::Fn, 
		          (LPVOID)Lucas::DrawFenceTrigger::Hk, 
		          reinterpret_cast<LPVOID*>(&Lucas::DrawFenceTrigger::O));

	MH_EnableHook(MH_ALL_HOOKS);
}
