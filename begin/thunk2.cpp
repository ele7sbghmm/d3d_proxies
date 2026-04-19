#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sstream>
#include <d3dx8.h>
#include "include/MinHook.h"

#include "worldsim/coins/coinmanager.h"
#include "camera/supercamcentral.h"
#include "p3d/pointcamera.hpp"
#include "worldsim/avatarmanager.h"
#include "worldsim/redbrick/vehicle.h"

#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "./libs/dx8/d3d8.lib")
#pragma comment(lib, "./libs/dx8/d3dx8.lib")
#pragma comment(lib, "./libs/libMinHook.x86.lib")


IDirect3DDevice8* pDevice = nullptr;
// 3. The Hook Function
typedef HRESULT(STDMETHODCALLTYPE* tCreateDevice)(void*, UINT, DWORD, HWND, DWORD, void*, void**);
tCreateDevice oCreateDevice = nullptr;

typedef HRESULT(STDMETHODCALLTYPE* tEndScene)(IDirect3DDevice8*);
tEndScene oEndScene = nullptr;

struct CustomVertex {
    float x, y, z, rhw;
    DWORD color;
};
struct ColorVertex {
    float x, y, z;
    DWORD color;
};
namespace CameraGlobals {
    D3DXMATRIX view;
    D3DXMATRIX projection;
    D3DXMATRIX viewProjection;
    int screenWidth = 800;
    int screenHeight = 600;
    void Update(SuperCam* asc) {
        if (!asc || !asc->mCamera || pDevice == nullptr) return;

        enum Flag { TRANSITION = 3, SHAKE = 9 };
        float fov = ((asc->mFlags & (1 << TRANSITION)) > 0 || (asc->mFlags & (1 << SHAKE)) > 0)
            ? asc->mCameraVirtualFOV : asc->mSCFOV;
        //float aspect = asc->mSCAspect;
        float aspect = (float)CameraGlobals::screenWidth / (float)CameraGlobals::screenHeight;

        D3DXVECTOR3 eye = reinterpret_cast<D3DXVECTOR3&>(asc->mCamera->position);
        D3DXVECTOR3 at = reinterpret_cast<D3DXVECTOR3&>(asc->mCamera->target);
        //D3DXVECTOR3 up = reinterpret_cast<D3DXVECTOR3&>(pc->vup);
        D3DXVECTOR3 up(0.f, 1.f, 0.f);
        D3DXVECTOR3 delta = eye - at;
        if (D3DXVec3LengthSq(&delta) < 0.001f) return;

        CameraGlobals::view = reinterpret_cast<D3DXMATRIX&>(asc->mCamera->worldToCamera);
        D3DXMatrixInverse(&CameraGlobals::view, NULL, (D3DXMATRIX*)&asc->mCamera->worldToCamera);
        //D3DXMatrixTranspose(&CameraGlobals::view, &CameraGlobals::view);
        //D3DXMatrixLookAtRH(&view, &eye, &at, &up);
        D3DXMatrixPerspectiveFovRH(&projection, fov, aspect, asc->mSCNearPlane, asc->mSCFarPlane);

        //viewProjection = view * projection;
        D3DXMatrixMultiply(&CameraGlobals::viewProjection, &CameraGlobals::view, &CameraGlobals::projection);

        //D3DXMATRIX viewToSet, projToSet;
        //D3DXMatrixTranspose(&viewToSet, &view);
        //D3DXMatrixTranspose(&projToSet, &projection);

        //pDevice->SetTransform(D3DTS_VIEW, &viewToSet);
        //pDevice->SetTransform(D3DTS_PROJECTION, &projToSet);
    }
}


// This "Flexible Vertex Format" (FVF) tells D3D what data we are sending
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
#define D3DFVF_COLORVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

//extern "C" void* ?GetPosition@SuperCam@@QBEXPAVVector@RadicalMathLibrary@@@Z = (void*)0x0045A120;


void DrawBox(float x, float y, float w, float h, DWORD color) {
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

    pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CustomVertex));
}

void* drawText(LPD3DXFONT pFont, int x, int y, DWORD color, const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
    va_end(args);

    RECT rect{}; rect.left = x; rect.top = y;
    pFont->DrawTextA(buffer, -1, &rect, DT_CALCRECT, color);
    
    float w = (float)(rect.right - rect.left);
    float h = (float)(rect.bottom - rect.top);
    DrawBox((float)x, float(y), w, h, 0x88000000);
    pFont->DrawTextA(buffer, -1, &rect, DT_NOCLIP, color);

    return {};
}

