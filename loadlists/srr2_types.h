#pragma once

#include <d3dx8.h>

namespace sim { struct CollisionVolume; }
struct StaticPhysDSG;

namespace rmt {
	struct Vector { float x, y, z; };
}

template <typename T>
struct SwapArray {
	size_t mSize;
	size_t mUseSize;
	T* mpData;
	void* mSwapT;
};

struct WorldSphereDSG {};
struct StaticEntityDSG {};
struct IntersectDSG {};
struct DynaPhysDSG {};
struct FenceEntityDSG {
	unsigned char _0_3c[0x3c];
	D3DXVECTOR3 mStartPoint, mEndPoint, mNormal;
};
struct AnimCollisionEntityDSG {};
struct AnimEntityDSG {};
struct TriggerVolume {};
struct RoadSegment {};
struct PathSegment {};
struct DynaLoadListDSG {
	unsigned char _0_8[0x8];
	SwapArray<WorldSphereDSG*>         mWorldSphereElems;
	SwapArray<StaticEntityDSG*>        mSEntityElems;
	SwapArray<StaticPhysDSG*>          mSPhysElems;
	SwapArray<IntersectDSG*>           mIntersectElems;
	SwapArray<DynaPhysDSG*>            mDPhysElems;
	SwapArray<FenceEntityDSG*>         mFenceElems;
	SwapArray<AnimCollisionEntityDSG*> mAnimCollElems;
	SwapArray<AnimEntityDSG*>          mAnimElems;
	SwapArray<TriggerVolume*>          mTrigVolElems;
	SwapArray<RoadSegment*>            mRoadSegmentElems;
	SwapArray<PathSegment*>            mPathSegmentElems;
};

struct WorldRenderLayer {
	unsigned char _0_110[0x110];
	SwapArray<DynaLoadListDSG*>mpLoadLists;
};
struct RenderManager {
	static auto GetInstance() -> RenderManager*;
	auto pWorldRenderLayer() -> WorldRenderLayer*;
};

struct Character {
	unsigned char _0_f0[0xf0];
	int mCollisionAreaIndex;
};
struct Avatar {
	auto GetPosition(rmt::Vector* pos) -> void;
};
struct AvatarManager {
	static auto GetInstance() -> AvatarManager*;
	auto GetAvatarForPlayer(size_t id) -> Avatar*;
};
struct SuperCamCentral {};
struct SuperCamManager {
	static auto GetInstance() -> SuperCamManager*;
	auto GetSCC(size_t id) -> SuperCamCentral*;
};

namespace sim {
	struct SimState;
	struct CollisionObject {
		void* base[0x4];
		SimState* mSimState;
		void* mPhysicsProperties;
		CollisionVolume* mCollisionVolume;
		int mNumJoint;
		void* mSelfCollisionList[0x5];
		void* mCollisionVolumeOwner[0x7];
		void* bits[0x4];
	};
	struct SimState {
		void* base[0x2];
		void* mAIRefPointer;
		int mAIRefIndex;
		D3DXMATRIX mTransform;
		unsigned char mVelocityState[0x18];
		float mScale;
		int mControl;
		void* mSimulatedObject;
		CollisionObject* mCollisionObject;
		void* mVirtualCM;
		bool mObjectMoving;
		float mSafeTimeBeforeCollision;
		float mApproxSpeedMagnitude;
		bool mArticulated;
	};
	struct CollisionVolume {
		void* _0_8[0x2];
		D3DXVECTOR3 mPosition;
		D3DXVECTOR3 mBoxSize;
		float mSphereRadius;
		unsigned int mType;
		int mObjRefIndex;
		int mOwnerIndex;
		CollisionObject* mCollisionObject;
		CollisionVolume* mSubVolumeList;
		bool mVisible;
		bool mUpdated;
		D3DXVECTOR3 mDP;
		DWORD mColour;
	};
}

struct tDrawable;
struct tName { void* _0_8[0x2]; };
struct IEntityDSG {
	void* base[0x5];
	float mRank;
	bool mTranslucent;
	tName mShaderName;
	void* mpSpatialNode;
};
struct Box3D {

};
struct StaticPhysDSG {
	IEntityDSG base;
struct StaticEntityDSG {
	IEntityDSG base;
	D3DXVECTOR3 mPosn;
	int mIsGeo;
	tDrawable* mpDrawstuff;
};
struct OBBoxVolume {
	sim::CollisionVolume mCollisionVolume;
	D3DXVECTOR3 mAxis[3];
	float mLength[3];
	unsigned int mAxisOrientation; // [CollisionVolume, Sphere, Cylinder, OBBox, Wall, BBox]
	D3DXVECTOR3 mAxisRef[3];
	float mLengthRef[3];
};