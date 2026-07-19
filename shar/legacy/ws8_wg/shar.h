#pragma once

#include <d3dx8.h>

namespace shar {
#pragma push(pack, 1)
	namespace rmt {
		using Vector = D3DXVECTOR3;
	}

	template <typename T> struct SwapArray { int mSize, mUseSize; T* mpData; T mSwapT; };

	struct RoadSegment {
		char pad[0x30];
		rmt::Vector mCorners[4];
	};

	struct FenceEntityDSG {
		char pad[0x3c];
		rmt::Vector mStartPoint, mEndPoint, mNormal;
	};

	struct DynaLoadListDSG {
		char pad[0x58];
		SwapArray<FenceEntityDSG*> mFenceElems;
		char pad1[0x30];
		SwapArray<RoadSegment*> mRoadSegmentElems;
	};

	struct WorldRenderLayer {
		char pad[0x4c];
		SwapArray<DynaLoadListDSG> mStaticLoadLists;
	};

	struct RenderManager {
		char pad[0x24];
		WorldRenderLayer* wrl_;

		static RenderManager* GetInstance() {
			return *reinterpret_cast<RenderManager**>(0x6c87b4);
		}
	};
#pragma pack(pop)
}