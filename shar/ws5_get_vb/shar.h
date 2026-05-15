#pragma once

#include <d3dx8.h>

using u32 = unsigned int;
using i32 = int;
using u8 = unsigned char;

namespace shar {

using tMark = i32;

namespace rmt {
struct Sphere {
  D3DXVECTOR3 center;
  float radius;
};
} // namespace rmt

template <typename T> struct SwapArray {
  i32 mSize, mUseSize;
  T *mpData;
  T mSwapT;
};
template <typename T> struct FixedArray {
  i32 mSize;
  T *mpData
};
template <typename T> struct ConfiguousBinNode {
  T mData;
  i32 mSubTreeSize, mParentOffset;
};

struct SpatialNode {
  u8 pad[0x88];
  SwapArray<RoadSegment> mRoadSegmentElems;
};

struct RoadSegment {
  u8 pad[0x28]; // 0  IEntityDSG
  struct Road *mRoad;
  u32 mSegmentIndex;

  D3DXVECTOR3 mCorners[4]; // 30
  D3DXVECTOR3 mEdgeNormals[4];
  D3DXVECTOR3 mNormal;

  float mfSegmentLength; // 9c
  float mfLaneWidth;
  float mfRadius;
  float mfAngle;
  rmt::Sphere mSphere;
};

struct SpatialTreeIter {
  u8 pad[0x30];
  FixedArray<tMark> mNodeMarks;

  SpatialNode &rIthNode(i32 iIth) {}
  i32 NumNodes() {}
};

struct WorldScene {
  enum { msStaticPhys = 0xf };
  u8 pad[4];
  SpatialTreeIter mStaticTreeWalker;
};

struct WorldRenderLayer {
  u8 pad[0x38];
  WorldScene *mpWorldScene;
};

struct RenderManager {
  u8 pad[0x24];
  WorldRenderLayer *_wrl;

  static RenderManager *GetInstance() { return *(RenderManager **)0x6c87b4; }
};

} // namespace shar

shar::SpatialTreeIter *get_spatial_tree_iter() {
  if (auto *rm = shar::RenderManager::GetInstance(); rm)
    return &rm->_wrl->mpWorldScene->mStaticTreeWalker;
  return nullptr;
}
