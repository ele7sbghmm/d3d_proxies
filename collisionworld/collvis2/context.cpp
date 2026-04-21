#include "pch.h"

#include "context.hpp"

Context::Context(IDirect3DDevice8* device) : m_device(device) {}

void Context::Init() {
  if (m_initialized) return;

  m_device->CreateVertexBuffer(MAX_VTX * sizeof(Vertex),
    D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_DEFAULT, &m_vb);

   m_initialized = true;
};

void Context::Commit() {
  Init();

  Vertex* vtx_data = nullptr;
  std::uint32_t* idx_data = nullptr;

  m_vb->Lock(0, 0, (BYTE**)&vtx_data, D3DLOCK_NOOVERWRITE);

  Iter(vtx_data);

  m_vb->Unlock();

  m_device->SetRenderState(D3DRS_LIGHTING, TRUE);

  m_device->SetStreamSource(0, m_vb.Get(), sizeof(Vertex));
  m_device->SetVertexShader(Vertex::FVF);
  m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_vtx_cursor / 3);
}

void Context::Iter(Vertex* vtx_data) {
  Shar::SpatialTreeIter& sti = Shar::RenderManager::GetInstance()->_wrl->mpWorldScene->mStaticTreeWalker;
  Shar::Avatar* avatar = Shar::AvatarManager::GetInstance()->GetAvatarForPlayer(0);

  D3DVECTOR position{};
  float radius{};

  avatar->GetPosition(&position);

  if (!avatar->mpVehicle)
    radius = 5.f;
  else
    radius = avatar->mpVehicle->GetWheelBase() * 2.f;

  Shar::SphereSP sphere;
  sphere._constructor();
  sphere.SetTo(position, radius);

  sti.MarkAll(sphere, Shar::WorldScene::msStaticPhys);
  for (std::size_t i = 0; i < sti.NumNodes(); ++i) {
    Shar::SpatialNode& node = sti.rIthNode(i);;
    Shar::tMark mark = sti.mNodeMarks.mpData[i];

    bool is_active_node = (mark & 0b1111) == Shar::WorldScene::msStaticPhys;

    if (!m_baked) {
      BakeFences(vtx_data, node, i, is_active_node);
      m_baked = true;
    } else {
      RecolorFences(vtx_data, m_node_geoms[i], mark);
    }
  }
}

void Context::BakeFences(Vertex* vtx_data, Shar::SpatialNode& node, std::size_t node_index, bool is_active) {
  if (!node.mFenceElems.mUseSize) return;

  std::size_t vtx_count = m_vtx_cursor;

  float top = 10.f;
  float bottom = -10.f;

  for (int i = node.mFenceElems.mUseSize; i >= 0; --i) {
    Shar::FenceEntityDSG* fence = node.mFenceElems.mpData[i];
    D3DVECTOR& start = fence->mStartPoint;
    D3DVECTOR& end = fence->mEndPoint;
    D3DVECTOR& normal = fence->mNormal;

    D3DVECTOR st = { start.x, top, start.z };
    D3DVECTOR sb = { start.x, bottom, start.z };
    D3DVECTOR eb = { end.x, bottom, end.z };
    D3DVECTOR et = { end.x, top, end.z };

    vtx_data[vtx_count++] = { st, normal, 0xff000000, 0xffff0000 };
    vtx_data[vtx_count++] = { sb, normal, 0xff000000, 0xffff0000 };
    vtx_data[vtx_count++] = { eb, normal, 0xff000000, 0xffff0000 };
    vtx_data[vtx_count++] = { eb, normal, 0xff000000, 0xffff0000 };
    vtx_data[vtx_count++] = { et, normal, 0xff000000, 0xffff0000 };
    vtx_data[vtx_count++] = { st, normal, 0xff000000, 0xffff0000 };
  }

  m_node_geoms[node_index].fence_range.start = m_vtx_cursor;
  m_node_geoms[node_index].fence_range.count = vtx_count - m_vtx_cursor;

  m_vtx_cursor = vtx_count;
}

void Context::RecolorFences(Vertex* vtx_data, NodeGeom& ng, Shar::tMark mark) {
  if (!ng.fence_range.start || ng.fence_range.count) return;
  if (ng.last_frame_mark == mark) return;

  D3DCOLOR color = mark ? 0xffff0000 : 0xffffffff;

  for (int i = 0; i < ng.fence_range.count; ++i)
    vtx_data[ng.fence_range.start + i].diffuse = color;
}