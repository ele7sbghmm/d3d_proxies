#pragma once

#include <wrl/client.h>
#include <cstddef>
#include <cstdint>
#include <vector>
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
  void Bake(shar::FenceEntityDSG*, float, float, D3DVECTOR*, int);
  void Draw();

private:
  inline static constexpr std::size_t MAX_VTX = 2000000;
  inline static constexpr std::size_t LINE_OFFSET = MAX_VTX / 2;
  inline static constexpr std::size_t MAX_COLLIDABLE = 8;
  
  ComPtr<IDirect3DDevice8> m_device;
  ComPtr<IDirect3DVertexBuffer8> m_vb;
  DWORD m_state_block_token = NULL;

  Vtx* m_vtx_data = nullptr;
  std::size_t m_vtx_cursor = 0;
  std::size_t m_line_vtx_cursor = LINE_OFFSET;
  std::size_t m_area_index = 0;
  std::size_t m_area_index_collidable_fences_count = 0;

  bool m_initialized = false;
  bool m_well_begun = false;
};

void Renderer::Init() {
  m_device->CreateVertexBuffer(MAX_VTX * sizeof(Vtx),
    D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
    Vtx::FVF,
    D3DPOOL_DEFAULT,
    &m_vb);

  m_device->CreateStateBlock(D3DSBT_ALL, &m_state_block_token);

  m_initialized = true;
}
void Renderer::Begin() {
  if (!m_initialized) Init();

  m_vtx_cursor = 0;
  m_line_vtx_cursor = 0;
  m_area_index_collidable_fences_count = 0;

  m_vb->Lock(0, 0, (BYTE**)&m_vtx_data, D3DLOCK_NOOVERWRITE);

  m_well_begun = true;
}
void Renderer::End() {
  m_vb->Unlock();

  m_well_begun = false;

  Draw();
}
void Renderer::Draw() {
  m_device->CaptureStateBlock(m_state_block_token);

  m_device->SetStreamSource(0, m_vb.Get(), sizeof(Vtx));
  m_device->SetVertexShader(Vtx::FVF);

  D3DMATERIAL8 mat{};
  mat.Ambient = { 1.f, 1.f, 1.f, 0.f };
  m_device->SetMaterial(&mat);
  m_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);

  m_device->SetTexture(0, NULL);
  m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
  m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
  m_device->SetRenderState(D3DRS_COLORVERTEX,TRUE);
  m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_vtx_cursor / 3);

  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_device->DrawPrimitive(D3DPT_LINELIST, 0, m_line_vtx_cursor / 2);

  m_device->ApplyStateBlock(m_state_block_token);
}
void Renderer::Bake(shar::FenceEntityDSG* fence, float range, float distance_to, D3DVECTOR* position, int area_index) {
  if (area_index != m_area_index) return;
  bool in_range =  distance_to < range;
  if (in_range) m_area_index_collidable_fences_count++;

  if (!m_well_begun) Begin();

  static constexpr D3DCOLOR RED = 0x40ff0000, WHITE = 0x40c0c0c0, GREEN = 0x4000ff00;
  D3DCOLOR color = !in_range ? WHITE
    : m_area_index_collidable_fences_count > MAX_COLLIDABLE
      ? GREEN : RED;
  std::size_t vi = m_vtx_cursor;
  std::size_t lvi = m_line_vtx_cursor;

  float top = position->y + 5.f;
  float bot = position->y - 0.f;
  const D3DVECTOR& s = fence->mStartPoint;
  const D3DVECTOR& e = fence->mEndPoint;
  const D3DVECTOR& n = fence->mNormal;

  m_vtx_data[vi++] = { { s.x, top, s.z}, n, color, color };
  m_vtx_data[vi++] = { { s.x, bot, s.z}, n, color, color };
  m_vtx_data[vi++] = { { e.x, bot, e.z}, n, color, color };
  m_vtx_data[vi++] = { { e.x, bot, e.z}, n, color, color };
  m_vtx_data[vi++] = { { e.x, top, e.z}, n, color, color };
  m_vtx_data[vi++] = { { s.x, top, s.z}, n, color, color };

  m_vtx_cursor = vi;
}

inline Renderer g_renderer;
