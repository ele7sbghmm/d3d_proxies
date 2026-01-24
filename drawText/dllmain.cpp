// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <windows.h>
#include <process.h>
#include <string>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9d.lib")
#pragma comment(linker, "/export:D3DPERF_BeginEvent=C:\\Windows\\System32\\d3d9.D3DPERF_BeginEvent")
#pragma comment(linker, "/export:D3DPERF_EndEvent=C:\\Windows\\System32\\d3d9.D3DPERF_EndEvent")
#pragma comment(linker, "/export:D3DPERF_GetStatus=C:\\Windows\\System32\\d3d9.D3DPERF_GetStatus")
#pragma comment(linker, "/export:D3DPERF_QueryRepeatFrame=C:\\Windows\\System32\\d3d9.D3DPERF_QueryRepeatFrame")
#pragma comment(linker, "/export:D3DPERF_SetMarker=C:\\Windows\\System32\\d3d9.D3DPERF_SetMarker")
#pragma comment(linker, "/export:D3DPERF_SetOptions=C:\\Windows\\System32\\d3d9.D3DPERF_SetOptions")
#pragma comment(linker, "/export:D3DPERF_SetRegion=C:\\Windows\\System32\\d3d9.D3DPERF_SetRegion")
#pragma comment(linker, "/export:DebugSetLevel=C:\\Windows\\System32\\d3d9.DebugSetLevel")
#pragma comment(linker, "/export:DebugSetMute=C:\\Windows\\System32\\d3d9.DebugSetMute")
#pragma comment(linker, "/export:Direct3D9EnableMaximizedWindowedModeShim=C:\\Windows\\System32\\d3d9.Direct3D9EnableMaximizedWindowedModeShim")
#pragma comment(linker, "/export:Direct3DCreate9=C:\\Windows\\System32\\d3d9.Direct3DCreate9")
#pragma comment(linker, "/export:Direct3DCreate9Ex=C:\\Windows\\System32\\d3d9.Direct3DCreate9Ex")
#pragma comment(linker, "/export:Direct3DShaderValidatorCreate9=C:\\Windows\\System32\\d3d9.Direct3DShaderValidatorCreate9")
#pragma comment(linker, "/export:PSGPError=C:\\Windows\\System32\\d3d9.PSGPError")
#pragma comment(linker, "/export:PSGPSampleTexture=C:\\Windows\\System32\\d3d9.PSGPSampleTexture")

typedef HRESULT(STDMETHODCALLTYPE* tEndScene)(IDirect3DDevice9* pDevice);
tEndScene oEndScene = nullptr;
ID3DXFont* pFont = nullptr;


static bool firstFrame = true;
HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice) {
    D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 0, 255);
    RECT rect = { 10., 10., 200., 50. };

    pFont->DrawTextA(NULL, "PROXY ACTIVE", -1, &rect, DT_NOCLIP, color);

    if (firstFrame) {
        MessageBoxA(NULL, "hkEndScene!!!!!!!!!!!!!!!!", "Success", MB_OK);
        firstFrame = false;
    }

    return oEndScene(pDevice);
}


void HookVTable(void** vTable, int index, void* newFunction) {

    MessageBoxA(NULL, "HookVTable!", "Debug", MB_OK);
}


DWORD WINAPI MainThread(LPVOID lpParams) {
    while (GetModuleHandleA("d3d9.dll") == NULL)
        Sleep(100);

    HWND hWnd = CreateWindowA("BUTTON", "Dummy", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, NULL, NULL);

    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D) {
        D3DPRESENT_PARAMETERS d3dpp = { 0 };
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = hWnd;

        IDirect3DDevice9* pDummyDevice = NULL;
        if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice) == D3D_OK) {
            void** vTable = *(void***)pDummyDevice;
            int index = 42;

            DWORD oldProtect;

            VirtualProtect(&vTable[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            oEndScene = (tEndScene)vTable[index];

            vTable[index] = hkEndScene;
            VirtualProtect(&vTable[index], sizeof(void*), oldProtect, &oldProtect);

            pDummyDevice->Release();
        }
        pD3D->Release();
    }

    DestroyWindow(hWnd);

    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);

            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);

            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
        }
    return TRUE;
}
