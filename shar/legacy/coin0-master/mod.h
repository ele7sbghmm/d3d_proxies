#pragma once

#include "shar.h"

namespace Mod {
  class Gui {
  public:
    void Draw(D3DXVECTOR3* pos);

    bool inited = false;
  };

  class CoinStats {
  public:
    static int s_num_coins;
    D3DXVECTOR3 m_last_position{ 1.f, 2.f, 3.f };

    static void Clear() {
      auto* cm = Shar::CoinManager::GetInstance();
      if (!cm) return;

      for (std::int32_t i = 0; i < 200; ++i) {
        Shar::CoinManager::ActiveCoin& coin = cm->mActiveCoins[i];
        coin.Position = {};
        coin.Age = 0;
        coin.Ground = 0;
        coin.State = Shar::CoinManager::eCoinState::CS_Inactive;
      }
      cm->mNextInactiveCoin = 0;
      cm->mNumActiveCoins = 0;
      cm->mNumHUDFlying = 0;
    }

    static void SetNumCoins(std::uint16_t n) {
      DWORD old;
      void* dst = (void*)0x505554;
      VirtualProtect(dst, 2, PAGE_READWRITE, &old);
      memcpy(dst, &n, 2);
      VirtualProtect(dst, 2, old, &old);

      dst = (void*)0x505571;
      VirtualProtect(dst, 2, PAGE_READWRITE, &old);
      memcpy(dst, &n, 2);
      VirtualProtect(dst, 2, old, &old);

      dst = (void*)0x505a3b;
      std::uint16_t len = n * sizeof(Shar::CoinManager::ActiveCoin);
      VirtualProtect(dst, 2, PAGE_READWRITE, &old);
      memcpy(dst, &len, 2);
      VirtualProtect(dst, 2, old, &old);
    }
  };
  inline int CoinStats::s_num_coins = 200;

  class RenderState {

  };

  class Context {
  public:
    Gui m_gui;
    RenderState m_rs;
    CoinStats m_coinstats;
    D3DXVECTOR3* pos;
  };

  inline Context context{};
}

inline bool g_inited = false;
inline void coin_set_zfunc() {
  g_inited = true;
  Shar::tContext* ctx = *Shar::p3d::context_p;
  if (!ctx) return;

  IDirect3DDevice8* device = ctx->GetDisplay()->GetD3DDevice();
  device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
}

inline void RenderGui() {
  Mod::context.m_gui.Draw(Mod::context.pos);
}
