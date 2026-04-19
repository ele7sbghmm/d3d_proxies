#pragma once

#include <cstdint>
#include <d3d8.h>

using usize = std::size_t;
using u64 = std::uint64_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;
using u8 = std::uint8_t;
using i8 = std::int8_t;

namespace Shar {

#pragma pack(push, 1)

	namespace rmt {
		using Vector = D3DVECTOR;
		using Matrix = D3DMATRIX;
		struct Box3D { Vector low, high; };
		struct Sphere { Vector center; float radius; };
	}

	using tMark = i32;
	using tName = u64;

	struct VoidPList { u8 pad_0[8]; i32 mSize, mArraySize; void** mArray; };
	template <typename T> struct SwapArray { usize mSize, mUseSize; T* mpData; T mSwapT; };
	template <typename T> struct FixedArray { usize mSize; T* mpData; };
	template <typename T> struct ReserveArray { usize mSize, mUseSize; T* mpData; };
	template <typename T> class ContiguousBinNode {
	public:
		i32 GetSubTreeSize() { return mSubTreeSize; }

		T mData;
		i32 mSubTreeSize;
		i32 mParentOffset;
	};

	struct AAPlane3f { i8 mAxis; u8 pad_1[3]; float mPosn; };
	struct ISpatialProxyAA { void** vfptr; };
	struct Bounds3f { rmt::Vector mMin, mMax; };
	struct BoxPts : ISpatialProxyAA { Bounds3f mBounds; };

	class SphereSP : public BoxPts {
	public:
		void _constructor() {
			void* addr = reinterpret_cast<void*>(0x49ac10);

			__asm mov eax, this
			__asm call addr
		}
		void SetTo(rmt::Vector center, float radius) {
			void* addr = reinterpret_cast<void*>(0x49add0);

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
		enum CollisionVolumeTypeEnum : u32 {
			CollisionVolumeType,
			SphereVolumeType,
			CylinderVolumeType,
			OBBoxVolumeType,
			WallVolumeType,
			BBoxVolumeType
		};
		struct CollisionVolume {
			u8 pad_0[8];
			D3DVECTOR mPosition;
			D3DVECTOR mBoxSize;
			float mSphereRadius;
			CollisionVolumeTypeEnum mType;
			u8 pad_28[0xc];
			VoidPList* mSubVolumeList;
			u8 pad_38[4];
			rmt::Vector mDP;
			u8 pad_48[4];
		};
		struct OBBoxVolume : CollisionVolume {
			rmt::Vector mAxis[3];
			float mLength[3];
		};
		struct SphereVolume : CollisionVolume {};
		struct CylinderVolume : CollisionVolume {
			rmt::Vector mAxis;
			float mLength;
			float mCylinderRadius;
			bool mFlatEnd;
		};
		struct CollisionObject {
			u8 pad_0[0x20];
			CollisionVolume* mCollisionVolume;
			u8 pad_24[0x3c];
			u8 pad_60_0 : 4;
			bool mCollisionEnabled : 1;
		};
		struct SimState {
			u8 pad_0[0x10];
			rmt::Matrix mTransform;
			u8 pad_50[0x18];
			float mScale;
			u8 pad_6c[8];
			CollisionObject* mCollisionObject;
		};
	}

	class CollisionEntityDSG {
	public:
		u8 pad_0[0x3c];
	};

	class StaticPhysDSG : public CollisionEntityDSG {
	public:
		rmt::Box3D	   mBBox;
		rmt::Sphere	   mSphere;
		rmt::Vector	   mPosn;
		sim::SimState* mpSimStateObj;
	};

	class DynaPhysDSG : public StaticPhysDSG {
	public:
	};

	class IntersectDSG {
	public:
		u8 pad_0[0x28];
		ReserveArray<i32>		  mTriIndices;
		ReserveArray<rmt::Vector> mTriPts;
		ReserveArray<rmt::Vector> mTriNorms;
		ReserveArray<u8>		  mTerrainType;
	};

	class FenceEntityDSG : public CollisionEntityDSG {
	public:
		rmt::Vector mStartPoint, mEndPoint, mNormal;
	};

	class SpatialNode {
	public:
		AAPlane3f						mSubDivPlane;
		SwapArray<class StaticEntityDSG*> mSEntityElems;
		SwapArray<StaticPhysDSG*>		mSPhysElems;
		SwapArray<IntersectDSG*>		mIntersectElems;
		SwapArray<DynaPhysDSG*>			mDPhysElems;
		SwapArray<FenceEntityDSG*>		mFenceElems;
		SwapArray<class AnimCollisionEntityDSG*> mAnimCollElems;
		SwapArray<class AnimEntityDSG*> mAnimElems;
		SwapArray<class TriggerVolume*> mTrigVolElems;
		SwapArray<class RoadSegment*>	mRoadSegmentElems;
		SwapArray<class PathSegment*>	mPathSegmentElems;
		BoxPts							mBBox;
	};

	class SpatialTreeIter {
	public:
		i32 NumNodes() { return mpRootNode->GetSubTreeSize() + 1; }
		SpatialNode& rIthNode(i32 iIth) { return (mpRootNode + iIth)->mData; }
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
			void* addr = reinterpret_cast<void*>(0x4b4200);

			__asm push iMark
			__asm push irDesiredVolume
			__asm push this
			__asm call addr
		}

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

	class WorldScene {
	public:
		enum : i32 { msStaticPhys = 0xf };
		u8 pad_0[4];
		SpatialTreeIter mStaticTreeWalker;
	};

	class WorldRenderLayer {
	public:
		u8 pad_0[0x38];
		WorldScene* mpWorldScene;
	};

	class RenderManager {
	public:
		static RenderManager* GetInstance() {
			return *reinterpret_cast<RenderManager**>(0x6c87b4);
		}

		u8 pad_0[0x24];
		WorldRenderLayer* _wrl;
	};

	class RoadSegment;
	class Character;
	class Vehicle;
	class Avatar {
	public:
		void GetPosition(rmt::Vector* irPosn) {
			void* addr = reinterpret_cast<void*>(0x4d76f0);

			__asm mov esi, irPosn
			__asm mov eax, this
			__asm call addr
		}

		u8 pad_0[0x10];
		RoadSegment* mLastRoadSegment;
		u8 pad_14[0x1c];
		Character* mpCharacter;
		Vehicle* mpVehicle;
	};

	class AvatarManager {
	public:
		static AvatarManager* GetInstance() {
			return *reinterpret_cast<AvatarManager**>(0x6c84e4);
		}
		Avatar* GetAvatarForPlayer(i32 playerId) {
			void* addr = reinterpret_cast<void*>(0x4d7f40);
			void* out = nullptr;

			__asm mov eax, playerId
			__asm mov ecx, this
			__asm call addr
			__asm mov out, eax

			return reinterpret_cast<Avatar*>(out);
		}
	};

	class Character {
	public:
	};

	class Vehicle {
	public:
		float GetWheelBase() { return mWheelBase; }

		u8 pad_0[0x410];
		float mWheelBase;
	};

	class RoadSegment {
	public:
	};

	class d3dDisplay {
	public:
		u8 pad_0[0x11c];
		IDirect3D8* d3d;
		IDirect3DDevice8* d3dDevice;
	};

	auto get_display() -> Shar::d3dDisplay* {
		return *reinterpret_cast<Shar::d3dDisplay**>(0x65ef5c);
	}

#pragma pack(pop)

}