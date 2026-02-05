#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <d3dx8.h>
#include "include/MinHook.h"
#include "worldsim/coins/coinmanager.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "./libs/dx8/d3d8.lib")
#pragma comment(lib, "./libs/dx8/d3dx8.lib")
#pragma comment(lib, "./libs/libMinHook.x86.lib")

void coinPos(CoinManager* cm, char x0[], char x1[], char y0[], char y1[], char z0[], char z1[]);

// 3. The Hook Function
typedef HRESULT(STDMETHODCALLTYPE* tCreateDevice)(void*, UINT, DWORD, HWND, DWORD, void*, void**);
tCreateDevice oCreateDevice = nullptr;

typedef HRESULT(STDMETHODCALLTYPE* tEndScene)(IDirect3DDevice8*);
tEndScene oEndScene = nullptr;

struct CustomVertex {
    float x, y, z, rhw;
    DWORD color;
};

// This "Flexible Vertex Format" (FVF) tells D3D what data we are sending
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)


void DrawBox(IDirect3DDevice8* pDevice, float x, float y, float w, float h, DWORD color) {
    // Define the 4 corners of the box
    // Order: Top-Left, Top-Right, Bottom-Left, Bottom-Right
    CustomVertex vertices[4] = {
        { x,     y,     0.0f, 1.0f, color }, // TL
        { x + w, y,     0.0f, 1.0f, color }, // TR
        { x,     y + h, 0.0f, 1.0f, color }, // BL
        { x + w, y + h, 0.0f, 1.0f, color }  // BR
    };

    // Tell D3D what kind of vertices we are sending
    // D3DFVF_XYZRHW = 2D coordinates, D3DFVF_DIFFUSE = Color
    pDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    // Draw the triangles
    pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CustomVertex));
}

void DrawLine(IDirect3DDevice8* pDevice, float x1, float y1, float x2, float y2, DWORD color) {
    //Beep(600, 100);

    CustomVertex vertices[2] = {
        { x1, y1, 0.0f, 1.0f, color },
        { x2, y2, 0.0f, 1.0f, color }
    };

    // 1. Tell D3D what kind of vertices we are using
    pDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);

    // 2. Draw the line (Primitive Type: Line List, 1 line from 2 vertices)
    pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(CustomVertex));
}

static CoinManager** ppCoinManager = (CoinManager**)0x6c8450;

static int nextInactiveCoinPrevious = 0;
static int nextInactiveCoinLastFrame = 0;

