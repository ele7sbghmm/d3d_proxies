#pragma once

#include <cstdint>
#include <cstddef>
#include <d3dx8.h>

namespace shar {

#pragma pack(push, 1)

    namespace rmt {
        using Vector = D3DXVECTOR3;
        using Matrix = D3DXMATRIX;
        struct Sphere { Vector center; float radius; };
    }

    template <typename T> struct SwapArray { int mSize, mUseSize; T* mpData; T mSwapData; };

    struct Locator {
        char pad0[0x14];
        unsigned int mID;
        unsigned int mData;
        unsigned int mFlags;
        rmt::Vector mLocation;
    };


    struct VehicleController {
        void** vfptr;
        unsigned int m_refCounted;
        struct Vehicle* mpVehicle;
    };

    struct Button {
        float mfValue;
        unsigned int mTickCountAtChange;
    };

    struct AiVehicleController : VehicleController {
        Button mGas;
        Button mBrake;
        Button mSteering;
        Button mHandBrake;
        Button mReverse;
        Button mHorn;
    };

    struct RoadSegment {
        char pad[0x28];
        struct Road* mRoad;
        unsigned int mSegmentIndex;
        rmt::Vector mCorners[4];
        rmt::Vector mEdgeNormals[4];
        rmt::Vector mNormal;
        float mfSegmentLength;
        float mfLaneWidth;
        float mfRadius;
        float mfAngle;
        rmt::Sphere mSphere;
    };

    struct Segment {
        rmt::Vector mStart;
        rmt::Vector mEnd;
        float mLength;
        RoadSegment* mpSegment;
        int mType;
    };

    struct RoadManager {
        enum ElementType { ET_INTERSECTION, ET_NORMALROAD };
        struct PathElement {
            ElementType type;
            void* elem;
        };
    };

    struct CatchupParams {
        struct RaceCatchupParams {
            float DistMaxCatchup;
            float FractionPlayerSpeedMinCatchup;
            float FractionPlayerSpeedMidCatchup;
            float FractionPlayerSpeedMaxCatchup;
        } Race;
        struct EvadeCatchupParams {
            float DistPlayerTooNear;
            float DistPlayerFarEnough;
        } Evade;
        struct TargetCatchupParams {
            float DistPlayerNearEnough;
            float DistPlayerTooFar;
        } Target;
    };

    struct Potentials {
        void** vfptr;
        float mValues[11];
    };

    struct PotentialField {
        void** vfptr;
        Potentials mPotentials[50];
        float mXdim;
        float mZdim;
        float mXscale;
        float mZscale;
        rmt::Vector mXaxis;
        rmt::Vector mZaxis;
        rmt::Vector mOrigin;
    };

    struct VehicleAI : AiVehicleController {
        enum VehicleAITypeEnum { AI_WAYPOINT, AI_CHASE };
        enum VehicleAIState {
            STATE_WAITING,
            STATE_WAITING_FOR_PLAYER,
            STATE_ACCEL,
            STATE_BRAKE,
            STATE_CORNER_PREPARE,
            STATE_REVERSE,
            STATE_STOP,
            STATE_EVADE,
            STATE_LIMBO,
            STATE_STUNNED,
            STATE_OUT_OF_CONTROL
        };
        void** vfptr2;
        int mNumSegments;
        Segment mSegments[20];
        float mSecondsStunned;
        float mSecondsOutOfControl;
        rmt::Vector mOutOfControlNormal;
        SwapArray<RoadManager::PathElement> mPathElements;
        int mCurPathElement;
        RoadManager::PathElement mLastPathElement;
        RoadSegment* mLastRoadSegment;
        float mLastRoadSegmentT;
        float mLastRoadT;
        RoadManager::PathElement mRacePathElement;
        RoadSegment* mRaceRoadSegment;
        float mRaceRoadSegmentT;
        float mRaceRoadT;
        int mShortcutSkillMod;
        float mDesiredSpeedKmh;
        float mSecondsSinceLastDoCatchUp;
        CatchupParams mCatchupParams; // 374
        rmt::Vector mDestination;
        rmt::Vector mNextDestination;
        unsigned int mStartStuckTime;
        unsigned int mNextStuckTime;
        float mSteeringRatio;
        VehicleAIState mState;
        VehicleAIState mLimboPushedState;
        PotentialField mPotentialField; // 3c0
        VehicleAITypeEnum mType;
        int mHudIndex;
        float mSecondsBeforeCorner;
        int mRenderHandle;
        int mMinShortcutSkill;
        int mMaxShortcutSkill;
        float mSecondsLeftToGetBackOnPath;
        float mReverseTime;
        bool mEvadeVehicles : 1;
        bool mEvadeStatics : 1;
        bool mEvading : 1;
        bool mEnableSegmentOptimization : 1;
        bool mUseMultiplier : 1;
        char pad[3];
    };

