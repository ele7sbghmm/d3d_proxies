#pragma once

#include "context.hpp"

extern Context g_context;

#define DECLARE_HOOK_INVOKE(addr,name,func)\
  namespace name {\
    inline void* orig = nullptr;\
    inline void __declspec(naked) hook() {\
      __asm pushad\
      __asm call func\
      __asm popad\
      __asm jmp orig\
    }\
    inline void install() {\
      MH_CreateHook((void*)addr, hook, (void**)&orig);\
      MH_EnableHook((void*)addr);\
    }\
  }

namespace hooks {
  void Draw();
  void install_all();
}