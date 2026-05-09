#pragma once

#include <cstdint>
#include <cstddef>

using uptr = std::uintptr_t;
using usize = std::size_t;
using u64 = std::uint64_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;
using u8 = std::uint8_t;
using i8 = std::int8_t;

#pragma pack(push, 1)

struct SpatialNode;

namespace rmt {
	//struct Vector { float x, y, z; };
	//struct Matrix { float m[16]; };
	using Vector = D3DXVECTOR3;
	using Matrix = D3DXMATRIX;
}

using tMark = i32;
using tName = u64;

template <typename T> struct SwapArray { usize mSize, mUseSize; T* mpData; T mSwapT; };
template <typename T> struct FixedArray { usize mSize; T* mpData; };
template <typename T> struct ContiguousBinNode { T mData; i32 mSubTreeSize, mParentOffset; };

struct AAPlane3f { i8 mAxis; float mPosn; };
struct Bounds3f { rmt::Vector mMin, mMax; };
struct BoxPts { uptr vfptr; Bounds3f mBounds; };
struct SphereSP : BoxPts { rmt::Vector mCenter; float mRadius; };

struct CollisionEntityDSG { uptr vfptr; u8 pad_0[0x38]; };

struct StaticPhysDSG : CollisionEntityDSG { /**/ };
struct IntersectDSG : CollisionEntityDSG { /**/ };
struct DynaPhysDSG : CollisionEntityDSG { /**/ };
struct FenceEntityDSG : CollisionEntityDSG {
	rmt::Vector mStartPoint, mEndPoint, mNormal;
};
struct AnimCollisionEntityDSG { /**/ };
struct AnimEntityDSG { /**/ };
struct RoadSegment { /**/ };

struct DynaLoadListDSG {
	u8 pad_0[0xa8];
	SwapArray<FenceEntityDSG*> mFenceElems;
};

struct SpatialNode {
	AAPlane3f							mSubDivPlane;
	SwapArray<struct StaticEntityDSG*>	mSEntityElems;
	SwapArray<StaticPhysDSG*>			mSPhysElems;
	SwapArray<IntersectDSG*>			mIntersectElems;
	SwapArray<DynaPhysDSG*>				mDPhysElems;
	SwapArray<FenceEntityDSG*>			mFenceElems;
	SwapArray<AnimCollisionEntityDSG*>	mAnimCollElems;
	SwapArray<AnimEntityDSG*>			mAnimElems;
	SwapArray<struct TriggerVolume*>	mTrigVolElems;
	SwapArray<RoadSegment*>				mRoadSegmentElems;
	SwapArray<struct PathSegment*>		mPathSegmentElems;
	BoxPts								mBBox;
};

struct SpatialTreeIter {
	SwapArray<SpatialNode*>			mCurNodes;
	SwapArray<SpatialNode*>			mCurAlwaysVisNodes;
	SwapArray<SpatialNode*>*		mpCurNodeList;
	ContiguousBinNode<SpatialNode>* mpRootNode;
	ContiguousBinNode<SpatialNode>* mpCurNode;
	i32								mCurNodeOffset;
	FixedArray<tMark>				mNodeMarks;
	tMark							mCurMarkFilter;
	i32								mCurNodeI;
	BoxPts							mBBox;
};

struct WorldScene {
	uptr vfptr;
	SpatialTreeIter mStaticTreeWalker;
};

struct WorldRenderLayer {
	uptr vfptr;
	u8 pad_4[0x34];
	WorldScene* mpWorldScene;
	SwapArray<struct WorldSphereDSG*> mWorldSpheres;
	SwapArray<DynaLoadListDSG> mStaticLoadLists;
};

struct RenderManager {
	uptr vfptr;
	u8 pad_4[0x20];
	WorldRenderLayer* _wrl;
};

#pragma pack(pop)
