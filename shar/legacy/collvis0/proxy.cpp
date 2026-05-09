#include "pch.h"

#include "mod.h"
#include "hooks.h"
#include "gui.h"

WNDPROC o_WndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall hk_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	return CallWindowProc(o_WndProc, hWnd, msg, wParam, lParam);
}

using t_CreateDevice = HRESULT(WINAPI*)(PDIRECT3D8, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, PDIRECT3DDEVICE8*);

t_CreateDevice o_CreateDevice = nullptr;

HRESULT WINAPI hk_CreateDevice(PDIRECT3D8 _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, PDIRECT3DDEVICE8* ppReturnedDeviceInterface) {
	HRESULT hr = o_CreateDevice(_this, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (FAILED(hr) || !ppReturnedDeviceInterface || !*ppReturnedDeviceInterface)
		return hr;

	void** vtable = *(void***)*ppReturnedDeviceInterface;

	inject_hooks();

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX8_Init(*ppReturnedDeviceInterface);

	g_collviscontext = CollVisContext(*ppReturnedDeviceInterface);

	o_WndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)hk_WndProc);

	return hr;
}

extern "C" PDIRECT3D8 WINAPI _Direct3DCreate8(UINT SDKVersion) {
	char path[MAX_PATH];
	GetSystemDirectory(path, MAX_PATH);
	strcat_s(path, "\\d3d8.dll");
	HMODULE hModule = LoadLibrary(path);

	using t_Direct3DCreate8 = PDIRECT3D8(WINAPI*)(UINT SDKVersion);
	t_Direct3DCreate8 o_Direct3DCreate8 = (t_Direct3DCreate8)GetProcAddress(hModule, "Direct3DCreate8");
	PDIRECT3D8 d3d = o_Direct3DCreate8(SDKVersion);

	if (!d3d || o_CreateDevice) return nullptr;
	void** vtable = *(void***)d3d;

	MH_Initialize();
	if (MH_CreateHook(vtable[15], (void*)hk_CreateDevice, reinterpret_cast<void**>(&o_CreateDevice)) != MH_OK)
		throw;
	MH_EnableHook(vtable[15]);

	return d3d;
}
