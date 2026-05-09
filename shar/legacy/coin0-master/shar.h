#pragma once

#include <cstdint>
#include <d3d8.h>

namespace Shar {

#pragma pack(push, 1)

  class Avatar {
  public:
    void GetPosition(D3DVECTOR* irPosn) {
      void* addr = reinterpret_cast<void*>(0x4d76f0);

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
    Avatar* GetAvatarForPlayer(std::uint32_t playerId) {
      void* addr = reinterpret_cast<void*>(0x4d7f40);
      void* out = nullptr;

      __asm mov eax, playerId
      __asm mov ecx, this
      __asm call addr
      __asm mov out, eax

      return reinterpret_cast<Avatar*>(out);
    }
  };

  class CoinManager {
  public:
    static auto GetInstance() -> CoinManager* { return *(CoinManager**)0x6c8450; }
    auto SpawnCoins(std::int32_t Count, D3DVECTOR* Position, float Ground, D3DVECTOR* Direction, bool Force) {
      void* addr = reinterpret_cast<void*>(0x5055d0);

      __asm mov al, Force
      __asm mov ecx, Direction
      __asm push Ground
      __asm push Position
      __asm push Count
      __asm mov edx, this
      __asm call addr
    }

    enum eCoinState : std::uint32_t {
      CS_Inactive,
      CS_InitialSpawning,
      CS_SpawnToCollect,
      CS_Spawning,
      CS_Resting,
      CS_RestingIndefinitely,
      CS_Decaying,
      CS_Collecting,
      CS_Collected,
      CS_FlyingToHUD,
      CS_FlyingFromHUD
    };
    struct ActiveCoin {
      D3DVECTOR Velocity, Position;
      float HeadingCos, HeadingSin, Age, Ground;
      eCoinState State;
    };

    static inline float* MAX_VISIBILIY = reinterpret_cast<float*>(0x6049e0);
    static inline float* FLYING_TIME = reinterpret_cast<float*>(0x6049dc);
    static inline float* I_FLYING_TIME = reinterpret_cast<float*>(0x6c9208);
    static inline float* RANGE = reinterpret_cast<float*>(0x6049cc);
    static inline float* IN_CAR_RANGE_MULTIPLIER = reinterpret_cast<float*>(0x6049f0);

    std::uint8_t pad_0[0x1c];
    ActiveCoin* mActiveCoins;
    std::uint16_t mNumActiveCoins;
    std::uint16_t mNextInactiveCoin;
    std::uint16_t mNumHUDFlying;
  };

  class d3dDevice {
  public:
  };

  class d3dContext {
  public:
  };

  class d3dDisplay {
  public:
    auto GetD3D() -> IDirect3D8* { return d3d; }
    auto GetD3DDevice() -> IDirect3DDevice8* { return d3dDevice; }
    std::uint8_t pad_0[0x10];
    IDirect3D8* d3d;
    IDirect3DDevice8* d3dDevice;
  };

  class tContext {
  public:
    auto GetDevice() -> d3dDevice* { return RenderDevice; }
    auto GetContext() -> d3dContext* { return RenderContext; }
    auto GetDisplay() -> d3dDisplay* { return RenderDisplay; }

    std::uint8_t pad_0[0xe8];
    d3dDevice* RenderDevice;
    d3dContext* RenderContext;
    d3dDisplay* RenderDisplay;
  };

  namespace p3d {
    static tContext** const context_p = reinterpret_cast<Shar::tContext**>(0x65EF70);
  }

#pragma pack(pop)

};