bool worldToScreen(D3DXVECTOR3 worldPos, D3DXVECTOR2& screenPos) {
    D3DVIEWPORT8 vp;
    pDevice->GetViewport(&vp);

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    
    // Use the D3DX function to ensure proper math
    D3DXVECTOR3 out;
    D3DXVec3Project(&out, &worldPos, &vp, &CameraGlobals::projection, &CameraGlobals::view, &identity);

    // If W is 1.0 and X/Y are just the input X/Y, the matrix is Identity (empty)
    if (out.z < 0.f || out.z >= 1.f) return false;

    // This converts NDC (-1 to 1) to Screen Pixels (0 to 800)
    screenPos.x = out.x;
    screenPos.y = out.y;

    return true;
}

void DrawLine2D(D3DXVECTOR2 start, D3DXVECTOR2 end, DWORD color) {
    CustomVertex vertices[2] = {
        { start.x, start.y, .5f, 1.f, color },
        { end.x, end.y, .5f, 1.f, color }
    };

    pDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);
    pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(CustomVertex));
}
//void DrawLine3D(D3DXVECTOR3 start, D3DXVECTOR3 end, D3DCOLOR color) {
//    D3DXVECTOR2 s, e;
//    if (worldToScreen(start, s) && worldToScreen(end, e))
//        DrawLine2D(s, e, color);
//}
void DrawLine3D(D3DXVECTOR3 start, D3DXVECTOR3 end, D3DCOLOR color) {
    ColorVertex vertices[2] = {
        { start.x, start.y, start.z, color },
        { end.x, end.y, end.z, color }
    };

    // 1. Tell D3D we are using WORLD coordinates (XYZ), not pixels (RHW)
    pDevice->SetVertexShader(D3DFVF_COLORVERTEX);

    // 2. Load the matrices we stole from the engine
    pDevice->SetTransform(D3DTS_VIEW, &CameraGlobals::view);
    pDevice->SetTransform(D3DTS_PROJECTION, &CameraGlobals::projection);

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    pDevice->SetTransform(D3DTS_WORLD, &identity);

    // 3. Enable Depth Testing so it doesn't "sit on top"
    pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    pDevice->SetTexture(0, NULL);

    pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(ColorVertex));
}


static const char* getCamName(int index) {
    switch (index) {
        case 0: return "FollowCamNear";
        case 1: return "FollowCamFar";
        case 2: return "BumperCam";
        case 3: return "ChaseCam";
        case 4: return "KullCam";
        case 5: return "DebugCam";
        case 6: return "TrackerCam";
        case 7: return "WalkerCam";
        case 8: return "ComedyCam";
        case 9: return "WrecklessCam";
        case 10: return "ConversationCam";
        case 11: return "ReverseCam";
        case 12: return "AnimationCam";
        case 13: return "RelativeAnimatedCam";
        case 14: return "PCCam";
        case 15: return "StaticCam";
        default: return "no name";
    }
};


CoinManager** ppCoinManager = (CoinManager**)0x6c8450;
SuperCamCentral** ppSuperCamCentral = (SuperCamCentral**)0x6c906c;
AvatarManager** ppAvatarManager = (AvatarManager**)0x6c84e4;


