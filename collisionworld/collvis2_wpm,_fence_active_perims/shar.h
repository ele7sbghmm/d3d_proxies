#pragma once

#include <cstdint>
#include <d3d8.h>

namespace Shar {

#pragma pack(push, 1)

  namespace rmt {
    using Vector = D3DVECTOR;
    using Matrix = D3DMATRIX;
    using Matrix3 = D3DMATRIX;
    struct Box3D { Vector low, high; };
    struct Sphere { Vector center; float radius; };
  }

  using tMark = std::int32_t;
  using tName = std::uint64_t;

  struct VoidPList { std::uint8_t pad_0[8]; std::int32_t mSize, mArraySize; void** mArray; };
  template <typename T> struct SwapArray { std::size_t mSize, mUseSize; T* mpData; T mSwapT; };
  template <typename T> struct FixedArray { std::size_t mSize; T* mpData; };
  template <typename T> struct ReserveArray { std::size_t mSize, mUseSize; T* mpData; };
  template <typename T> class ContiguousBinNode {
  public:
    std::int32_t GetSubTreeSize() { return mSubTreeSize; }

    T mData;
    std::int32_t mSubTreeSize;
    std::int32_t mParentOffset;
  };

  struct AAPlane3f { std::int8_t mAxis; std::uint8_t pad_1[3]; float mPosn; };
  struct ISpatialProxyAA { void** vfptr; };
  struct Bounds3f { rmt::Vector mMin, mMax; };
  struct BoxPts : ISpatialProxyAA { Bounds3f mBounds; };

  class SphereSP : public BoxPts {
  public:
    void _constructor() {
      void* addr = (void*)0x49ac10;

      __asm mov eax, this
      __asm call addr
    }
    void SetTo(rmt::Vector center, float radius) {
      void* addr = (void*)0x49add0;

      float x = center.x;
      float y = center.y;
      float z = center.z;

      __asm push radius
      __asm push z
      __asm push y
      __asm push x
      __asm mov eax, this
      __asm call addr
    }

    rmt::Vector mCenter;
    float mRadius;
  };

  namespace sim {
    enum CollisionVolumeTypeEnum : std::uint32_t {
      CollisionVolumeType,
      SphereVolumeType,
      CylinderVolumeType,
      OBBoxVolumeType,
      WallVolumeType,
      BBoxVolumeType
    };
    struct CollisionVolume {
      std::uint8_t pad_0[8];
      D3DVECTOR    mPosition;
      D3DVECTOR    mBoxSize;
      float        mSphereRadius;
      CollisionVolumeTypeEnum mType;
      std::uint8_t pad_28[0xc];
      VoidPList*   mSubVolumeList;
      std::uint8_t pad_38[4];
      rmt::Vector  mDP;
      std::uint8_t pad_48[4];
    };
    struct OBBoxVolume : CollisionVolume {
      rmt::Vector mAxis[3];
      float mLength[3];
    };
    struct SphereVolume : CollisionVolume { };
    struct CylinderVolume : CollisionVolume {
      rmt::Vector mAxis;
      float       mLength;
      float       mCylinderRadius;
      bool        mFlatEnd;
    };
    struct CollisionObject {
      std::uint8_t pad_0[0x20];
      CollisionVolume* mCollisionVolume;
      std::uint8_t pad_24[0x3c];
      std::uint8_t pad_60_0 : 4;
      bool         mCollisionEnabled : 1;
    };
    struct SimState {
      std::uint8_t pad_0[0x10];
      rmt::Matrix  mTransform;
      std::uint8_t pad_50[0x18];
      float        mScale;
      std::uint8_t pad_6c[8];
      CollisionObject* mCollisionObject;
    };
  }

  class CollisionEntityDSG {
  public:
    std::uint8_t pad_0[0x3c];
  };

  class StaticPhysDSG : public CollisionEntityDSG {
  public:
    rmt::Box3D     mBBox;
    rmt::Sphere    mSphere;
    rmt::Vector    mPosn;
    sim::SimState* mpSimStateObj;
  };

  class DynaPhysDSG : public StaticPhysDSG {
  public:
  };

  class IntersectDSG {
  public:
    std::uint8_t               pad_0[0x28];
    ReserveArray<std::int32_t> mTriIndices;
    ReserveArray<rmt::Vector>  mTriPts;
    ReserveArray<rmt::Vector>  mTriNorms;
    ReserveArray<std::uint8_t> mTerrainType;
  };

