// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <windows.h>
#include <process.h>
#include <string>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9d.lib")
#pragma comment(linker, "/export:Direct3DCreate9=C:\\Windows\\SysWOW64\\d3d9.Direct3DCreate9")
#pragma comment(linker, "/export:Direct3DCreate9Ex=C:\\Windows\\SysWOW64\\d3d9.Direct3DCreate9Ex")
#pragma comment(linker, "/export:D3DPERF_BeginEvent=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_BeginEvent")
#pragma comment(linker, "/export:D3DPERF_EndEvent=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_EndEvent")
#pragma comment(linker, "/export:D3DPERF_GetStatus=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_GetStatus")
#pragma comment(linker, "/export:D3DPERF_QueryRepeatFrame=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_QueryRepeatFrame")
#pragma comment(linker, "/export:D3DPERF_SetMarker=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_SetMarker")
#pragma comment(linker, "/export:D3DPERF_SetOptions=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_SetOptions")
#pragma comment(linker, "/export:D3DPERF_SetRegion=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_SetRegion")
#pragma comment(linker, "/export:DebugSetLevel=C:\\Windows\\SysWOW64\\d3d9.DebugSetLevel")
#pragma comment(linker, "/export:DebugSetMute=C:\\Windows\\SysWOW64\\d3d9.DebugSetMute")
#pragma comment(linker, "/export:Direct3D9EnableMaximizedWindowedModeShim=C:\\Windows\\SysWOW64\\d3d9.Direct3D9EnableMaximizedWindowedModeShim")
#pragma comment(linker, "/export:Direct3DShaderValidatorCreate9=C:\\Windows\\SysWOW64\\d3d9.Direct3DShaderValidatorCreate9")
#pragma comment(linker, "/export:PSGPError=C:\\Windows\\SysWOW64\\d3d9.PSGPError")
#pragma comment(linker, "/export:PSGPSampleTexture=C:\\Windows\\SysWOW64\\d3d9.PSGPSampleTexture")

// 2. DEFINITIONS
typedef HRESULT(STDMETHODCALLTYPE* tEndScene)(IDirect3DDevice9*);
typedef HRESULT(STDMETHODCALLTYPE* tCreateDevice)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);

tEndScene oEndScene = nullptr;
tCreateDevice oCreateDevice = nullptr;
ID3DXFont* pFont = nullptr;

// 3. THE HOOKED ENDSCENE (The Drawing Room)
HRESULT STDMETHODCALLTYPE hkEndScene(IDirect3DDevice9* pDevice) {
    MessageBoxA(NULL, "hkEndScene!", "Success", MB_OK);

    if (!pFont) {
        D3DXCreateFontA(pDevice, 20, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &pFont);
    }

    RECT rect = { 10, 10, 300, 50 };
    pFont->DrawTextA(NULL, "HOOK STATUS: ACTIVE", -1, &rect, DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 255, 0));

    return oEndScene(pDevice);
}

// 4. THE HOOKED CREATEDEVICE (The Hijacker)
HRESULT STDMETHODCALLTYPE hkCreateDevice(IDirect3D9* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    MessageBoxA(NULL, "hkCreateDevice!", "Success", MB_OK);

    HRESULT hr = oCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    if (hr == D3D_OK && ppReturnedDeviceInterface != nullptr) {
        void** vTable = *(void***)(*ppReturnedDeviceInterface);
        DWORD old;
        VirtualProtect(&vTable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
        oEndScene = (tEndScene)vTable[42];
        vTable[42] = (void*)hkEndScene;
        VirtualProtect(&vTable[42], sizeof(void*), old, &old);
    }
    return hr;
}

extern "C" IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
    // Use the full path to avoid the DLL loading itself!
    HMODULE hRealD3D = GetModuleHandleA("C:\\Windows\\System32\\d3d9.dll");
    if (!hRealD3D) hRealD3D = LoadLibraryA("C:\\Windows\\System32\\d3d9.dll");

    if (!hRealD3D) return nullptr;

    typedef IDirect3D9* (WINAPI* tD3DCreate9)(UINT);
    tD3DCreate9 oDirect3DCreate9 = (tD3DCreate9)GetProcAddress(hRealD3D, "Direct3DCreate9");

    // Just return the real object for now to see if the game starts
    return oDirect3DCreate9(SDKVersion);
}

// Inside your MainThread
DWORD WINAPI MainThread(LPVOID lpParams) {
    Sleep(1000);
    MessageBoxA(NULL, "MainThread!", "Success", MB_OK);

    // 1. Wait for the game to actually initialize its own D3D
    // If we hook too early, we hook a "clean" VTable that gets overwritten

    // 2. Find the game's d3d9.dll module in its own memory
    HMODULE d3d9Base = GetModuleHandleA("d3d9.dll");
    if (!d3d9Base) return 0;

    // 3. We use the "Dummy" method but we DON'T Release it immediately
    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D) {
        D3DPRESENT_PARAMETERS d3dpp = { 0 };
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = GetForegroundWindow();

        IDirect3DDevice9* pDummyDevice = NULL;
        if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice) == D3D_OK) {

            // Get the VTable from the dummy
            void** vTable = *(void***)pDummyDevice;

            // HOOK: Index 42 is EndScene
            DWORD old;
            VirtualProtect(&vTable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
            oEndScene = (tEndScene)vTable[42];
            vTable[42] = (void*)hkEndScene;
            VirtualProtect(&vTable[42], sizeof(void*), old, &old);

            // IMPORTANT: If we release pDummyDevice, some games reset the VTable.
            // Just let it sit in memory.
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            Sleep(2000);

            // Optional: improves performance by not notifying this DLL when threads are created
            DisableThreadLibraryCalls(hModule);

            // Create our "hack" thread
            // We pass hModule so the thread can unload itself if needed
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);

            // Close the handle to the thread (this doesn't kill the thread, just cleans up)
            if (hThread) {
                CloseHandle(hThread);
            }
            break;
        }
        case DLL_PROCESS_DETACH:
            // Cleanup code if necessary
            break;
    }
    return TRUE;
}
