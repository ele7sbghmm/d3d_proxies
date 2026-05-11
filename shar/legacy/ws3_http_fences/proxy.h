#pragma once

class proxy
{
public:
    using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
    Direct3DCreate8_t m_oDirect3DCreate8 = nullptr;
    HMODULE m_hModule = nullptr;

    void hook();
    IDirect3D8* call(std::uint32_t SDKVersion) const;
};
