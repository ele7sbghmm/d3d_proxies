#pragma once

#include "util.h"

struct DynaLoadListDSG;

struct WorldScene {
	static WorldScene* Get();
};

struct WorldRenderLayer {
	char _0_38[0x38];
	WorldScene* mpWorldScene;
	char _3c_4c[0x10];
	SwapArray<DynaLoadListDSG> mStaticLoadLists;
	char _5c_110[0xb4];
	SwapArray<DynaLoadListDSG*> mLoadLists;

	static WorldRenderLayer* Get();
};

struct RenderManager {
	char _0_24[0x24];
	WorldRenderLayer* mpRenderLayers_WRL;
	char _28_34[0xc];
	unsigned int msLayer;

	static RenderManager* Get();
};
