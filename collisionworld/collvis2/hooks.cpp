#include "pch.h"

#include "hooks.hpp"

namespace hooks {
  void Draw() {
    g_context.Commit();
  }

  //DECLARE_HOOK_INVOKE(0x49cb40,RenderOpaque,Draw);
  namespace RenderOpaque {
    void* orig = nullptr;
    void __declspec(naked) hook() {
      __asm pushad
      __asm call Draw
      __asm popad
      __asm jmp orig
    }
    void install() {
      MH_CreateHook((void*)0x49cb40, hook, (void**)&orig);
      MH_EnableHook((void*)0x49cb40);
    }
  };

  void install_all() {
    RenderOpaque::install();
  }
}