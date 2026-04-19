#include "pch.h"

#include "mod.h"
#include <algorithm>

extern bool g_inited;

bool DragIntStyled(const char* label, int* v, int v_min, int v_max) {
  ImVec2 vec = ImGui::GetContentRegionAvail();
  ImGui::SetNextItemWidth(vec.x * .4);
  //ImGui::SetCursorPosX((ImGui::GetWindowWidth() - w) * .3f);
  return ImGui::DragInt(label, v, 1, v_min, v_max);
}

namespace Mod {
  void Gui::Draw(D3DXVECTOR3* pos) {
    if (!g_inited) return;

    Shar::CoinManager* cm = Shar::CoinManager::GetInstance();
    if (!cm) return;

    char buf[256];

    ImGui_ImplDX8_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = true;
    io.FontGlobalScale = 1.2f;

    ImGui::Begin("##being");

    if (ImGui::Button("spawn coin")) {
      Shar::AvatarManager* am = Shar::AvatarManager::GetInstance();
      if (am && am->GetAvatarForPlayer(0)) {
        D3DVECTOR pos, dir{ 0.f, 1.f, 0.f };
        am->GetAvatarForPlayer(0)->GetPosition(&pos);
        cm->SpawnCoins(2, &pos, pos.y, &dir, false);
      }
    }
    if (ImGui::Button("clear"))
      Mod::CoinStats::Clear();

    int tmp = Mod::CoinStats::s_num_coins;
    if (DragIntStyled("coins", &tmp, 1, 200))
      Mod::CoinStats::s_num_coins = std::clamp(tmp, 0, 200);
      CoinStats::SetNumCoins(Mod::CoinStats::s_num_coins);

    tmp = cm->mNumActiveCoins;
    if (DragIntStyled("active coins", &tmp, 0, Mod::CoinStats::s_num_coins * 2))
      cm->mNumActiveCoins = (std::uint16_t)tmp;

    tmp = cm->mNumHUDFlying;
    if (DragIntStyled("hud flying", &tmp, 0, Mod::CoinStats::s_num_coins * 2))
      cm->mNumHUDFlying = (std::uint16_t)tmp;

    tmp = cm->mNextInactiveCoin;
    if (DragIntStyled("next inactive", &tmp, 0, Mod::CoinStats::s_num_coins))
      cm->mNextInactiveCoin = (std::uint16_t)tmp;

    //ImGui::SliderFloat3("last coin to transition", (float*)&tmp, -10.f, 10.f, "%.2f");

    float val = *Shar::CoinManager::FLYING_TIME;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * .4f);
    if (ImGui::DragFloat("flying time", &val, .1f, .01f, 100.f, "%.2f")) {
      DWORD old;
      VirtualProtect(Shar::CoinManager::FLYING_TIME, sizeof(float), PAGE_READWRITE, &old);
      *Shar::CoinManager::FLYING_TIME = val;
      VirtualProtect(Shar::CoinManager::FLYING_TIME, sizeof(float), old, &old);
      VirtualProtect(Shar::CoinManager::I_FLYING_TIME, sizeof(float), PAGE_READWRITE, &old);
      *Shar::CoinManager::I_FLYING_TIME = 1 / val;
      VirtualProtect(Shar::CoinManager::I_FLYING_TIME, sizeof(float), old, &old);
    }
    val = *Shar::CoinManager::I_FLYING_TIME;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * .4f);
    if (ImGui::DragFloat("inv flying time", &val, .1f, .01f, 100.f, "%.2f")) {
      DWORD old;
      VirtualProtect(Shar::CoinManager::FLYING_TIME, sizeof(float), PAGE_READWRITE, &old);
      *Shar::CoinManager::FLYING_TIME = 1 / val;
      VirtualProtect(Shar::CoinManager::FLYING_TIME, sizeof(float), old, &old);
      VirtualProtect(Shar::CoinManager::I_FLYING_TIME, sizeof(float), PAGE_READWRITE, &old);
      *Shar::CoinManager::I_FLYING_TIME = val;
      VirtualProtect(Shar::CoinManager::I_FLYING_TIME, sizeof(float), old, &old);
    }

    val = *Shar::CoinManager::MAX_VISIBILIY;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * .4f);
    if (ImGui::DragFloat("max visibility", &val, 1.f, 0.f, 500.f, "%.0f")) {
      DWORD old;
      VirtualProtect(Shar::CoinManager::MAX_VISIBILIY, sizeof(float), PAGE_READWRITE, &old);
      *Shar::CoinManager::MAX_VISIBILIY = val;
      VirtualProtect(Shar::CoinManager::MAX_VISIBILIY, sizeof(float), old, &old);
    }
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
  }
}
