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

    struct SuperCam {
        enum Type : int {

        };
    };

    struct Locator {
        char pad0[0x14];
        unsigned int mID;
        unsigned int mData;
        unsigned int mFlags;
        rmt::Vector mLocation;
    };

    struct Vehicle {
        char pad0[0xb8];
        rmt::Matrix mTransform;
    };

    struct VehicleController {
        void** vfptr;
        unsigned int m_refCounted;
        Vehicle* mpVehicle;
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
        static constexpr int MAX_WAYPOINTS = 32;
        struct WayPoint {
            Locator* loc;
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
        WayPoint mpWayPoints[MAX_WAYPOINTS];
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

    struct MissionCondition {
        enum ConditionTypeEnum {
            COND_INVALID,
            COND_VEHICLE_DAMAGE,
            COND_PLAYER_HIT,
            COND_TIME_OUT,
            COND_PLAYER_OUT_OF_VEHICLE,
            COND_FOLLOW_DISTANCE,
            COND_OUT_OF_BOUNDS,
            COND_RACE,
            COND_LEAVE_INTERIOR,
            COND_POSITION,
            COND_CARRYING_STATEPROP_COLLECTIBLE,
            COND_NOT_ABDUCTED,
            COND_HIT_AND_RUN_CAUGHT,
            COND_KEEP_BARREL,
            COND_GET_COLLECTIBLES
        };
        void** vfptr;
        ConditionTypeEnum mType;
        bool mbIsViolated;
        bool mLeaveInteror;
        char pad[2];
        static bool* mFailedHitNRun;
    };
    bool* MissionCondition::mFailedHitNRun = reinterpret_cast<bool*>(0x0);

    struct VehicleCondition : MissionCondition {
        Vehicle* mpVehicle;
    };

    struct FollowCondition : VehicleCondition {
        float mMinDistance;
        float mMaxDistance;
        unsigned int muTime;
    };

    struct CameraInfo {
        SuperCam::Type type;
        bool cut;
        bool quickTransition;
        bool active;
        char pad[1];
    };
    struct ChaseVehicleStruct {};
    struct SafeZone {};
    struct LockRequirement {};

    struct MissionStage {
        static constexpr int MAX_CONDITIONS = 8;
        enum MissionStageState : int {
            STAGE_IDLE,
            STAGE_INPROGRESS,
            STAGE_COMPLETE,
            STAGE_FAILED,
            STAGE_BACKUP,
            STAGE_ALL_COMPLETE
        };
        enum StageTimeType : int {
            STAGETIME_NOT_TIMED,
            STAGETIME_ADD,
            STAGETIME_SET
        };
        enum { MAX_CHARACTERS_IN_STAGE = 6 };
        struct VehicleInfo {
            Vehicle* vehicle;
            struct CarStartLocator* spawn;
            int vehicleAINum;
            VehicleAI* vehicleAI;
        };
        struct CharacterInfo {
            char name[16];
            struct Character* character;
            struct CarStartLocator* locator;
            struct CarStartLocator* carLocator;
            struct ZoneEventLocator* pZoneEventLocator;
            Vehicle* vehicle;
            char VehicleName[16];
        };
        char pad[0x60];
        bool mbStayBlackForStage;
        bool mbDisablePlayerControlForCountDown;
        char pad2[2];
        MissionStageState mState;
        struct MissionObjective* mObjective;
        int mNumConditions;
        MissionCondition* mConditions[MAX_CONDITIONS];
        StageTimeType mStageTimeType;
        int mStageTime;
        int mNumVehicles;
        VehicleInfo mVehicles[4];
        int mNumWaypoints;
        Locator* mWaypoints[32];
        int mNumCharacters;
        CharacterInfo mCharacters[MAX_CHARACTERS_IN_STAGE];
        char mCharacterToHide[16];
        bool mbLevelOver : 1;
        bool mbGameOver : 1;
        bool mbFinalStage : 1;
        int miNameIndex;
        int miStartMessageIndex;
        bool mbShowMessage;
        char pad3[3];
        CameraInfo mCamInfo;
        bool mMusicChange;
        bool mKeepMusicOn;
        char pad4[2];
        unsigned int mMusicEventKey;
        unsigned int mMusicStateKey;
        unsigned int mMusicStateEventKey;
        unsigned int mDialogEventKey;
        std::uint64_t mConversationCharacterKey;
        ChaseVehicleStruct m_ChaseData_Array[1];
        SafeZone* m_SafeZone_Array[3];
        bool mbClearTrafficForStage;
        bool mbNoTrafficForStage;
        bool mb_DisableHitAndRun;
        bool mb_InsideSafeZone;
        bool mb_UseElapsedTime;
        char pad5[3];
        int mRaceEntryFee;
        bool mbPutPlayerInCar;
        bool mbRacePaidOut;
        bool mbSwapInDefaultCar;
        char pad6[1];
        char mSwapDefaultCarRespawnLocatorName[32];
        char mSwapForceCarRespawnLocatorName[32];
        char mSwapPlayerRespawnLocatorName[32];
        char mPlayerRespawnLocatorName[32];
        char mPlayerCarRespawnLocatorName[32];
        char mTrafficDensity;
        bool mIsBonusObjectiveStart;
        bool mStartBonusObjectives;
        char pad7[1];
        LockRequirement mRequirement[2];
        bool mMissionLocked;
        bool mShowStageComplete : 1;
        bool mCountdownEnabled : 1;
        char pad8[1];
        char mHUDIconImage[16];
        bool mIrisAtEnd;
        bool mFadeOutAtEnd;
        char pad9[2];
        float mIrisSpeed;
        struct CountdownSequenceUnit* mCountdownSequenceUnits;
        int mNumCountdownSequenceUnits;
        std::uint64_t mSecondSpeakerUID;
        bool mAllowMissionAbort : 1;
        char pad10[3];
        int mResetCounter;
    };

    struct Mission {
        enum : int { MAX_BONUS_OBJECTIVES = 3 };
        enum MissionState {
            STATE_WAITING,
            STATE_INPROGRESS,
            STATE_FAILED,
            STATE_SUCCESS
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
        MissionStage::MissionStageState mLastStageState;
        CarStartLocator* mVehicleRestart;
        Locator* mPlayerRestart;
        struct ZoneEventLocator* mDynaloadLoc;
        ZoneEventLocator* mStreetRacePropsLoad;
        ZoneEventLocator* mStreetRacePropsUnload;
        int mResetToStage;
        bool mSundayDrive : 1;
        bool mBonusMisison : 1;
        char pad2[3];
        struct BonusObjective* mBonusObjectives[MAX_BONUS_OBJECTIVES];
        unsigned int mNumBonusObjectives;
        bool mIsForcedCar;
        bool mbSwappedCars;
        bool mbCarryOverOutOfCarCondition;
        bool mbTriggerPattyAndSelmaScreen;
        int mFinalDelay;
        int mCompeteDelay;
        bool mChangingStages;
        bool mNoTimeUpdate;
        bool mJumpBackStage;
        char mJumpBackBy;
        int mNumStatePropCollectibles;
        struct StatePropCollectible** mStatePropCollectibles;
        struct AnimatedIcon* mDoorStars;
        int mInitPedGroupId;
        bool mShowHUD : 1;
        char pad3[3];
        int mNumValidFailureHints;
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
        char pad[0x120];
        IDirect3DDevice8* d3dDevice;
    };

#pragma pack(pop)

}
