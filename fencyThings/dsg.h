#pragma once

#include <cstdint>
#include <d3dx8.h>

#include "util.h"

struct CollisionEntityDSG {};
struct WorldSphereDSG {};
struct StaticEntityDSG {};
struct StaticPhysDSG {};
struct IntersectDSG {};
struct DynaPhysDSG {};
struct FenceEntityDSG {
	char p_0_3c[0x3c];
	D3DXVECTOR3 mStartPoint, mEndPoint, mNormal;
};
struct AnimCollisionEntityDSG {};
struct AnimEntityDSG {};
struct TriggerVolume {}; // unused
struct RoadSegment {};
struct PathSegment {};
struct DynaLoadListDSG {
	unsigned int tGiveItAFuckinName[2];
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
