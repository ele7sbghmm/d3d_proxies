#pragma once

#include <cstdlib>
#include <wrl/client.h>
#include <d3dx8.h>

#include "shar.h"

using Microsoft::WRL::ComPtr;

shar::d3dDisplay* get_shar_d3dDisplay() { return *(shar::d3dDisplay**)0x65ef5c; }

struct Vtx {
    D3DVECTOR xyz;
    D3DCOLOR c;
    static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

void draw_circle(Vtx* data, std::size_t* vtx_n, D3DVECTOR pos, float r, D3DCOLOR c) {
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

class Ai {
public:
    Ai() = default;
    void init() {
        shar::d3dDisplay* display = get_shar_d3dDisplay();
        if (!display) return;
        m_device = display->GetD3DDevice();

        m_device->CreateVertexBuffer(MAX_VTX * sizeof(Vtx), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, Vtx::FVF, D3DPOOL_DEFAULT, &m_vb);

        m_device->CreateStateBlock(D3DSBT_ALL, &m_sbt);

        m_inited = true;
    }
    void populate() {
        if (!m_inited)
            init();

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

        if (ms->mNumVehicles <= 0) return;
        for (int i = 0; i < ms->mNumVehicles; ++i) {
            shar::MissionStage::VehicleInfo vi = ms->mVehicles[i];
            if (vi.vehicleAI == nullptr) continue;

            for (int j = 0; j < vi.vehicleAI->mNumSegments; ++j) {
                shar::Segment s = vi.vehicleAI->mSegments[j];
                data[m_vtx_n++] = { s.mStart, 0xff00ff00 };
                data[m_vtx_n++] = { s.mEnd, 0xff00ff00 };
            }
            
            if (vi.vehicleAI->mType == shar::VehicleAI::AI_WAYPOINT) {
                auto* ai = static_cast<shar::WaypointAI*>(vi.vehicleAI);
                for (int j = 0; j < ai->miNumWayPoints; ++j) {
                    shar::WaypointAI::WayPoint wp = ai->mpWayPoints[j];
                    D3DCOLOR c = j == ai->miCurrentWayPoint ? 0xffff0000 : 0xff00ffff;

                    if (wp.lco) {
                        draw_circle(data, &m_vtx_n, wp.lco->mLocation, ai->mTriggerRadius, c);
                    }
                    if (wp.seg) {
                        D3DXVECTOR3 start = (wp.seg->mCorners[0] + wp.seg->mCorners[3]);
                        D3DXVECTOR3 end = (wp.seg->mCorners[1] + wp.seg->mCorners[2]);
                        start *= .5f;
                        end *= .5f;

                        //data[m_vtx_n++] = { start, c };
                        //data[m_vtx_n++] = { end, c };
                    }
                }
            } else if (vi.vehicleAI->mType == shar::VehicleAI::AI_CHASE) {
                auto* ai = static_cast<shar::ChaseAI*>(vi.vehicleAI);
            } else {
                throw;
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
