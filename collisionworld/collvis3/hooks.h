#pragma once

#include <d3d8.h>
#include <MinHook.h>
#include <cstdint>
#include "renderer.h"

void __stdcall bake(shar::FenceEntityDSG* fence, float range, float distance_to, D3DVECTOR* position, int area_index) {
  g_renderer.Bake(fence, range, distance_to, position, area_index);
}

void __stdcall end() {
  g_renderer.End();
}

namespace hooks {
  namespace FindFenceElems {
    void* addr = (void*)0x4b4ba3; // fcompp
    void* orig = nullptr;
    void __declspec(naked) hook() {
      __asm {
        pushad

        mov     eax, [esp + 0x5c + 0x20]  // return address
        cmp     eax, 0x4de265             // from SubmitFencePiecesPseudoCallback
        jne     Trampoline                // wrong caller

        lea     eax, [esp + 0xa0 + 0x20]

        push    [eax + 8]                 // int             area index
        push    [eax]                     // D3DVECTOR*      position
        
        sub     esp, 8
        fxch    st(1)
        fst     float ptr [esp+4]         // float           distance to fence
        fxch    st(1)
        fst     float ptr [esp]           // float           fence's activate distance

        mov     eax, [edx + 0x50]
        push    [eax + ecx * 4]           // FenceEntityDSG*

        call    bake

      Trampoline :
        popad

        jmp     orig
      }
    }
    void install() {
      MH_CreateHook((void*)addr, hook, (void**)&orig);
      MH_EnableHook((void*)addr);
    }
  }
  #define OPAQUE 
  #define TRANSPARENT 0x4aadc2
  #define SHOW_TREE 0x4aadc2;
  namespace Render {
    void* addr = (void*)SHOW_TREE;
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
    Render::install();
  }
}