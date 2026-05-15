#pragma once

#include "shar.h"

class PrimList {
public:
  size_t m_vtx_n = 0;
  D3DXVECTOR3 *m_data = nullptr;
  D3DCOLOR m_c = 0xffffff00;

  PrimList(D3DCOLOR c) : m_c(c) {}
  void draw_line(D3DXVECTOR3 s, D3DXVECTOR3 e) {
    m_data[m_vtx_n++] = s;
    m_data[m_vtx_n++] = e;
  }
};

class Draw {
public:
  PrimList yellow{ 0xffffff00 };

  void draw_road(PrimList pt, shar::RoadSegment *seg, D3DCOLOR c) {
    D3DXVECTOR3 lead_edge = .5f * (seg->mCorners[0] + seg->mCorners[3]);
    D3DXVECTOR3 trail_edge = .5f * (seg->mCorners[1] + seg->mCorners[2]);

    pt.draw_line(lead_edge, trail_edge);
  }
};

class Ctx {
public:
  Draw b{};

  Ctx() = default;
  void iter_nodes(shar::SpatialTreeIter &sti) {
    // sti.
  }
};

inline Ctx g_ctx{};