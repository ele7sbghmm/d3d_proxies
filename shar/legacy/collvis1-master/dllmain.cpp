// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "collvis.h"

void draw() {
    g_context.CommitDraw();
}

void gui() {
    if (!g_context.m_inited) return;

    ImGui_ImplDX8_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::GetIO().MouseDrawCursor = true;

    char buf[256];
    ImGui::Begin("##begin");
    ImGui::BeginTabBar("##begintabbar");
    if (ImGui::BeginTabItem("stats")) {
        float* pos = reinterpret_cast<float*>(&g_context.m_player_position);
        sprintf_s(buf, "position %+.2f %+.2f %+.2f", pos[0], pos[1], pos[2]);
        ImGui::Text(buf);
        sprintf_s(buf, "radius %.2f", g_context.m_player_radius);
        ImGui::Text(buf);

        sprintf_s(buf, "fences %d", g_context.m_collidable_fence_count);
        ImGui::Text(buf);
        sprintf_s(buf, "statics %d", g_context.m_collidable_statics_count);
        ImGui::Text(buf);

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("config")) {
        ImGui::Checkbox("draw world spheres", &g_context.m_config.draw_world_sphere);
        ImGui::Checkbox("draw opaque", &g_context.m_config.draw_opaque);
        ImGui::Checkbox("draw simple shadows", &g_context.m_config.draw_simple_shadows);
        ImGui::Checkbox("draw translucent", &g_context.m_config.draw_translucent);

        ImGui::Checkbox("draw passive nodes", &g_context.m_config.draw_passive_nodes);
        ImGui::Checkbox("draw active nodes", &g_context.m_config.draw_active_nodes);
        ImGui::Checkbox("draw fences", &g_context.m_config.draw_fences);
        ImGui::Checkbox("draw statics", &g_context.m_config.draw_statics);
        ImGui::Checkbox("draw dynamics", &g_context.m_config.draw_dynamics);
        ImGui::Checkbox("draw intersects", &g_context.m_config.draw_intersects);

        ImGui::SeparatorText("Update");
        ImGui::Checkbox("position", &g_context.m_config.update_position);
        ImGui::Checkbox("y", &g_context.m_config.update_y);
        ImGui::Checkbox("radius", &g_context.m_config.update_radius);
        ImGui::DragFloat("fence height", &g_context.m_config.fence_height, 1.f, 0.f, 100.f, "%.2f");
        ImGui::DragFloat("fence y offset", &g_context.m_config.fence_y_offset, 1.f, -100.f, 100.f, "%.2f");

        ImGui::Checkbox("custom radius", &g_context.m_config.use_custom_radius);
        ImGui::DragFloat("##custom_radius_slider", &g_context.m_config.custom_radius, .01f, 0.f, 100.f, "%.2f");

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("color")) {
        ImGui::ColorEdit4("passive", (float*)&g_context.m_config.color_passive);
        ImGui::ColorEdit4("active", (float*)&g_context.m_config.color_active);
        ImGui::ColorEdit4("collidable", (float*)&g_context.m_config.color_collidable);
        ImGui::ColorEdit4("overflow", (float*)&g_context.m_config.color_overflow);

        ImGui::ColorEdit4("statics passive", (float*)&g_context.m_config.color_statics_passive);
        ImGui::ColorEdit4("statics active", (float*)&g_context.m_config.color_statics_active);
        ImGui::ColorEdit4("statics collidable", (float*)&g_context.m_config.color_statics_collidable);

        ImGui::SeparatorText("terrain");
        for (std::size_t i = 0; i < 8; ++i) {
            auto& tt = g_context.m_config.color_terrain_palette[i];
            ImGui::ColorEdit4(tt.name, (float*)&tt.color);
        }

        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
}

#define DECLARE_STUB_HOOK(addr, name, toggle_ptr) \
    namespace name { \
	    bool* toggle = toggle_ptr; \
	    void* address = (void*)addr; \
	    void* original = nullptr; \
	    __declspec(naked) void hook() { \
		    __asm mov eax, toggle \
		    __asm cmp byte ptr[eax], 0 \
		    __asm je Skip \
		    __asm jmp original \
		    __asm Skip: \
		    __asm ret \
	    } \
    }
namespace Hooks {
    DECLARE_STUB_HOOK(0x4a6180, WorldSphereDSG_Display, &g_context.m_config.draw_world_sphere);
    DECLARE_STUB_HOOK(0x49cb40, WorldScene_RenderOpaque, &g_context.m_config.draw_opaque);
    DECLARE_STUB_HOOK(0x49cab0, WorldScene_RenderSimpleShadows, &g_context.m_config.draw_simple_shadows);
    DECLARE_STUB_HOOK(0x49cbd0, WorldScene_RenderTransparent, &g_context.m_config.draw_translucent);

    namespace SuperCam_SetActiveSuperCam {
        void* address = reinterpret_cast<void*>(0x4298f0);
        void* original = nullptr;
        __declspec(naked) void hook() {
            __asm cmp ecx, 13 // rail cam
            __asm je SetPCCam
            __asm cmp ecx, 15 // static cam
            __asm je SetPCCam
            __asm cmp ecx, 16 // burnout cam
            __asm je SetPCCam
            __asm jmp original

            __asm SetPCCam:
            __asm mov ecx, 25 // pc cam
            __asm jmp original
        }
    };

    void Inject() {
        MH_CreateHook(
            WorldSphereDSG_Display::address,
            (void*)WorldSphereDSG_Display::hook,
            reinterpret_cast<void**>(&WorldSphereDSG_Display::original)
        );
        MH_CreateHook(
            WorldScene_RenderOpaque::address,
            (void*)WorldScene_RenderOpaque::hook,
            reinterpret_cast<void**>(&WorldScene_RenderOpaque::original)
        );
        MH_CreateHook(
            WorldScene_RenderSimpleShadows::address,
            (void*)WorldScene_RenderSimpleShadows::hook,
            reinterpret_cast<void**>(&WorldScene_RenderSimpleShadows::original)
        );
        MH_CreateHook(
            WorldScene_RenderTransparent::address,
            (void*)WorldScene_RenderTransparent::hook,
            reinterpret_cast<void**>(&WorldScene_RenderTransparent::original)
        );
        MH_CreateHook(
            SuperCam_SetActiveSuperCam::address,
            (void*)SuperCam_SetActiveSuperCam::hook,
            reinterpret_cast<void**>(&SuperCam_SetActiveSuperCam::original)
        );

        MH_EnableHook(MH_ALL_HOOKS);
    }
};

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT __stdcall hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

void* addrRenderTranslucent = (void*)0x49cbd0;
void* oriRenderTranslucent = nullptr;
__declspec(naked) void hkRenderTranslucent() {
    __asm pushad
    __asm call draw
    __asm popad
    __asm jmp oriRenderTranslucent
}

decltype(&IDirect3DDevice8::EndScene) oEndScene = nullptr;
HRESULT WINAPI hkEndScene(IDirect3DDevice8* _this) {
    gui();

    return (_this->*oEndScene)();
}

decltype(&IDirect3D8::CreateDevice) oCreateDevice = nullptr;
HRESULT WINAPI hkCreateDevice(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
    HRESULT hr = (_this->*oCreateDevice)(Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    if (!ppReturnedDeviceInterface || !*ppReturnedDeviceInterface)
        return hr;

    void** vftable = *(void***)*ppReturnedDeviceInterface;
    MH_CreateHook(vftable[35], (void*)hkEndScene, reinterpret_cast<void**>(&oEndScene));
    MH_EnableHook(vftable[35]);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX8_Init(*ppReturnedDeviceInterface);
    oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

    return hr;
}

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    char path[MAX_PATH];
    GetSystemDirectory(path, MAX_PATH);
    strcat_s(path, "\\d3d8.dll");
    HMODULE hModule = LoadLibrary(path);
    if (!hModule) throw;

    typedef IDirect3D8*(WINAPI* tDirect3DCreate8)(UINT);
    tDirect3DCreate8 oDirect3DCreate8 = (tDirect3DCreate8)GetProcAddress(hModule, "Direct3DCreate8");
    IDirect3D8* pD3D = oDirect3DCreate8(SDKVersion);
    if (!pD3D) return pD3D;

    void** vftable = *(void***)pD3D;

    MH_Initialize();
    MH_CreateHook(vftable[15], (void*)hkCreateDevice, reinterpret_cast<void**>(&oCreateDevice));
    MH_EnableHook(vftable[15]);
    MH_CreateHook(addrRenderTranslucent,
        (void*)hkRenderTranslucent,
        reinterpret_cast<void**>(&oriRenderTranslucent));
    MH_EnableHook(addrRenderTranslucent);

    Hooks::Inject();

    return pD3D;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

