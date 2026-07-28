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
        if (!m_inited)
            Init();
        m_vtx = nullptr;
        m_n = 0;

        m_vb->Lock(0, 0, (void**)&m_vtx, D3DLOCK_DISCARD);

        m_vtx[m_n++] = { { 0, -1000, 100 }, 0xffff0000 };
        m_vtx[m_n++] = { { 0,  1000, 100 }, 0xffff0000 };
        m_vtx[m_n++] = { { -1000, 0, 100 }, 0xffff0000 };
        m_vtx[m_n++] = { {  1000, 0, 100 }, 0xffff0000 };
    }
    void Flush()
    {
        m_vb->Unlock();

        m_sb->Capture();
        m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
        m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
        m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

        m_device->SetTexture(0, nullptr);
        m_device->SetVertexShader(nullptr);
        m_device->SetPixelShader(nullptr);
        m_device->SetFVF(Vtx::FVF);
        m_device->SetStreamSource(0, m_vb, 0, sizeof(Vtx));
        m_device->DrawPrimitive(D3DPT_LINELIST, 0, m_n / 2);
        m_sb->Apply();
    }
};

inline Drawer g_Drawer{};