    struct ChaseAI : VehicleAI {
        struct Avatar* mpTarget;
        bool mTargetHasMovedToAnotherPathElement : 1;
        char pad[3];
        RoadManager::PathElement mTargetPathElement;
        RoadSegment* mTargetRoadSegment;
        float mTargetRoadSegmentT;
        float mTargetRoadT;
        float mBeelineDist;
    };

    struct WaypointAI : VehicleAI {
        struct WayPoint {
            Locator* lco;
            RoadManager::PathElement elem;
            float segT;
            RoadSegment* seg;
            float roadT;
        };
        enum WaypointAIType {
            RACE = 0,
            EVADE = 1,
            TARGET = 2
        };
        WayPoint mpWayPoints[32];
        int miNumWayPoints;
        int miCurrentWayPoint;
        int miNextWayPoint;
        float mTriggerRadius;
        float mDistToCurrentCollectible;
        int miCurrentCollectible;
        int miNumLapsCompleted;
        bool mCurrWayPointHasMoved : 1;
        bool mNeedsResetOnSpot : 1;
        bool mAutoResetOnDestroy : 1;
        char pad[3];
        float mSecondsTillResetOnSpot;
        WaypointAIType mWaypointAIType;
        float mSecondsSinceTurboUse;
        float mSecondsWaitingForPlayer;
    };

    struct MissionStage {
        enum StageTimeType {
            STAGETIME_NOT_TIMED,
            STAGETIME_ADD,
            STAGETIME_SET
        };
        struct VehicleInfo {
            struct Vehicle* vehicle;
            struct CarStartLocator* spawn;
            int vehicleAINum;
            VehicleAI* vehicleAI;
        };
        void* pad[0x24];//char pad0[0x84];
        StageTimeType mStageTimeType;
        int mStageTime;
        int mNumVehicles;
        VehicleInfo mVehicles[4];
        int mNumWaypoints;
        Locator* mWaypoints[32];
        // ...
    };

    struct Mission {
        enum MissionState {
            STATE_WAITING,
            STATE_INPROGRESS,
            STATE_FAILED,
            STATE_SUCCESS
        };
        enum MissionStageState {
            STAGE_IDLE,
            STAGE_INPROGRESS,
            STAGE_COMPLETE,
            STAGE_FAILED,
            STAGE_BACKUP,
            STAGE_ALL_COMPLETE
        };
        void** vfptr;
        bool mIsStreetRace1Or2;
        char pad5[3];
        int mNumMissionStages;
        MissionStage* mMissionStages[25];
        int mCurrentStage;
        int mResetMission;
        char mcName[0x10];
        enum GameMemoryAllocator mHeap;
        bool mbComplete;
        bool mbIsLastStage;
        char pad8e[2];
        int mMissionTimer;
        int mElapsedTimems;
        MissionState mState;
        MissionStageState mLastStageState;
        // ...
    };

    struct GameplayManager {
        static auto GetInstance() -> GameplayManager* {
            return *(GameplayManager**)0x6c8998;
        }

        char pad0[0x3b4];
        int mCurrentMission;
        char pad3b8[0x68];
        int mNumMissions;
        Mission* mMission[30];
    };

    struct d3dDisplay {
        IDirect3DDevice8* GetD3DDevice() { return d3dDevice; }
        char pad[0x120];
        IDirect3DDevice8* d3dDevice;
    };

#pragma pack(pop)

}
