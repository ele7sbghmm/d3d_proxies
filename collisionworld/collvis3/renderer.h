#pragma once

#include <wrl/client.h>
#include <cstddef>
#include <cstdint>
#include <d3dx8.h>
#include "shar.h"

using Microsoft::WRL::ComPtr;

struct Vtx {
  D3DVECTOR xyz, normal;
  D3DCOLOR diffuse, specular;
  inline static constexpr DWORD FVF
   = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
};

class Renderer {
public:
  Renderer() = default;
  Renderer(IDirect3DDevice8* device) : m_device(device) {}

  void Init();
  void Begin();
  void End();
  void Bake(shar::SpatialNode*, shar::FenceEntityDSG*, D3DVECTOR*, float*, bool, std::uint32_t);
  void Draw();

private:
  inline static constexpr std::size_t MAX_VTX = 2000000;
  inline static constexpr std::size_t MAX_IDX = MAX_VTX;
  inline static constexpr std::size_t LINE_OFFSET = MAX_VTX / 2;
  
  ComPtr<IDirect3DDevice8> m_device;
  ComPtr<IDirect3DVertexBuffer8> m_vb;
  ComPtr<IDirect3DIndexBuffer8> m_ib;
  DWORD m_state_block_token = NULL;

  Vtx* m_vtx_data = nullptr;
  std::uint16_t* m_idx_data = nullptr;
  std::size_t m_vtx_cursor = 0;
  std::size_t m_idx_cursor = 0;
  std::size_t m_line_vtx_cursor = LINE_OFFSET;
  std::size_t m_line_idx_cursor = LINE_OFFSET;

  bool m_initialized = false;
  bool m_well_begun = false;
};

void Renderer::Init() {
  m_device->CreateVertexBuffer(MAX_VTX * sizeof(Vtx),
    D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
    Vtx::FVF,
    D3DPOOL_DEFAULT,
    &m_vb);
  m_device->CreateIndexBuffer(MAX_IDX * sizeof(std::uint16_t),
    D3DUSAGE_WRITEONLY,
    D3DFMT_INDEX16,
    D3DPOOL_DEFAULT,
    &m_ib);

  m_device->CreateStateBlock(D3DSBT_ALL, &m_state_block_token);

  m_initialized = true;
}
void Renderer::Begin() {
  if (!m_initialized) Init();

  m_vtx_cursor = 0;
  m_idx_cursor = 0;
  m_line_vtx_cursor = 0;
  m_line_idx_cursor = 0;

  m_vb->Lock(0, 0, (BYTE**)&m_vtx_data, D3DLOCK_DISCARD);
  m_ib->Lock(0, 0, (BYTE**)&m_idx_data, D3DLOCK_DISCARD);

  m_well_begun = true;
}
void Renderer::End() {
  m_vb->Unlock();
  m_ib->Unlock();

  m_well_begun = false;

  Draw();
}
void Renderer::Draw() {
  m_device->CaptureStateBlock(m_state_block_token);

  m_device->SetStreamSource(0, m_vb.Get(), sizeof(Vtx));
  m_device->SetIndices(m_ib.Get(), 0);
  m_device->SetVertexShader(Vtx::FVF);

  m_device->SetTexture(0, NULL);
  m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_vtx_cursor, 0, m_idx_cursor / 3);

  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_device->DrawIndexedPrimitive(D3DPT_LINELIST, 0, m_line_vtx_cursor, LINE_OFFSET, m_line_idx_cursor / 2);

  m_device->ApplyStateBlock(m_state_block_token);
}
void Renderer::Bake(shar::SpatialNode* node, shar::FenceEntityDSG* fence, D3DVECTOR* position, float* radius, bool in_range, std::uint32_t area_index) {
  if (area_index) return;
  if (!m_well_begun) Begin();

  D3DCOLOR color = in_range ? 0xffff0000 : 0xffffffff;
  std::size_t vi = m_vtx_cursor;
  std::size_t ii = m_idx_cursor;
  std::size_t lvi = m_line_vtx_cursor;
  std::size_t lii = m_line_idx_cursor;

  float top = position->y + 5.f;
  float bot = position->y - 0.f;
  const D3DVECTOR& s = fence->mStartPoint;
  const D3DVECTOR& e = fence->mEndPoint;
  const D3DVECTOR& n = fence->mNormal;

  m_idx_data[ii++] = vi + 0;
  m_idx_data[ii++] = vi + 1;
  m_idx_data[ii++] = vi + 2;
  m_idx_data[ii++] = vi + 2;
  m_idx_data[ii++] = vi + 3;
  m_idx_data[ii++] = vi + 0;

  m_vtx_data[vi++] = { { s.x, top, s.z}, n, color, color };
  m_vtx_data[vi++] = { { s.x, bot, s.z}, n, color, color };
  m_vtx_data[vi++] = { { e.x, bot, e.z}, n, color, color };
  m_vtx_data[vi++] = { { e.x, top, e.z}, n, color, color };

  D3DXVECTOR3 center = s;
  center += e;
  center.y = 0.f;
  center *= .5;
  D3DXVECTOR3 scratch = s;
  scratch.y = position.y
  scratch -=

  std::size_t sides = 36;
  for (std::int32_t i = 0; i <= sides; ++i) {
    float theta = (D3DX_PI * 2 / i);

    if (i == 0 || i == sides) {
      m_vtx_data[lvi++] = {{}}
    }
  }

  m_vtx_cursor = vi;
  m_idx_cursor = ii;
}

inline Renderer g_renderer;
