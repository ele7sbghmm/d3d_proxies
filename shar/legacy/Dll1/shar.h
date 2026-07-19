#pragma once

#include <cstdint>
#include <d3dx8.h>

using uptr = std::uintptr_t;
using u64 = std::uint64_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;

namespace shar {
#pragma pack(push, 1)
    const i32 MAX_PLAYERS = 4;

    using tName = u64;

    namespace rmt {
        using Vector = D3DXVECTOR3;
        using Matrix = D3DXMATRIX;
        struct Sphere {
            Vector center;
            float radius;
        };
    }

    namespace LocatorType {
        enum Type {
            EVENT,
            SCRIPT,
            GENERIC,
            CAR_START,
            SPLINE,
            DYNAMIC_ZONE,
            OCCLUSION,
            INTERIOR_ENTRANCE,
            DIRECTIONAL,
            ACTION,
            FOV,
            BREAKABLE_CAMERA,
            STATIC_CAMERA,
            PED_GROUP,
            COIN,
            SPAWN_POINT,
        };
    }

    struct d3dDisplay {
        u8 pad[0x120];
        IDirect3DDevice8* d3dDevice;

        static d3dDisplay* get_() {
            return *(d3dDisplay**)0x65ef5c;
        }
    };

    struct Vehicle {

    };

    struct Locator {
        u8 pad[0x14]; // tEntity + IHudMapIconLocator
        u32 mID;
        u32 mDate;
        u32 mFlags;
        rmt::Vector mLocation;
    }; // 0x2c

    struct TriggerLocator : Locator { // 0x2c
        struct TriggerVolume** mTriggerVolumes;
        u16 mNumTriggers;
        u16 mMaxNumTriggers;
        bool mPlayerEntered;
        u8 pad2[3];
        i32 mPlayerID;
    }; // 0x3c

    struct TriggerVolume {
        enum Type { SPHERE, RECTANGLE };

        void** vfptr;
        u8 pad[0x24]; // IEntityDSG
        TriggerLocator* mLocator;
        rmt::Vector mPosition;
        u8 mTrackingPlayers;
        u8 pad2[3];
        u32 mFrameUsed;
        i32 mUser;

        Type GetType() {
            using Fn = Type(*)();
            return ((Fn)vfptr[24])();
        }
    }; // 0x44

    struct RectTriggerVolume : TriggerVolume {
        rmt::Vector mAxisX;
        rmt::Vector mAxisY;
        rmt::Vector mAxisZ;
        float mExtentX;
        float mExtentY;
        float mExtentZ;
        float mRadius;
        rmt::Matrix mWorld2Trigger;
        rmt::Matrix mTrigger2World;
    }; // 0xf8

    struct TriggerVolumeTracker {
        struct RegisteredAI { i32 mTriggerTypes; Vehicle* mVehicle; };
        enum { MAX_VOLUMES = 500, MAX_ACTIVE = 20, MAX_AI = 10, MAX_AI_VOLUMES = 100, };
        void** vfptr;
        struct tDrawable* mpTriggerSphere;
        u32 mTriggerCount;
        TriggerVolume* mTriggerVolumes[MAX_VOLUMES];
        u32 mActiveCount[MAX_PLAYERS];
        TriggerVolume* mActiveVolumes[MAX_PLAYERS][MAX_ACTIVE];
        u32 mAICount[MAX_AI];
        TriggerVolume* mActiveAIVolumes[MAX_AI][MAX_AI_VOLUMES];
        u32 mNumRegisteredAI;
        RegisteredAI mRegisteredVehicles[MAX_AI];
        bool mIgnoreTriggers;
        u8 pad2[3];

        static rmt::Vector* p_sP1;
        static rmt::Vector* p_sP2;
        static rmt::Vector* p_sP3;
        static rmt::Vector* p_sP4;

        static TriggerVolumeTracker* GetInstance() {
            return *reinterpret_cast<TriggerVolumeTracker**>(0x6c8410);
        }
    };
    rmt::Vector* TriggerVolumeTracker::p_sP1 = (rmt::Vector*)0x6c9190;
    rmt::Vector* TriggerVolumeTracker::p_sP2 = (rmt::Vector*)0x6c919c;
    rmt::Vector* TriggerVolumeTracker::p_sP3 = (rmt::Vector*)0x6c91a8;
    rmt::Vector* TriggerVolumeTracker::p_sP4 = (rmt::Vector*)0x6c91b4;

#pragma pack(pop)
}