LPD3DXFONT pFont{};
HFONT hFontGDI{};
D3DSURFACE_DESC desc{};
IDirect3DSurface8* pBackBuffer{};
static ULONGLONG lastPrint{};
static ULONGLONG lastSlowClear{};
HRESULT STDMETHODCALLTYPE hkEndScene(IDirect3DDevice8* pDevicePtr) {    // 1. Re-initialize font if it was lost (due to Reset)
    pDevice = pDevicePtr;

    if (pFont == nullptr) {
        hFontGDI = CreateFontA(20, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        D3DXCreateFont(pDevice, hFontGDI, &pFont);
    }

    // 2. Force the render states every single frame
    // These are the most important for "Overlay" drawing
    //pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);          // Disable Depth Testing (Stay on top)
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

    // Reset World matrix to Identity (so 0,0,0 is the actual center of the map)
 
    //DrawWorldLine(pDevice, 229.f, -1000.0f, -177.f, 229.f, 1000.0f, -177.f, 0xFF0000FF);
    //DrawWorldLine(pDevice, 0, 0, 0, 0, 0, 100, 0xFFFF0000);
    //DrawLine(0.f, 0.f, 800.f, 600.f, 0xFFFF0000);

    D3DVIEWPORT8 vp;
    pDevice->GetViewport(&vp);
    CameraGlobals::screenWidth = vp.Width;
    CameraGlobals::screenHeight = vp.Height;

    do {
        D3DMATRIX oldView, oldProj, oldWorld;
        pDevice->GetTransform(D3DTS_VIEW, &oldView);
        pDevice->GetTransform(D3DTS_PROJECTION, &oldProj);
        pDevice->GetTransform(D3DTS_WORLD, &oldWorld);

        if (ppSuperCamCentral == nullptr
            || ppAvatarManager == nullptr) break;

        if (*ppSuperCamCentral == nullptr
            || *ppAvatarManager == nullptr
            || (*ppAvatarManager)->mAvatarArray[0] == nullptr) break;

        AvatarManager* avm = *ppAvatarManager;
        Avatar* av = avm->mAvatarArray[0];
        char vhpb[256] = "null";
        if (av->mpVehicle != nullptr) {
            Vehicle* vh = av->mpVehicle;
            rmt::Vector vhp = *(rmt::Vector*)vh->mTransform.m[3];
            sprintf_s(vhpb, -1, "%.2f %.2f %2.f", vhp.x, vhp.y, vhp.z);
        }
        Character* ch = av->mpCharacter;


        //SuperCam* pSuperCam = *(SuperCam**)((uintptr_t)*ppSuperCamCentral + 0x0c);
        SuperCamCentral* scc = *ppSuperCamCentral;
        SuperCam* asc = scc->mActiveSuperCam;
        if (asc == nullptr || asc->mCamera == nullptr) break;
        CameraGlobals::Update(asc);

        D3DXMATRIX world{};
        D3DXMatrixIdentity(&world);
        pDevice->SetTransform(D3DTS_VIEW, &CameraGlobals::view);
        pDevice->SetTransform(D3DTS_PROJECTION, &CameraGlobals::projection);
        pDevice->SetTransform(D3DTS_WORLD, &world);

        DrawLine3D(D3DXVECTOR3(229.f, -1000.0f, -177.f), D3DXVECTOR3(229.f, 1000.0f, -177.f), 0xFF0000FF);

        tPointCamera* pc = asc->mCamera;
        if (pc == nullptr) break;

        drawText(pFont, 0, 200, 0xffffffff,
            "vehicle: %s\n\n%s %d\neye: %.2f %.2f %.2f\nat: %.2f %.2f %.2f\nvup: %.2f %.2f %.2f\nfov: %f\naspect: %f\nnearZ: %f\nfarZ: %f\n",
            vhpb, getCamName(scc->mActiveSuperCamIndex), scc->mActiveSuperCamIndex, pc->position.x, pc->position.y, pc->position.z, pc->target.x, pc->target.y, pc->target.z, pc->vup.x, pc->vup.y, pc->vup.z, asc->mSCFOV, asc->mSCAspect, asc->mSCNearPlane, asc->mSCFarPlane);

        pDevice->SetTransform(D3DTS_VIEW, &oldView);
        pDevice->SetTransform(D3DTS_PROJECTION, &oldProj);
        pDevice->SetTransform(D3DTS_WORLD, &oldWorld);


        if (ppCoinManager != nullptr
            && (*ppCoinManager)->mActiveCoins != nullptr) {

            ULONGLONG currentTime = GetTickCount64();
            if (currentTime - lastSlowClear > 10000) {
                lastSlowClear = currentTime;
            }
            if (currentTime - lastPrint > 50) {
                lastPrint = currentTime;
            }
        }
    } while (false);

    // 5. Restore Z-Buffer so the game's 3D world doesn't look broken
    //pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

    return oEndScene(pDevice);
}

HRESULT STDMETHODCALLTYPE hkCreateDevice(void* pD3D, UINT Adp, DWORD Typ, HWND hW, DWORD Flg, void* pP, void** ppD) {
    HRESULT hr = oCreateDevice(pD3D, Adp, Typ, hW, Flg, pP, ppD);

    if (hr == D3D_OK && ppD != nullptr && *ppD != nullptr) {
        static bool hooked = false;
        if (!hooked) {
            pDevice = (IDirect3DDevice8*)*ppD;
            void** vTable = *(void***)pDevice;

            if (MH_CreateHook(vTable[35], (LPVOID)&hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene)) == MH_OK) {
                MH_EnableHook(vTable[35]);
                hooked = true;
            }
        }
    }
    return hr;
}

typedef IDirect3D8* (WINAPI* tDirect3DCreate8)(UINT);
extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    HMODULE hRealD3D8 = LoadLibraryA("C:\\Windows\\SysWOW64\\d3d8.dll");
    if (!hRealD3D8) return nullptr;

    tDirect3DCreate8 realCreate = (tDirect3DCreate8)GetProcAddress(hRealD3D8, "Direct3DCreate8");

    IDirect3D8* pD3D = realCreate(SDKVersion);
	if (!pD3D) return nullptr;
    void** vTable = *(void***)pD3D;

    if (oCreateDevice == nullptr) {
        if (MH_CreateHook(vTable[15], (LPVOID)&hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice)) == MH_OK) {
            MH_EnableHook(vTable[15]);
        }
    }

    return pD3D;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        if (MH_Initialize() != MH_OK) return FALSE;

        //AllocConsole();
        //FILE* f;
        //freopen_s(&f, "CONOUT$", "w", stdout); // Redirect stdout to the console
        //printf("Debug Console Initialized\n");
    }
    return TRUE;
}
