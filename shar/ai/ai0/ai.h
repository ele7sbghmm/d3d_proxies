#pragma once

#include <cstdlib>
#include <wrl/client.h>
#include <d3dx8.h>
#pragma comment(lib, "d3dx8.lib")

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx8.h>
#include "shar.h"

using Microsoft::WRL::ComPtr;

shar::d3dDisplay* get_shar_d3dDisplay() { return *(shar::d3dDisplay**)0x65ef5c; }

struct Vtx {
    D3DXVECTOR3 xyz;
    D3DCOLOR c;
    static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

void draw_circle(Vtx* data, std::size_t* vtx_n, D3DXVECTOR3 pos, float r, D3DCOLOR c) {
    std::size_t n = *vtx_n;

    int sides = 72;
    float inc = (D3DX_PI * 2) / sides;
    for (int i = 0; i <= sides; ++i) {
        float theta = inc * i;
        float x = sinf(theta) * r + pos.x;
        float z = cosf(theta) * r + pos.z;

        if (i != 0 && i != sides)
            data[n++] = { { x, pos.y, z }, c };
        data[n++] = { { x, pos.y, z }, c };
    }

    *vtx_n = n;
}

//WNDPROC oWndProc;
//LRESULT __stdcall hWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
//    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
//    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
//}

class Ai {
public:
    Ai() = default;
    void init() {
        if (shar::d3dDisplay* display = get_shar_d3dDisplay(); display != nullptr)
            m_device = display->d3dDevice;

        m_device->CreateVertexBuffer(MAX_VTX * sizeof(Vtx), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, Vtx::FVF, D3DPOOL_DEFAULT, &m_vb);

        m_device->CreateStateBlock(D3DSBT_ALL, &m_sbt);

        //ImGui::CreateContext();
        //ImGui_ImplWin32_Init(GetActiveWindow());
        //ImGui_ImplDX8_Init(m_device.Get());
        //oWndProc = (WNDPROC)SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)hWndProc);

        m_inited = true;
    }
    void populate() {
        if (!m_inited)
            init();

        //ImGui_ImplWin32_NewFrame();
        //ImGui_ImplDX8_NewFrame();
        //ImGui::NewFrame();
        //ImGui::GetIO().MouseDrawCursor = true;

        Vtx* data = {};
        m_vb->Lock(0, 0, (BYTE**)&data, D3DLOCK_NOOVERWRITE);

        auto* gm = shar::GameplayManager::GetInstance();
        if (!gm) return;

        if (gm->mCurrentMission < 0) return;
        shar::Mission* m = gm->mMission[gm->mCurrentMission];
        if (!m) return;

        if (m->mCurrentStage < 0) return;
        shar::MissionStage* ms = m->mMissionStages[m->mCurrentStage];
        if (!ms) return;

        for (int i = 0; i < ms->mNumConditions; ++i) {
            auto* mc = ms->mConditions[i];
            if (!mc) continue;

            if (mc->mType == shar::MissionCondition::COND_FOLLOW_DISTANCE) {
                auto* fc = static_cast<shar::FollowCondition*>(mc);
                auto& p = *reinterpret_cast<D3DXVECTOR3*>(fc->mpVehicle->mTransform.m[3]);

                draw_circle(data, &m_vtx_n, p, fc->mMaxDistance, 0xffff0000);
            }
        }

        if (ms->mNumVehicles <= 0) return;
        for (int i = 0; i < ms->mNumVehicles; ++i) {
            shar::MissionStage::VehicleInfo vi = ms->mVehicles[i];
            if (vi.vehicleAI == nullptr) continue;
            shar::VehicleAI* vai = vi.vehicleAI;

            for (int j = 0; j < vai->mNumSegments; ++j) {
                shar::Segment s = vai->mSegments[j];
                data[m_vtx_n++] = { s.mStart, 0xff00ffff };
                data[m_vtx_n++] = { s.mEnd, 0xff0000ff };
            }

            if (auto* v = vai->mpVehicle; v != nullptr) {
                D3DXVECTOR3& dest = vai->mDestination;
                D3DXVECTOR3& nd = vai->mNextDestination;
                auto& p = *reinterpret_cast<D3DXVECTOR3*>(v->mTransform.m[3]);

                D3DXVECTOR3 delta = dest - p;
                D3DXVec3Normalize(&delta, &delta);

                D3DXVECTOR3 perp;
                D3DXVECTOR3 up = { 0.f, 1.f, 0.f };
                D3DXVec3Cross(&perp, &delta, &up);
                D3DXVec3Normalize(&perp, &perp);

                D3DXVECTOR3 tip_l = dest + perp - delta;
                D3DXVECTOR3 tip_r = dest - perp - delta;

                data[m_vtx_n++] = { { tip_l.x, tip_l.y, tip_l.z }, 0xff00ff00 };
                data[m_vtx_n++] = { { dest.x, dest.y, dest.z }, 0xff00ff00 };
                data[m_vtx_n++] = { { tip_r.x, tip_r.y, tip_r.z }, 0xff00ff00 };
                data[m_vtx_n++] = { { dest.x, dest.y, dest.z }, 0xff00ff00 };
                data[m_vtx_n++] = { { dest.x, dest.y, dest.z }, 0xff00ff00 };
                data[m_vtx_n++] = { { p.x, dest.y, p.z }, 0xff00ff00 };
            }
            
            if (vai->mType == shar::VehicleAI::AI_WAYPOINT) {
                auto* ai = static_cast<shar::WaypointAI*>(vi.vehicleAI);
                for (int j = 0; j < ai->miNumWayPoints; ++j) {
                    shar::WaypointAI::WayPoint wp = ai->mpWayPoints[j];
                    if (!wp.loc) continue;

                    D3DCOLOR c = j == ai->miCurrentWayPoint ? 0xffff0000
                               : j == ai->miNextWayPoint ? 0xffff8000 : 0xffffff00;

                    draw_circle(data, &m_vtx_n, wp.loc->mLocation, ai->mTriggerRadius, c);
                }
            } else if (vi.vehicleAI->mType == shar::VehicleAI::AI_CHASE) {
                auto* ai = static_cast<shar::ChaseAI*>(vi.vehicleAI);
            }
        }

        m_vb->Unlock();
    }
    void draw() {
        m_device->SetStreamSource(0, m_vb.Get(), sizeof(Vtx));
        m_device->SetTexture(0, NULL);

        DWORD oLighting, oColorVertex, oAlphaBlendEnable, oAlphaTestEnable, oZFunc;
        m_device->GetRenderState(D3DRS_LIGHTING, &oLighting);
        m_device->GetRenderState(D3DRS_COLORVERTEX, &oColorVertex);
        m_device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oAlphaBlendEnable);
        m_device->GetRenderState(D3DRS_ALPHATESTENABLE, &oAlphaTestEnable);
        m_device->GetRenderState(D3DRS_ZFUNC, &oZFunc);

        m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
        m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        m_device->DrawPrimitive(D3DPT_LINELIST, 0, m_vtx_n / 2);

        m_device->SetRenderState(D3DRS_LIGHTING, oLighting);
        m_device->SetRenderState(D3DRS_COLORVERTEX, oColorVertex);
        m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, oAlphaBlendEnable);
        m_device->SetRenderState(D3DRS_ALPHATESTENABLE, oAlphaTestEnable);
        m_device->SetRenderState(D3DRS_ZFUNC, oZFunc);

        m_vtx_n = 0;
    }

    ComPtr<IDirect3DDevice8> m_device{};
    ComPtr<IDirect3DVertexBuffer8> m_vb{};
    DWORD m_sbt = NULL;
    std::size_t m_vtx_n = 0;

    bool m_inited = false;

    static constexpr std::size_t MAX_VTX = 100000;


};

inline Ai g_ai;
