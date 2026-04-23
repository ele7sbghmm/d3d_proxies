#pragma once

#include <d3d8.h>
#include <MinHook.h>
#include <cstdint>
#include "renderer.h"

void __stdcall bake(shar::FenceEntityDSG* fence, D3DVECTOR* position, float* radius, std::int32_t area_index, std::uint32_t flags) {
  #define X86_ZF (1 << 6);
  bool eq = flags & X86_ZF;

  g_renderer.Bake(fence, position, radius, eq, area_index);
}

void __stdcall end() {
  g_renderer.End();
}

namespace hooks {
  namespace FindFenceElems {
    void* addr = (void*)0x4b4bb0;
    void* orig = nullptr;
    void __declspec(naked) hook() {
      __asm {
        pushad
        pushfd

        mov     eax, [esp + 0x5c + 0x24]         // return address
        cmp     eax, 0x4de265             // SubmitFencePiecesPseudoCallback
        jne     Trampoline                // wrong caller

        lea     eax, [esp + 0xa0 + 0x24]

        push    [esp]                    // flags
        push    [eax + 8]                // int             area index
        lea     ebx, [eax + 4]
        push    ebx                      // float*          radius
        push    [eax]                    // D3DVECTOR*      position

        mov     eax, [edx + 0x50]
        push    [eax + ecx * 4]          // FenceEntityDSG*

        call    bake

      Trampoline :
        popfd
        popad

        jmp     orig
      }
    }
    void install() {
      MH_CreateHook((void*)addr, hook, (void**)&orig);
      MH_EnableHook((void*)addr);
    }
  }

  namespace RenderTransparent {
    void* addr = (void*)0x4aadc2;
    void* orig = nullptr;
    void __declspec(naked) hook() {
      __asm {
        pushad
        call end
        popad
        jmp orig
      }
    }
    void install() {
      MH_CreateHook((void*)addr, hook, (void**)&orig);
      MH_EnableHook((void*)addr);
    }
  }

  inline void install_all() {
    FindFenceElems::install();
    RenderTransparent::install();
  }
}