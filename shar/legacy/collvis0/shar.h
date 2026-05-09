#pragma once

#include <cstdint>
#include <cstddef>
#include <d3dx8.h>

using uptr = std::uintptr_t;
using usize = std::size_t;
using u64 = std::uint64_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;
using u8 = std::uint8_t;
using i8 = std::int8_t;


namespace shar {

#pragma pack(push, 1)

	namespace rmt {
		using Vector = D3DXVECTOR3;
		using Matrix = D3DXMATRIX;
	}

	using tMark = i32;
	using tName = u64;

	struct AAPlane3f { i8 mAxis; u8 pad_1[3]; float mPosn; };
	struct ISpatialProxyAA { void** vfptr; };
	struct Bounds3f { rmt::Vector mMin, mMax; };
	struct BoxPts : ISpatialProxyAA { Bounds3f mBounds; };

	template <typename T> struct SwapArray { usize mSize, mUseSize; T* mpData; T mSwapT; };
	template <typename T> struct FixedArray { usize mSize; T* mpData; };
	template <typename T> class ContiguousBinNode {
	public:
		T mData;
		i32 mSubTreeSize, mParentOffset;

		i32 GetSubTreeSize() { return mSubTreeSize; }
	};

	class SphereSP : public BoxPts {
	public:
		rmt::Vector mCenter;
		float mRadius;

		SphereSP() {
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
	};

	class CollisionEntityDSG {
	public:
		u8 pad_0[0x3c];
	};

	class FenceEntityDSG : public CollisionEntityDSG {
	public:
		rmt::Vector mStartPoint, mEndPoint, mNormal;
	};

	class SpatialNode {
	public:
		AAPlane3f								 mSubDivPlane;
		SwapArray<class StaticEntityDSG*>		 mSEntityElems;
		SwapArray<class StaticPhysDSG*>			 mSPhysElems;
		SwapArray<class IntersectDSG*>			 mIntersectElems;
		SwapArray<class DynaPhysDSG*>			 mDPhysElems;
		SwapArray<FenceEntityDSG*>				 mFenceElems;
		SwapArray<class AnimCollisionEntityDSG*> mAnimCollElems;
		SwapArray<class AnimEntityDSG*>			 mAnimElems;
		SwapArray<class TriggerVolume*>			 mTrigVolElems;
		SwapArray<class RoadSegment*>			 mRoadSegmentElems;
		SwapArray<class PathSegment*>			 mPathSegmentElems;
		BoxPts									 mBBox;
	};

	class SpatialTreeIter {
	public:
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

		i32 NumNodes() { return mpRootNode->GetSubTreeSize() + 1; }
		SpatialNode& rIthNode(i32 iIth) { return (mpRootNode + iIth)->mData; }
		SpatialNode& rCurrent() { return *mpCurNodeList->mpData[mCurNodeI]; }
		void MoveToFirst() {
			mpCurNode = mpRootNode;
			mCurNodeOffset = 0;
			mCurNodeI = 0;
			mpCurNodeList = &mCurNodes;
		}
		void MoveToNext(bool ibIncludeVis = false) { mCurNodeI++; }
		bool NotDone() { return mCurNodeI < mpCurNodeList->mUseSize; }
		bool IsCurrentLeaf() { return mpCurNodeList->mpData[mCurNodeI]->mSubDivPlane.mAxis == -1; }
		SpatialNode& rSeekNode(ISpatialProxyAA& irVolume, int iCurNodeOffset) {
			void* addr = (void*)0x49a420;
			void* out = nullptr;

			__asm mov eax, iCurNodeOffset
			__asm push irVolume
			__asm mov ebx, this
			__asm call addr
			__asm mov out, eax

			return *(SpatialNode*)out;
		}
		void MarkAll(SphereSP& irDesiredVolume, tMark iMark) {
			mCurNodes.mUseSize = 0;
			BoxPts tmpBox(mBBox);
			MarkAll(0, tmpBox, irDesiredVolume, iMark);
		}
		void MarkAll(int iCurNodeOffset, BoxPts& irBoxPts, SphereSP& irDesiredVolume, tMark iMark) {
			void* addr = (void*)0x49a510;

			__asm push iMark
			__asm push irDesiredVolume
			__asm push irBoxPts
			__asm push iCurNodeOffset
			__asm mov ecx, this
			__asm call addr
		}
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
		u8 pad_0[0x24];
		WorldRenderLayer* _wrl;

		static RenderManager* GetInstance() {
			return *reinterpret_cast<RenderManager**>(0x6c87b4);
		}
	};

	class RoadSegment;
	class Character;
	class Vehicle;
	class Avatar {
	public:
		u8 pad_0[0x10];
		RoadSegment* mLastRoadSegment;
		u8 pad_14[0x1c];
		Character* mpCharacter;
		Vehicle* mpVehicle;

		void GetPosition(rmt::Vector* irPosn) {
			void* addr = (void*)0x4d76f0;

			__asm mov esi, irPosn
			__asm mov eax, this
			__asm call addr
		}
	};

	class AvatarManager {
	public:
		static AvatarManager* GetInstance() {
			return *reinterpret_cast<AvatarManager**>(0x6c84e4);
		}
		Avatar* GetAvatarForPlayer(i32 playerId) {
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
		u8 pad_0[0x410];
		float mWheelBase;

		float GetWheelBase() { return mWheelBase; }
	};

#pragma pack(pop)

}
