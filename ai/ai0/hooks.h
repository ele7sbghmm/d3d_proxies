#pragma once

#include <Minhook.h>
#pragma comment(lib, "libMinhook.x86.lib")

#include "ai.h"

void draw() {
    g_ai.populate();
    g_ai.draw();
}

namespace hooks {
    namespace is_cheat_entered {
        inline void* addr = (void*)0x4aadc2;
        inline void* orig = nullptr;
        void hook();
        inline void inject() {
            MH_CreateHook(addr, hook, (void**)&orig);
            MH_EnableHook(addr);
        }
    }
}

__declspec(naked) inline void hooks::is_cheat_entered::hook() {
    __asm {
        pushad
        call draw
        popad
        jmp orig
    }
}