LPD3DXFONT pFont = nullptr;
D3DSURFACE_DESC desc;
IDirect3DSurface8* pBackBuffer = nullptr;
static DWORD lastUpdate = 0;
HRESULT STDMETHODCALLTYPE hkEndScene(IDirect3DDevice8* pDevice) {
    // 1. Re-initialize font if it was lost (due to Reset)
    if (pFont == nullptr) {
        D3DXCreateFont(pDevice, CreateFontA(12, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial"), &pFont);
    }

    // 2. Force the render states every single frame
    // These are the most important for "Overlay" drawing
    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);          // Disable Depth Testing (Stay on top)
    pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);  // Ensure we aren't in wireframe
    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);          // No shadows/light on our line
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);   // Draw both sides
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);  // Enable transparency for text
    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // 3. Clear the Texture Slot (Important!)
    // If the game just drew a car, the car's texture is still "active"
    // and will try to wrap itself around your line. Set to NULL for solid color.
    pDevice->SetTexture(0, NULL);

    // 4. Draw
    DrawLine(pDevice, 0.0f, 0.0f, 1280.0f, 720.0f, 0xFFFF0000);

    if (SUCCEEDED(pDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer))) {
        pBackBuffer->GetDesc(&desc);

        float screenWidth = (float)desc.Width;
        float screenHeight = (float)desc.Height;

        // Don't forget to release the back buffer reference to avoid memory leaks!
        pBackBuffer->Release();

        // Use screenWidth for your Box/Text calculations...
    }
    do {
        static char x0[5000] = ""; static char x1[5000] = "";
        static char y0[5000] = ""; static char y1[5000] = "";
        static char z0[5000] = ""; static char z1[5000] = "";
        
        if (pFont == nullptr) break;
        if (ppCoinManager == nullptr) break;
        CoinManager* pCoinManager = *ppCoinManager;

        if (pCoinManager->mActiveCoins != nullptr) {
            DWORD currentTime = GetTickCount();
            if (currentTime - lastUpdate > 1000) {
                coinPos(pCoinManager, x0, x1, y0, y1, z0, z1);
            }

            DrawBox(pDevice, 0, 0, 200, 5000, 0x80000000);
            DrawBox(pDevice, 600, 0, 800, 5000, 0x80000000);
            RECT rx0 = { 2, 2, 0, 0 };
            RECT ry0 = { 50, 2, 0, 0 };
            RECT rz0 = { 100, 2, 0, 0 };
            RECT rx1 = { 600, 2, 0, 0 };
            RECT ry1 = { 650, 2, 0, 0 };
            RECT rz1 = { 700, 2, 0, 0 };
            pFont->DrawTextA(x0, -1, &rx0, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);
            pFont->DrawTextA(y0, -1, &ry0, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);
            pFont->DrawTextA(z0, -1, &rz0, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);
            pFont->DrawTextA(x1, -1, &rx1, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);
            pFont->DrawTextA(y1, -1, &ry1, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);
            pFont->DrawTextA(z1, -1, &rz1, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);
            

            RECT rect = { 400, 2, 0, 0 };
            char buffer[256];
            sprintf_s(buffer, "mNumActiveCoins: %hd\nmNumHUDFlying: %hd\nmNextInactiveCoin: %d\nprevious inactive coin: %d",
                pCoinManager->mNumActiveCoins,
                pCoinManager->mNumHUDFlying,
                pCoinManager->mNextInactiveCoin,
                nextInactiveCoinPrevious);
            pFont->DrawTextA(buffer, -1, &rect, DT_CALCRECT, 0xFFFFFFFF);

            DrawBox(pDevice, (float)rect.left - 5, (float)rect.top - 5,
                (float)(rect.right - rect.left) + 10,
                (float)(rect.bottom - rect.top) + 10, 0x80000000);
            pFont->DrawTextA(buffer, -1, &rect, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF);

            nextInactiveCoinPrevious = nextInactiveCoinLastFrame == pCoinManager->mNextInactiveCoin
                ? nextInactiveCoinPrevious : nextInactiveCoinLastFrame; 
            nextInactiveCoinLastFrame = pCoinManager->mNextInactiveCoin;

        }
    } while (false);

    // 5. Restore Z-Buffer so the game's 3D world doesn't look broken
    //pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

    return oEndScene(pDevice);
}

HRESULT STDMETHODCALLTYPE hkCreateDevice(void* pD3D, UINT Adp, DWORD Typ, HWND hW, DWORD Flg, void* pP, void** ppD) {
    HRESULT hr = oCreateDevice(pD3D, Adp, Typ, hW, Flg, pP, ppD);

    // 2. Now that the device exists (*ppD), hook its EndScene
    if (hr == D3D_OK && ppD != nullptr && *ppD != nullptr) {
        static bool hooked = false;
        if (!hooked) {
            IDirect3DDevice8* pDevice = (IDirect3DDevice8*)*ppD;
            void** vTable = *(void***)pDevice;

            if (MH_CreateHook(vTable[35], (LPVOID)&hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene)) == MH_OK) {
                MH_EnableHook(vTable[35]);
                //Beep(3000, 100); // This beep will sound once the device is ready!
                hooked = true;
            }
        }
    }
    return hr;
}

typedef IDirect3D8* (WINAPI* tDirect3DCreate8)(UINT);
extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    // 1. Load the real system DLL
    HMODULE hRealD3D8 = LoadLibraryA("C:\\Windows\\SysWOW64\\d3d8.dll");
    if (!hRealD3D8) return nullptr;

    // 2. Get the real function address
    tDirect3DCreate8 realCreate = (tDirect3DCreate8)GetProcAddress(hRealD3D8, "Direct3DCreate8");

    // 3. Call the real function to get the Direct3D object
    IDirect3D8* pD3D = realCreate(SDKVersion);
	if (!pD3D) return nullptr;
    void** vTable = *(void***)pD3D;

    // 4. This is the perfect time to hook! 
    // Since we have the pD3D object right here, we can hook its VTable immediately.
    if (oCreateDevice == nullptr) {
        if (MH_CreateHook(vTable[15], (LPVOID)&hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice)) == MH_OK) {
            MH_EnableHook(vTable[15]);
            //Beep(1000, 500); // 3rd Beep: Hooked at the exact moment of creation!
        }
    }

    return pD3D;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        //Beep(500, 100); // DLL Loaded Beep

        if (MH_Initialize() != MH_OK) return FALSE;
    }
    return TRUE;
}