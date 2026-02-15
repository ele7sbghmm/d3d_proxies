#include "pch.h"

#include "globals.h"
#include "types.h"

HWND CreateDebugWindow(int width, int height) {
    WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "DebugView", NULL };

    wc.lpszClassName = "DebugView";

    RegisterClassEx(&wc);

    return CreateWindow("DebugView", "Bird's Eye View", WS_OVERLAPPEDWINDOW, 100, 100, width, height, NULL, NULL, wc.hInstance, NULL);
}

bool Init(HWND hWnd) {
    IDirect3D8* pDirect3D = Direct3DCreate8(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS pPp = {};
    pPp.Windowed = TRUE;
    pPp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    //pPp.BackBufferFormat = D3DFMT_UNKNOWN;
    pPp.BackBufferFormat = D3DFMT_X8R8G8B8; // Standard 32-bit color
    pPp.BackBufferWidth = 0;
    pPp.BackBufferHeight = 0;

    HRESULT hr_CreateDevice = pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pPp, &Globals::ToolDevice);
    if (hr_CreateDevice < 0)
        return false;
    return true;
}

void RenderDebugScene() {
    if (!Globals::ToolDevice) return;

    HRESULT hr_Clear = Globals::ToolDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(20, 20, 20), 1.0f, 0);
    if (SUCCEEDED(Globals::ToolDevice->BeginScene())) {
        do {
            //break;
            float zoom = 50.f;
            D3DXMATRIX proj;
            D3DXMatrixOrthoLH(&proj, zoom, zoom, 1.f, 1000.f);
            Globals::ToolDevice->SetTransform(D3DTS_PROJECTION, &proj);

            D3DXVECTOR3* p = *(D3DXVECTOR3**)0x006c87a4;
            if (p == NULL) {
                Globals::ToolDevice->EndScene();
                break;
            }

            D3DXMATRIX view;
            //D3DXVECTOR3 eye(p->x, 500.f, p->z);
            //D3DXVECTOR3 at(p->x, 0.f, p->z);
            D3DXVECTOR3 eye(0.f, 500.f, 0.f);
            D3DXVECTOR3 at(0.f, 0.f, 0.f);
            D3DXVECTOR3 up(0.f, 0.f, 1.f);
            D3DXMatrixLookAtLH(&view, &eye, &at, &up);
            Globals::ToolDevice->SetTransform(D3DTS_VIEW, &view);

            VertexColor verts[2] = {
                { p->x, 0.f, p->z, 0xffffffff },
                { 0.f, 0.f, 0.f, 0xffffffff }
            };
            Globals::ToolDevice->SetTexture(0, NULL);
            Globals::ToolDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
            Globals::ToolDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            Globals::ToolDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE);
            Globals::ToolDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, verts, sizeof(VertexColor));

            //DrawWorldGrid();
            //DrawPlayerMarker(p);
        } while (false);

        Globals::ToolDevice->EndScene();
    }
    HRESULT hr_Present = Globals::ToolDevice->Present(NULL, NULL, NULL, NULL);
}

void DebugWindowThread() {
    Sleep(500);

    HWND hwnd = CreateDebugWindow(800, 600);
    HWND sharHWnd = FindWindowA(NULL, "The Simpsons Hit & Run");

    if (Init(hwnd)) {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(sharHWnd);
        UpdateWindow(hwnd);
        MSG msg = { 0 };
        // This loop runs as long as the window hasn't been closed
        while (msg.message != WM_QUIT) {

            // 1. CHECK FOR MESSAGES (Non-blocking)
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {
                // 2. NO MESSAGES? RENDER!
                // This runs every single frame, as fast as possible.
                RenderDebugScene();

                // 3. Optional: Cap the CPU usage slightly
                // Without this, one CPU core will stay at 100%
                Sleep(1);
            }
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DebugWindowThread, NULL, 0, NULL);

        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "w", stdin);

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
