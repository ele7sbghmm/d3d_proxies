#pragma once

#include "context.hpp"

extern Context g_context;

#define DECLARE_HOOK(addr,name)\
  namespace name {\
    void* orig = nullptr;\
    void hook(); \
    void install() {\
      MH_CreateHook((void*)addr, hook, (void**)&orig);\
      MH_EnableHook((void*)addr);\
    }\
  }

#define DECLARE_HOOK_INVOKE(addr,name,func)\
  DECLARE_HOOK(addr,name);\
  void __declspec(naked) name##::hook() {\
    __asm pushad\
    __asm call func\
    __asm popad\
    __asm jmp orig\
  }

namespace hooks {
  void __stdcall Draw();
  void __stdcall Bake(int);
  void install_all();
}
