#pragma once

#include <Minhook.h>
#pragma comment(lib, "libMinhook.x86.lib")
#include <d3d8.h>

namespace hooks {
    namespace pddi_set_projection_mode {
        inline void* addr = (void*)0x10002270;
        inline void* orig = nullptr;
        inline void hook();
        inline void inject() {
            MH_CreateHook(addr, hook, (void**)&orig);
            MH_EnableHook(addr);
        }
    }
}

inline __declspec(naked) void hooks::pddi_set_projection_mode::hook() {
    __asm {
        cmp [esp+4], 0
        jne Jump
        mov [esp+4], 1
    Jump:
        jmp orig
    }
}