  class FenceEntityDSG : public CollisionEntityDSG {
  public:
    rmt::Vector mStartPoint;
    rmt::Vector mEndPoint;
    rmt::Vector mNormal;
  };

  class SpatialNode {
  public:
    AAPlane3f                         mSubDivPlane;
    SwapArray<class StaticEntityDSG*> mSEntityElems;
    SwapArray<StaticPhysDSG*>         mSPhysElems;
    SwapArray<IntersectDSG*>          mIntersectElems;
    SwapArray<DynaPhysDSG*>           mDPhysElems;
    SwapArray<FenceEntityDSG*>        mFenceElems;
    SwapArray<class AnimCollisionEntityDSG*> mAnimCollElems;
    SwapArray<class AnimEntityDSG*>   mAnimElems;
    SwapArray<class TriggerVolume*>   mTrigVolElems;
    SwapArray<class RoadSegment*>     mRoadSegmentElems;
    SwapArray<class PathSegment*>     mPathSegmentElems;
    BoxPts                            mBBox;
  };

  class SpatialTreeIter {
  public:
    std::int32_t NumNodes() { return mpRootNode->GetSubTreeSize() + 1; }
    SpatialNode& rIthNode(std::int32_t iIth) { return (mpRootNode + iIth)->mData; }
    SpatialNode& rCurrent() { return *mpCurNodeList->mpData[mCurNodeI]; }
    void MoveToFirst() {
      mpCurNode = mpRootNode;
      mCurNodeOffset = 0;
      mCurNodeI = 0;
      mpCurNodeList = &mCurNodes;
    }
    void MoveToNext() { mCurNodeI++; }
    bool NotDone() { return mCurNodeI < mpCurNodeList->mUseSize; }
    bool IsCurrentLeaf() {
      return mpCurNodeList->mpData[mCurNodeI]->mSubDivPlane.mAxis == -1;
    }
    void MarkAll(SphereSP& irDesiredVolume, tMark iMark) {
      void* addr = (void*)0x4b4200;

      __asm push iMark
      __asm push irDesiredVolume
      __asm push this
      __asm call addr
    }

    SwapArray<SpatialNode*> mCurNodes;
    SwapArray<SpatialNode*> mCurAlwaysVisNodes;
    SwapArray<SpatialNode*>* mpCurNodeList;
    ContiguousBinNode<SpatialNode>* mpRootNode;
    ContiguousBinNode<SpatialNode>* mpCurNode;
    std::int32_t            mCurNodeOffset;
    FixedArray<tMark>       mNodeMarks;
    tMark                   mCurMarkFilter;
    std::int32_t            mCurNodeI;
    BoxPts                  mBBox;
  };

  class WorldScene {
  public:
    enum : std::int32_t { msStaticPhys = 0xf };
    std::uint8_t pad_0[4];
    SpatialTreeIter mStaticTreeWalker;
  };

  class WorldRenderLayer {
  public:
    std::uint8_t pad_0[0x38];
    WorldScene* mpWorldScene;
  };

  class RenderManager {
  public:
    static RenderManager* GetInstance() {
      return *(RenderManager**)0x6c87b4;
    }

    std::uint8_t pad_0[0x24];
    WorldRenderLayer* _wrl;
  };

  class RoadSegment;
  class Character;
  class Vehicle;
  class Avatar {
  public:
    void GetPosition(rmt::Vector* irPosn) {
      void* addr = (void*)0x4d76f0;

      __asm mov esi, irPosn
      __asm mov eax, this
      __asm call addr
    }

    std::uint8_t pad_0[0x10];
    RoadSegment* mLastRoadSegment;
    std::uint8_t pad_14[0x1c];
    Character* mpCharacter;
    Vehicle* mpVehicle;
  };

  class AvatarManager {
  public:
    static AvatarManager* GetInstance() {
      return *(AvatarManager**)0x6c84e4;
    }
    Avatar* GetAvatarForPlayer(std::int32_t playerId) {
      void* addr = (void*)0x4d7f40;
      void* out = nullptr;

      __asm mov eax, playerId
      __asm mov ecx, this
      __asm call addr
      __asm mov out, eax

      return (Avatar*)out;
    }
  };

  class Character {
  public:
  };

  class Vehicle {
  public:
    float GetWheelBase() { return mWheelBase; }

    std::uint8_t pad_0[0x410];
    float mWheelBase;
  };

}