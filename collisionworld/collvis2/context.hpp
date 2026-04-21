#pragma once

#include <wrl/client.h>
#include <d3d8.h>
#include <cstdint>
#include <cstddef>

#include "shar.h"

using Microsoft::WRL::ComPtr;

struct Vertex {
  D3DVECTOR xyz, normal;
  D3DCOLOR diffuse, specular;
  inline static constexpr DWORD FVF
   = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
};

struct NodeGeom {
  struct Range { std::size_t start, count; };

  Shar::tMark last_frame_mark;
  Range fence_range;
};

class Context {
public:
  Context() = default;
  Context(IDirect3DDevice8*);
  void Init();
  void Commit();
  void Iter(Vertex*);
  void BakeFences(Vertex*, Shar::SpatialNode&, std::size_t, bool);
  void RecolorFences(Vertex*, NodeGeom&, Shar::tMark);

  ComPtr<IDirect3DDevice8> m_device{};
  ComPtr<IDirect3DVertexBuffer8> m_vb{};
  ComPtr<IDirect3DIndexBuffer8> m_ib{};
  std::size_t m_vtx_cursor = 0;

  NodeGeom m_node_geoms[10000]{};

  inline static constexpr std::size_t MAX_VTX = 1000000;
  inline static constexpr std::size_t MAX_IDX = 1000000;

  bool m_initialized = false;
  bool m_baked = false;
};

inline Context g_context;
