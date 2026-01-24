#include "pch.h"
#include <windows.h>
#include <d3d9.h>

// 1. FORWARD BINK CALLS
// These tell the game: "If you want to play a video, ask binkw32_original.dll"
#pragma comment(lib, "d3d9.lib")
#pragma comment(linker, "/export:_BinkClose@4=binkw32_original._BinkClose@4")
#pragma comment(linker, "/export:_BinkDoFrame@4=binkw32_original._BinkDoFrame@4")
#pragma comment(linker, "/export:_BinkNextFrame@4=binkw32_original._BinkNextFrame@4")
#pragma comment(linker, "/export:_BinkOpen@8=binkw32_original._BinkOpen@8")
#pragma comment(linker, "/export:_BinkPause@8=binkw32_original._BinkPause@8")
#pragma comment(linker, "/export:_BinkCopyToBuffer@28=binkw32_original._BinkCopyToBuffer@28")
#pragma comment(linker, "/export:_BinkSetPan@12=binkw32_original._BinkSetPan@12")
#pragma comment(linker, "/export:_BinkSetVolume@12=binkw32_original._BinkSetVolume@12")
#pragma comment(linker, "/export:_BinkGetError@0=binkw32_original._BinkGetError@0")
#pragma comment(linker, "/export:_BinkSetIO@4=binkw32_original._BinkSetIO@4")
#pragma comment(linker, "/export:_BinkSetSoundTrack@8=binkw32_original._BinkSetSoundTrack@8")
#pragma comment(linker, "/export:_BinkSetSoundSystem@8=binkw32_original._BinkSetSoundSystem@8")
#pragma comment(linker, "/export:_BinkOpenDirectSound@4=binkw32_original._BinkOpenDirectSound@4")
#pragma comment(linker, "/export:_BinkCopyToBufferRect@44=binkw32_original._BinkCopyToBufferRect@44")
#pragma comment(linker, "/export:_BinkWait@4=binkw32_original._BinkWait@4")
#pragma comment(linker, "/export:_RADSetMemory@8=binkw32_original._RADSetMemory@8")
#pragma comment(linker, "/export:_RADTimerRead@0=binkw32_original._RADTimerRead@0")

// 2. YOUR SEARCH LOGIC
DWORD WINAPI MainThread(LPVOID lpParams) {
    // Wait for the game to finish loading the D3D9 wrapper mod
    Sleep(8000);

    // Find the D3D9 module that the game is ACTUALLY using
    HMODULE hD3D9 = GetModuleHandleA("d3d9.dll");

    if (!hD3D9) return 0;

    Beep(1000, 500);

    unsigned char endScenePattern[] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x6A, 0xFF };
    uintptr_t endSceneAddr = FindPattern(hD3D9, endScenePattern, sizeof(endScenePattern));

    if (!endSceneAddr) return 0;
    DWORD old;
    VirtualProtect((void*)endSceneAddr, 1, PAGE_EXECUTE_READWRITE, &old);
    *(unsigned char*)endSceneAddr = 0xCC;
    VirtualProtect((void*)endSceneAddr, 1, old, &old);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0, MainThread, 0, 0, 0);
    }
    return TRUE;
}