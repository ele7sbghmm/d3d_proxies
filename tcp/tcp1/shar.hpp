#pragma once

#include <cstdint>
#include <d3d8.h>

#pragma pack(push, 1)
class Shar {
public:
    template <typename T> struct SwapArray { std::uint32_t mSize, mUseSize; T* mpData; T mSwapData; };
    struct FenceEntityDSG {
        std::uint8_t pad_0[0x3c];
        D3DVECTOR mStartPoint;
        D3DVECTOR mEndPoint;
        D3DVECTOR mNormal;
    };

    struct DynaLoadListDSG {
        std::uint8_t pad_0[0x58];
        SwapArray<FenceEntityDSG*> mFenceElems;
    };

    class WorldRenderLayer {
    public:
        std::uint8_t pad_0[0x4c];
        SwapArray<DynaLoadListDSG> mStaticLoadList;
    };

    class RenderManager {
    public:
        static RenderManager* GetInstance() { return *(RenderManager**)0x6c87b4; }
        WorldRenderLayer* GetWorldRenderLayer() { return (WorldRenderLayer*)((std::uint8_t*)this + 0x24); }
    };

    class d3dDisplay {
    public:
        IDirect3DDevice8* GetD3DDevice() { return d3dDevice; }
        std::uint8_t pad_0[0x120];
        IDirect3DDevice8* d3dDevice;
    };

    class p3d {
    public:
        static d3dDisplay** pddi;
    };
};
#pragma pack(pop)

inline Shar::d3dDisplay** Shar::p3d::pddi = (Shar::d3dDisplay**)0x65ef5c;
