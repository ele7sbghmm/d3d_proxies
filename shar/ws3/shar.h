#pragma once

#include <string>
#include <d3dx8.h>

class FenceEntityDSG {
public:
    char pad[0x3c];
    D3DVECTOR start, end, normal;

    std::string stringify() {
        char buf[1024]{};
        sprintf_s(buf, sizeof(buf),
            "{"
            "\"start\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
            "\"end\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
            "\"normal\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}"
            "}",
            start.x * .5f + 500.f, start.y, start.z * -.5f + 500.f,
            end.x * .5f + 500.f, end.y, end.z * -.5f + 500.f,
            normal.x, normal.y, normal.z
        );
        return buf;
    }
};

template <typename T> class SwapArray {
public:
    int mSize, mUseSize;
    T* mpData;
    T mSwapT;
};

class DynaLoadListDSG {
public:
    char pad[0x58];
    SwapArray<FenceEntityDSG*> mFenceElems;
};

class WorldRenderLayer {
public:
    char pad[0x110];
    SwapArray<DynaLoadListDSG*> mLoadLists;
};

class RenderManager {
public:
    char pad[0x24];
    WorldRenderLayer* wrl;

    static RenderManager* GetInstance() {
        return *(RenderManager**)0x6c87b4;
    }
};
