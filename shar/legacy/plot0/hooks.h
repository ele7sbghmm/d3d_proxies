#pragma once

#define DEFINE_HOOK(addr,name)\
	namespace Hooks {\
		namespace name {\
			inline uintptr_t Address = (uintptr_t)addr;\
			inline uintptr_t Original = NULL;\
			void Hook();\
			inline void Create() {\
				if (MH_CreateHook(\
					(void*)Address,\
					(void*)Hook,\
					reinterpret_cast<void**>(&Original)\
				) != MH_OK) throw;\
			}\
		}\
	}

DEFINE_HOOK(0x49cb40, RenderOpaque);
DEFINE_HOOK(0x4ac4d0, DoPostDynaLoad);

auto CreateHooks() -> void;
