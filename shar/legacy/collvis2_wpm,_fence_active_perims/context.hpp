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

class Context {
public:
  Context() = default;
  Context(IDirect3DDevice8*);
  void Init();
  void Commit(D3DVECTOR&, float&, int);
  void Draw();
  void Iter(Vertex*, D3DVECTOR&, float&, int);
  void BakeFences(Vertex*, Shar::SpatialNode&, bool, D3DVECTOR&, float);

  inline static constexpr std::size_t MaxVtx        = 10000000;
  inline static constexpr std::size_t PassiveOffset = 0;
  inline static constexpr std::size_t ActiveOffset  = 2000000;
  inline static constexpr std::size_t LineOffs      = 5000000;
  inline static constexpr std::size_t MaxCollidable = 8;
  
  ComPtr<IDirect3DDevice8> m_device{};
  ComPtr<IDirect3DVertexBuffer8> m_vb{};
  D3DMATERIAL8 m_mat{};
  DWORD m_state_block{};

  std::size_t m_passive_cursor = PassiveOffset;
  std::size_t m_active_cursor  = ActiveOffset;
  std::size_t m_line_cursor    = LineOffs;
  std::size_t m_count_in_collidable_range = 0;

  bool m_initialized = false;
};

inline Context g_context;
