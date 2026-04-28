#pragma once

#include <Minhook.h>
#pragma comment(lib, "libMinhook.x86.lib")
#include "shar.hpp"
#include "server.hpp"

namespace hooks {
    namespace FindFenceElems {
        inline void* address = (void*)0x4b4ba3;
        inline void* original = nullptr;
        inline void __stdcall send(Shar::FenceEntityDSG* fence, float range, float distance, D3DVECTOR* position, float radius, int collision_area_index) {
            server::Fence fence_data = {
                { fence->mStartPoint.x, fence->mStartPoint.y, fence->mStartPoint.z },
                { fence->mEndPoint.x,   fence->mEndPoint.y,   fence->mEndPoint.z },
                { fence->mNormal.x,     fence->mNormal.y,     fence->mNormal.z },
                { position->x,          position->y,          position->z },
                range,
                distance,
                radius,
                collision_area_index
            };
            server::SendFence(fence_data);
        }
        inline void __declspec(naked) hook() {
            __asm {
                pushad

                mov eax, [esp + 0x5c + 0x20]
                cmp eax, 0x4de265
                jne Trampoline
                lea eax, [esp + 0xa0 + 0x20]

                push [eax + 8]
                push [eax + 4]
                push [eax]

                sub esp, 8
                fxch st(1)
                fst float ptr[esp + 4]
                fxch st(1)
                fst float ptr[esp]

                mov eax, [edx + 0x50]
                push[eax + ecx * 4]

                call send

                Trampoline :
                popad

                    jmp original
            }
        }
        inline void install() {
            MH_CreateHook(address, hook, (void**)&original);
            MH_EnableHook(address);
        }
    }

    inline void install_all() {
        FindFenceElems::install();
    }
};
