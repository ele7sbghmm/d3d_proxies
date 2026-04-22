#include "pch.h"

#include "hooks.hpp"

namespace hooks {
  void __stdcall Draw() {
    g_context.Draw();
  }

  //DECLARE_HOOK_INVOKE(0x49cb40, RenderOpaque, Draw);
  DECLARE_HOOK_INVOKE(0x49cbd0, RenderTransparent, Draw);
  //DECLARE_HOOK(0x4de220, SubmitFencePiecesPseudoCallback);
  namespace SubmitFencePiecesPseudoCallback {
    void* orig = nullptr;
    void __stdcall Bake(D3DVECTOR*, float*, int);
    void hook();
    void install() {
      MH_CreateHook((void*)0x4de220, hook, (void**)&orig); MH_EnableHook((void*)0x4de220);
    }
  };
  void __stdcall SubmitFencePiecesPseudoCallback::Bake(D3DVECTOR* position, float* radius, int collision_area_index) {
    g_context.Commit(*position, *radius, collision_area_index);
  }
  void __declspec(naked) SubmitFencePiecesPseudoCallback::hook() {
    __asm {
      pushad

      lea edi, [esp + 0x24]
      push [edi + 8]
      lea ebx, [edi + 4]
      push ebx
      push [edi + 0]

      call Bake

      popad

      jmp orig
    }
  }

  void install_all() {
    RenderTransparent::install();
    SubmitFencePiecesPseudoCallback::install();
  }
}
