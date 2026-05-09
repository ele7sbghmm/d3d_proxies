#include "pch.h"

#include <thread>

#include "proxy.h"
#include "server.h"

void proxy::hook()
{
    if (m_oDirect3DCreate8) return;

    if (!m_hModule) {
        char path[MAX_PATH];
        GetSystemDirectory(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        if (m_hModule = LoadLibrary(path); m_hModule == nullptr)
            throw std::nullptr_t();
    }
    if (m_oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(m_hModule, "Direct3DCreate8"); m_oDirect3DCreate8 == nullptr)
        throw std::nullptr_t();
}

IDirect3D8* proxy::call(std::uint32_t SDKVersion) const
{
    if (!m_oDirect3DCreate8) return nullptr;

    IDirect3D8* d3d = m_oDirect3DCreate8(SDKVersion);
    if (!d3d) return nullptr;

    server s{};
    std::thread(s.run).detach();

    return d3d;
}

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    proxy p{};
    p.hook();
    return p.call(SDKVersion);
}
