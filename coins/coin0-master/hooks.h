#pragma once

#include <MinHook.h>
#include "shar.h"
#include "mod.h"

extern void RenderGui();
extern void coin_set_zfunc();
namespace Mod { extern Context context; }

namespace Hooks {
  namespace CoinManager {
    namespace Coin_transition {
      void* address = reinterpret_cast<void*>(0x506353);
      void* original = nullptr;

      __declspec(naked) void hook() {
        //static Mod::Context* context = &Mod::context;
        //__asm mov dword ptr [context.pos], edi
        __asm jmp original
      }
      void inject() {
        MH_CreateHook(address,
          reinterpret_cast<void*>(hook),
          reinterpret_cast<void**>(&original)
        );
      }
    }

    namespace Render_begin {
      void* address = reinterpret_cast<void*>(0x5064ad);
      void* original = nullptr;

      __declspec(naked) void hook() {
        __asm pushad
        __asm call coin_set_zfunc
        __asm popad
        __asm jmp original
      }
      void inject() {
        MH_CreateHook(address,
          reinterpret_cast<void*>(hook),
          reinterpret_cast<void**>(&original)
        );
      }
    }
    namespace Render_clean {
      void* address = reinterpret_cast<void*>(0x5064b6);
      void* original = nullptr;

      void Render_clean() {
        IDirect3DDevice8* device = (*Shar::p3d::context_p)->GetDisplay()->GetD3DDevice();
        device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
      }

      __declspec(naked) void hook() {
        __asm pushad
        __asm call Render_clean
        __asm popad
        __asm jmp original
      }
      void inject() {
        MH_CreateHook(address,
          reinterpret_cast<void*>(hook),
          reinterpret_cast<void**>(&original)
        );
      }
    }
  }
  namespace FE_Render {
    void* address = reinterpret_cast<void*>(0x4a7e6a);
    void* original = nullptr;

    __declspec(naked) void hook() {
      __asm pushad
      __asm call RenderGui
      __asm popad
      __asm jmp original
    }
    void inject() {
      MH_CreateHook(address,
        reinterpret_cast<void*>(hook),
        reinterpret_cast<void**>(&original)
      );
    }
  }

  void inject_hooks() {
    MH_Initialize();
    CoinManager::Render_begin::inject();
    CoinManager::Render_clean::inject();
    FE_Render::inject();
    CoinManager::Coin_transition::inject();
    MH_EnableHook(MH_ALL_HOOKS);
  }
}