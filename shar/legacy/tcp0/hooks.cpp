#include "pch.h"

#include "hooks.h"
#include "server.h"

#define DECLARE_HOOK(target,name)                                        \
  namespace name {                                                       \
    void* trampoline = nullptr;                                          \
    void detour();                                                       \
    void install() {                                                     \
      MH_CreateHook((void*)target, detour, (void**)&trampoline);  \
      MH_EnableHook((void*)target);                                      \
    }                                                                    \
  }

namespace hooks {
  DECLARE_HOOK(0x4ac4d0,DoPostDynaLoad);
  using server::Send_DoPostDynaLoad;
  void __declspec(naked) DoPostDynaLoad::detour() {
    __asm pushad
    __asm call Send_DoPostDynaLoad
    __asm popad
    __asm jmp trampoline
  }

  void install_game_hooks() {
    MH_Initialize();
    DoPostDynaLoad::install();
  }
}
