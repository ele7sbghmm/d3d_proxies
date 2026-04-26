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
  inline static constexpr std::size_t MAX_FENCES = 100000;
  inline static constexpr std::size_t MAX_VERTICES = MAX_FENCES * 6 * 3;
  inline static constexpr std::size_t NONCOLLIDABLE_OFFSET = 0;
  inline static constexpr std::size_t COLLIDABLE_OFFSET = 10000;
  inline static constexpr std::size_t OVERFLOW_OFFSET = COLLIDABLE_OFFSET * 2;
  inline static constexpr std::size_t MAX_COLLIDABLE = 8;
  
  ComPtr<IDirect3DDevice8> m_device;
  ComPtr<IDirect3DVertexBuffer8> m_vb;
  D3DMATERIAL8 m_noncollidable{};
  D3DMATERIAL8 m_collidable{};
  D3DMATERIAL8 m_overflow{};
  DWORD m_state_block_token = NULL;

  Vtx* m_vtx_data = nullptr;
  std::size_t m_vtx_cursor_noncollidable = 0;
  std::size_t m_vtx_cursor_collidable = COLLIDABLE_OFFSET;
  std::size_t m_vtx_cursor_overflow = OVERFLOW_OFFSET;
  std::size_t m_area_index = 0;
  std::size_t m_area_index_collidable_fences_count = 0;

  bool m_initialized = false;
  bool m_well_begun = false;
};

void Renderer::Init() {
  m_device->CreateVertexBuffer(MAX_VERTICES * sizeof(Vtx),
    D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
    Vtx::FVF,
    D3DPOOL_DEFAULT,
    &m_vb);

  m_device->CreateStateBlock(D3DSBT_ALL, &m_state_block_token);

  #define _ 0.f
  #define D .2f
  #define E .9f
  m_noncollidable.Diffuse  = { D, D, D, _ };
  m_noncollidable.Ambient  = { _, _, _, _ };
  m_noncollidable.Specular = { _, _, _, _ };
  m_noncollidable.Emissive = { E, E, E, _ };
  m_collidable.Diffuse     = { D, _, _, _ };
  m_collidable.Ambient     = { _, _, _, _ };
  m_collidable.Specular    = { _, _, _, _ };
  m_collidable.Emissive    = { E, _, _, _ };
  m_overflow.Diffuse       = { _, D, _, _ };
  m_overflow.Ambient       = { _, _, _, _ };
  m_overflow.Specular      = { _, _, _, _ };
  m_overflow.Emissive      = { _, E, _, _ };

  m_initialized = true;
}
void Renderer::Begin() {
  if (!m_initialized) Init();

  m_vtx_cursor_noncollidable = 0;
  m_vtx_cursor_collidable = COLLIDABLE_OFFSET;
  m_vtx_cursor_overflow = OVERFLOW_OFFSET;
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

  //m_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR2);
  //m_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR2);
  //m_device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
  //m_device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
  m_device->SetRenderState(D3DRS_AMBIENT, NULL);

  m_device->SetTexture(0, NULL);
  m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  m_device->SetRenderState(D3DRS_LIGHTING, TRUE);
  m_device->SetRenderState(D3DRS_COLORVERTEX, FALSE);
  m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

  m_device->SetMaterial(&m_noncollidable);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, NONCOLLIDABLE_OFFSET, (m_vtx_cursor_noncollidable - NONCOLLIDABLE_OFFSET) / 3);
  m_device->SetMaterial(&m_collidable);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, COLLIDABLE_OFFSET, (m_vtx_cursor_collidable - COLLIDABLE_OFFSET) / 3);
  m_device->SetMaterial(&m_overflow);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, OVERFLOW_OFFSET, (m_vtx_cursor_overflow - OVERFLOW_OFFSET) / 3);

  m_device->ApplyStateBlock(m_state_block_token);
}
void Renderer::Bake(shar::FenceEntityDSG* fence, float range, float distance_to, D3DVECTOR* position, int area_index) {
  if (area_index != m_area_index) return;
  if (!m_well_begun) Begin();

  float top = position->y + 10.f;
  float bot = position->y - 1.f;
  const D3DVECTOR& s = fence->mStartPoint;
  const D3DVECTOR& e = fence->mEndPoint;
  const D3DVECTOR& n = fence->mNormal;

  static constexpr D3DCOLOR WHITE = 0x4000ffff, RED = 0x40ff0000, GREEN = 0x4000ff00;

  if (range > distance_to) {
    m_area_index_collidable_fences_count++;
    if (m_area_index_collidable_fences_count > MAX_COLLIDABLE) {
      std::size_t vi = m_vtx_cursor_overflow;
      m_vtx_data[vi++] = { { s.x, top, s.z}, n, GREEN, NULL };
      m_vtx_data[vi++] = { { s.x, bot, s.z}, n, GREEN, NULL };
      m_vtx_data[vi++] = { { e.x, bot, e.z}, n, GREEN, NULL };
      m_vtx_data[vi++] = { { e.x, bot, e.z}, n, GREEN, NULL };
      m_vtx_data[vi++] = { { e.x, top, e.z}, n, GREEN, NULL };
      m_vtx_data[vi++] = { { s.x, top, s.z}, n, GREEN, NULL };
      m_vtx_cursor_overflow = vi;
    } else {
      std::size_t vi = m_vtx_cursor_collidable;
      m_vtx_data[vi++] = { { s.x, top, s.z}, n, RED, NULL };
      m_vtx_data[vi++] = { { s.x, bot, s.z}, n, RED, NULL };
      m_vtx_data[vi++] = { { e.x, bot, e.z}, n, RED, NULL };
      m_vtx_data[vi++] = { { e.x, bot, e.z}, n, RED, NULL };
      m_vtx_data[vi++] = { { e.x, top, e.z}, n, RED, NULL };
      m_vtx_data[vi++] = { { s.x, top, s.z}, n, RED, NULL };
      m_vtx_cursor_collidable = vi;
    }
  } else {
    std::size_t vi = m_vtx_cursor_noncollidable;
    m_vtx_data[vi++] = { { s.x, top, s.z}, n, WHITE, NULL };
    m_vtx_data[vi++] = { { s.x, bot, s.z}, n, WHITE, NULL };
    m_vtx_data[vi++] = { { e.x, bot, e.z}, n, WHITE, NULL };
    m_vtx_data[vi++] = { { e.x, bot, e.z}, n, WHITE, NULL };
    m_vtx_data[vi++] = { { e.x, top, e.z}, n, WHITE, NULL };
    m_vtx_data[vi++] = { { s.x, top, s.z}, n, WHITE, NULL };
    m_vtx_cursor_noncollidable = vi;
  }

}

inline Renderer g_renderer;
