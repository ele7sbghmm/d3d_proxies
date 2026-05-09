#include "pch.h"

#include "server.h"

void server::serve_html(SOCKET client)
{
    HMODULE hModule{};
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&hModule,
        &hModule
    );
    HRSRC res = FindResource(hModule, MAKEINTRESOURCE(IDR_HTML), "HTML");
    HGLOBAL handle = LoadResource(hModule, res);
    void* data = LockResource(handle);
    DWORD size = SizeofResource(hModule, res);

    send(client, (char*)data, size, 0);
}