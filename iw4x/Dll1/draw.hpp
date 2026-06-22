#pragma once

#include <d3dx9.h>

struct Vtx
{
    D3DVECTOR xyz;
    D3DCOLOR color;
    static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

class Drawer {
public:
    IDirect3DDevice9* m_device = nullptr;
    IDirect3DVertexBuffer9* m_vb = nullptr;
    IDirect3DStateBlock9* m_sb = nullptr;
    Vtx* m_vtx = nullptr;
    size_t m_n = 0;

    bool m_inited = false;

    static constexpr size_t MAX_VTX_N = 100000;

    Drawer() = default;
    Drawer(IDirect3DDevice9* device) : m_device(device) {}
    void Init()
    {
        if (m_inited) return;

        m_device->CreateVertexBuffer(MAX_VTX_N * sizeof(Vtx),
                                     D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                     Vtx::FVF,
                                     D3DPOOL_DEFAULT,
                                     &m_vb,
                                     nullptr);
        m_device->CreateStateBlock(D3DSBT_ALL, &m_sb);

        m_inited = true;
    }
    void Begin()
    {
        m_vtx = nullptr;
        size_t m_n = 0;

        m_vb->Lock(0, 0, (void**)m_vtx, D3DLOCK_NOOVERWRITE);
    }
    void Flush()
    {
        m_vb->Unlock();

        
    }
};

inline Drawer g_drawer{};
