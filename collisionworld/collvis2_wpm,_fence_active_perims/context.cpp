#include "pch.h"

#include "context.hpp"

Context::Context(IDirect3DDevice8* device) : m_device(device) {}

void Context::Init() {
  if (m_initialized) return;

  m_device->CreateVertexBuffer(MaxVtx * sizeof(Vertex),
    D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_DEFAULT, &m_vb);

  m_device->CreateStateBlock(D3DSBT_ALL, &m_state_block);

  m_initialized = true;
};

void Context::Commit(D3DVECTOR& position, float& radius, int area_index) {
  int index = 0;
  if (area_index != index) return;

  Init();

  m_passive_cursor = PassiveOffset;
  m_active_cursor = ActiveOffset;
  m_line_cursor = LineOffs;
  m_count_in_collidable_range = 0;

  Vertex* vtx_data = nullptr; 
  m_vb->Lock(0, 0, (BYTE**)&vtx_data, D3DLOCK_NOOVERWRITE);

  Iter(vtx_data, position, radius, area_index);

  m_vb->Unlock();
  vtx_data = nullptr;
}

void Context::Draw() {
  m_device->CaptureStateBlock(m_state_block);

  m_device->SetStreamSource(0, m_vb.Get(), sizeof(Vertex));
  m_device->SetVertexShader(Vertex::FVF);

  m_device->SetTexture(0, NULL);
  //m_device->SetMaterial(&m_mat);
  m_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
  m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);

  m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
  m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);


  m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, PassiveOffset, (m_passive_cursor - PassiveOffset) / 3);

  m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, ActiveOffset, (m_active_cursor - ActiveOffset) / 3);

  int bias = 2;
  m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
  m_device->SetRenderState(D3DRS_ZBIAS, bias);
  m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
  m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
  m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
  m_device->DrawPrimitive(D3DPT_LINELIST, LineOffs, (m_line_cursor - LineOffs) / 2);

  m_device->ApplyStateBlock(m_state_block);
}

void Context::Iter(Vertex* vtx_data, D3DVECTOR& position, float& radius, int area_index) {
  Shar::SpatialTreeIter& sti = Shar::RenderManager::GetInstance()->_wrl->mpWorldScene->mStaticTreeWalker;

  for (std::size_t i = 0; i < sti.NumNodes(); ++i) {
    Shar::SpatialNode& node = sti.rIthNode(i);;
    Shar::tMark mark = sti.mNodeMarks.mpData[i];

    bool node_is_active = (mark & 0b1111) == Shar::WorldScene::msStaticPhys;

    BakeFences(vtx_data, node, node_is_active, position, radius);
  }
}

void Context::BakeFences(Vertex* vtx_data, Shar::SpatialNode& node, bool is_active, D3DVECTOR& position, float radius) {
  if (!node.mFenceElems.mUseSize) return;
  if (!is_active) return;

  D3DCOLOR cd = is_active ? 0x4000ffff : 0x40c0c0c0;
  D3DCOLOR cs = cd;

  std::size_t passive_i = m_passive_cursor;
  std::size_t active_i = m_active_cursor;
  std::size_t line_i = m_line_cursor;

  float top = position.y + 5.f;
  float bot = position.y - 0.f;

  D3DXVECTOR3 center, scratch;
  float radius_sq = radius * radius;

  for (int i = node.mFenceElems.mUseSize - 1; i >= 0; --i) {
    cd = is_active ? 0x4000ffff : 0x40c0c0c0;
    bool collidable = false;
    bool overflow = false;

    Shar::FenceEntityDSG* fence = node.mFenceElems.mpData[i];
    D3DVECTOR& start = fence->mStartPoint;
    D3DVECTOR& end = fence->mEndPoint;
    D3DVECTOR& normal = fence->mNormal;

    D3DVECTOR st = { start.x, top, start.z };
    D3DVECTOR sb = { start.x, bot, start.z };
    D3DVECTOR et = { end.x, top, end.z };
    D3DVECTOR eb = { end.x, bot, end.z };

    //if (is_active) {

    scratch = end;
    scratch.y = 0.f;
    center = start;
    center.y = 0.f;
    center += scratch;
    center *= .5f;

    scratch -= center;
    float halflength_sq = D3DXVec3LengthSq(&scratch);
    float fence_is_collidable_distance_sq = halflength_sq + radius_sq;
    float fence_is_collidable_distance = sqrt(fence_is_collidable_distance_sq);

    scratch = position;
    scratch.y = 0.f;
    scratch -= center;
    float distance_to_fence_sq = D3DXVec3LengthSq(&scratch);

    if (distance_to_fence_sq < fence_is_collidable_distance_sq) {
      m_count_in_collidable_range++;
      if (m_count_in_collidable_range > MaxCollidable) {
        overflow = true;
        cd = 0x4000ff00;
      } else {
        collidable = true;
        cd = 0x40ff0000;
      }
    }

    if (overflow || collidable) {
      vtx_data[active_i++] = { st, normal, cd, cs };
      vtx_data[active_i++] = { sb, normal, cd, cs };
      vtx_data[active_i++] = { eb, normal, cd, cs };
      vtx_data[active_i++] = { eb, normal, cd, cs };
      vtx_data[active_i++] = { et, normal, cd, cs };
      vtx_data[active_i++] = { st, normal, cd, cs };

      vtx_data[line_i++] = { st, normal, cd, cs };
      vtx_data[line_i++] = { sb, normal, cd, cs };
      vtx_data[line_i++] = { sb, normal, cd, cs };
      vtx_data[line_i++] = { eb, normal, cd, cs };
      vtx_data[line_i++] = { eb, normal, cd, cs };
      vtx_data[line_i++] = { et, normal, cd, cs };
      vtx_data[line_i++] = { et, normal, cd, cs };
      vtx_data[line_i++] = { st, normal, cd, cs };
    }
    
    float pi = D3DX_PI;
    float tau = pi * 2;

    int sides = 24;
    for (int i = 0; i <= sides; ++i) {
      float theta = (tau / sides) * i;
      float x = sinf(theta) * fence_is_collidable_distance + center.x;
      float z = cosf(theta) * fence_is_collidable_distance + center.z;

      if (i == 0 || i == sides) {
        vtx_data[line_i++] = { { x, position.y, z }, normal, cd, cs };
        continue;
      }
      vtx_data[line_i++] = { { x, position.y, z }, normal, cd, cs };
      vtx_data[line_i++] = { { x, position.y, z }, normal, cd, cs };
    }
  }

  m_passive_cursor = passive_i;
  m_active_cursor = active_i;
  m_line_cursor = line_i;
}
