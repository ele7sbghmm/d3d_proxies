#pragma once
#pragma pack(push, 1)

#include <vector>
#include <d3dx8.h>

namespace Shar {
	struct SpatialNode;
	struct PathSegment;
	struct tShader;
	struct Road;
	struct RoadSegment;
	template <typename T> struct ContiguousBinNode;
	struct tDrawable;
	struct SpatialTreeIter;
	struct WorldSphereDSG;
	struct DynaLoadListDSG;

	using pddiMatrix = rmt::Matrix;
	using tMark = int;

	struct tUidUnaligned { unsigned int u0, u1; };
	struct tName { tUID uid; };
	using radKey = tUidUnaligned;
	using tUID = radKey;

	using pddiColour = tColour;
	struct tColour { unsigned c; };

	struct IRefCount { uintptr_t vfptr; };
	struct radLoadObject : IRefCount { unsigned int m_refCount; };
	struct tRefCounted : radLoadObject {};
	struct tEntity : tRefCounted { tName name; };

	namespace rmt {
		struct Vector { float x, y, z; };
		struct Vector4 { float x, y, z, w; };
		struct Matrix { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
		struct Box3D { Vector low, high; };
		struct Sphere { Vector center; float radius; };
	}

	struct ISpatialProxyAA { uintptr_t vfptr; };
	struct Vector3f : rmt::Vector {};
	struct Bounds3f { Vector3f mMin, mMax; };
	struct BoxPts : ISpatialProxyAA { Bounds3f mBounds; };
	struct SphereSP : BoxPts { rmt::Vector mCenter; float mRadius; };
	struct AAPlane3f { char mAxis; float mPosn; };

	template <typename T> struct SwapArray {size_t mSize, mUseSize; T* mpData; void* mSwapT; };
	template <typename T> struct NodeSwapArray { size_t mSize, mUseSize; T* mpData; void* mSwapT; };
	template <typename T> struct ReserveArray { size_t mSize, mUseSize; T* mpData; };
	template <typename T> struct FixedArray { size_t mSize; T* mpData; };

	struct tDrawable : tRefCounted { struct ShaderCallback {}; };
	struct IEntityDSG : tEntity {
		float			mRank;
		bool			mTranslucent;
		char			pad[3];
		tName			mShaderName;
		SpatialNode*	mpSpatialNode;
	};

	using radMemoryAllocator = int;
	using voidp = void*;
	namespace sim {
		struct VoidPList {
			uintptr_t			vfptr;
			bool				mLock;
			char				pad[3];
			size_t				mSize;
			size_t				mArraySize;
			voidp*				mArray;
			radMemoryAllocator	mAllocator;
		};
		template <typename T> struct TList : VoidPList {};
		template <typename T> struct TArray {
			int					mSize;
			int					mArraySize;
			T*					mArray;
			bool				mLock;
			bool				mDontUseMemCpy;
			char				pad[2];
			radMemoryAllocator	mAllocator;
		};
	}

	namespace sim {
		namespace rmt = ::Shar::rmt;
		using ::Shar::tEntity;
		using ::Shar::tRefCounted;
		struct SimState;
		struct CollisionObject;
		struct CollisionVolume;

		enum CollisionVolumeTypeEnum {
			/*0*/ CollisionVolumeType,
			/*1*/ SphereVolumeType,
			/*2*/ ylinderVolumeType,
			/*3*/ BBoxVolumeType,
			/*4*/ WallVolumeType,
			/*5*/ BBoxVolumeType,
			/*6*/ MaxCollisionVolumeType
		};

		struct SimUnits { /**/ };
		struct PhysicsProperties : tEntity, SimUnits { /**/ };
		struct CollisionVolume : tRefCounted {
			rmt::Vector					mPosition;
			rmt::Vector					mBoxSize;
			float						mSphereRadius;
			CollisionVolumeTypeEnum		mType;
			int							mObjRefIndex;
			int							mOwnerIndex;
			CollisionObject*			mCollisionObject;
			TList<CollisionVolume*>*	mSubVolumeList;
			bool						mVisible;
			bool						mUpdate;
			char						pad[2];
			rmt::Vector					mDP;
			tColour						mColour;
		};
		using CollisionAnalyserPossibleEvent = int;
		struct CollisionObject : tEntity {
			struct CollisionVolumeOwner {
				int				mNumOwner;
				TArray<tUID>*	mOwnerList;
				TArray<bool>	mVisible; 
			};
			struct SelfCollision {
				int					mIndex1;
				int					mIndex2;
				bool				mSelf1;
				bool				mSelf2;
				char				pad[2];
				CollisionVolume*	mCollisionVolume1;
				CollisionVolume*	mCollisionVolume2;
			};

			tName							mStringData;
			SimState*						mSimState;
			PhysicsProperties*				mPhysicsProperties;
			CollisionVolume*				mCollisionVolume;
			int								mNumJoint;
			TArray<SelfCollision>			mSelfCollisionList;
			CollisionVolumeOwner			mCollisionVOlumeOwner;
			bool							mIsStatic : 1;
			bool							mCollidedWithStatic : 1;
			char							pad[3];
			int								mDefaultArea;
			bool							mHasMoved : 1;
			bool							mAutoPair : 1;
			bool							mManualUpdate : 1;
			bool							mCollisionEnabled : 1;
			bool							mSelfCollisionEnabled : 1;
			bool							mRayCastingEnabled : 1;
			char							pad2[3];
			CollisionAnalyserPossibleEvent	mPossibleCollisionEvents;
		};
		struct SimVelocityState {
			rmt::Vector	mLinear;
			rmt::Vector	mAngular;
		};
		struct SimWind : tRefCounted { 
			rmt::Vector	mWindseed;
			rmt::Vector	mWind;
		};

		struct SimEnvironment : tEntity, SimUnits {
			rmt::Vector	mGravityCGS;
			rmt::Vector	mGravity;
			float		mGravityNorm;
			float		mViscosityCGS;
			float		mViscosity;
			SimWind*	mSimWind;
		};
		enum SimulatedObjectTypeEnum {
			/*0*/RigidObjectType,
			/*1*/ArticulatedObjectType,
			/*2*/FlexibleObjectType,
			/*3*/MaxPhObjEnum
		};
		struct SimulatedObject : tEntity {
			SimState*				mSimState;
			float					mProgressTime;
			PhysicsProperties*		mPhysicsProperties;
			SimEnvironment*			mSimEnvironment;
			bool					mUseRestingDetector;
			char					pad[3];
			SimulatedObjectTypeEnum	mType;
			int						mRefIndex;
		};
		struct VirtualCM {};
		enum SimControlEnum {
			/*0*/simAICtrl = 0,
			/*1*/simSimulationCtrl
		};
		struct SimState : tRefCounted {
			void*				mAIRefPointer;
			int					mAIRefIndex;
			rmt::Matrix			mTransform;
			SimVelocityState	mVelocityState;
			float				mScale;
			SimControlEnum		mContorl;
			SimulatedObject*	mSimulatedObject;
			CollisionObject*	mCollisionObject;
			VirtualCM*			mVirtualCM;
			bool				mObjectMoving;
			char				pad[3];
			float				mSafeTimeBeforeCollision;
			float				mApproxSpeedMagnitude;
			bool				mArticulated;
			char				pad2[3];
		};
		struct SimStateArtiuclated : SimState {};
		enum VolAxisOrientation {
			/*0*/VolAxisNotOriented = 0,
			/*1*/VolAxisOriented,
			/*2*/VolAxisXOriented,
			/*3*/VolAxisYOriented,
			/*4*/VolAxisZOriented
		};
		struct SphereVolume : CollisionVolume {
			float	mSphereRadiusRef;
		};
		struct CylinderVolume : CollisionVolume {
			rmt::Vector			mAxis;
			float				mLength;
			float				mCylinderRadius;
			bool				mFlatEnd;
			char				pad[3];
			VolAxisOrientation	mAxisOrientation;
			rmt::Vector			mAxisRef;
			float				mLengthRef;
			float				mCylinderRadiusRef;
		};
		struct OBBoxVolume : CollisionVolume {
			rmt::Vector			mAxis[3];
			float				mLength[3];
			VolAxisOrientation	mAxisOrientation;
			rmt::Vector			mAxisRef[3];
			float				mLengthRef[3];
		};
		struct WallVolume : CollisionVolume {
			rmt::Vector			mNormal;
			VolAxisOrientation	mAxisOrientation;
			rmt::Vector			mNormalRef;
		};
		struct BBoxVolume : CollisionVolume {};
	}

	struct RenderEnums {
		enum LayerEnum {
			/*0*/GUI = 0x00000000,
			/*1*/PresentationSlot,
			/*2*/LevelSlot,
			/*3*/MissionSlot1,
			/*4*/MissionSlot2,
			/*5*/umLayers,
			/*255*/LayerOnlyMask = 0x000000FF
		};
		enum UserDataEnum {};
		enum LoadZoneEnum {};
		enum GutsCallEnum {};
		enum LevelEnum {
			/*0*/L1,
			/*1*/L2,
			/*2*/L3,
			/*3*/L4,
			/*4*/L5,
			/*5*/L6,
			/*6*/L7,
			/*7*/numLevels,
			/*7*/B00 = numLevels,
			/*8*/B01,
			/*9*/B02,
			/*10*/B03,
			/*11*/B04,
			/*12*/B05,
			/*13*/B06,
			/*14*/B07,
			/*15*/MAX_LEVEL
		};
		enum LevelMissionCountEnum {};
		enum MissionEnum {
			M1/*0*/,
			M2/*1*/,
			M3/*2*/,
			M4/*3*/,
			M5/*4*/,
			M6/*5*/,
			M7/*6*/,
			M8/*7*/,
			M9/*8*/,
			M10/*9*/,
			numMissions/*10*/
		};
	};
	struct RenderManager {

	};
	struct tView {};
	enum eExportedState {
		msDead/*0*/,
		msFrozen/*1*/,
		msRenderReady/*2*/
	};
	const int MAX_PLAYERS = 4;
	struct RenderLayer {
		eExportedState			mExportedState;
		eExportedState			mPreviousState;
		tView*					mpView[MAX_PLAYERS];
		float					mAlpha;
		SwapArray<tDrawable*>	mpGuts;
		bool					mIsBeginView;
		char					pad[3];
		unsigned int			mNumViews;
	};
	enum DynaLoadState {
		msPreLoads/*0*/, msNoLoad/*1*/, msLoad/*2*/, msIgnoreLoad/*3*/ 
	};

	struct EventListener {
		uintptr_t vfptr;
	};
	struct SpatialTree : tEntity {
		FixedArray<ContiguousBinNode<SpatialNode>>	mTreeNodes;
		Bounds3f									mTreeBounds;
	};
	struct WorldScene : EventListener {
		struct zSortBlah {
			IEntityDSG*	entityPtr;
			tUID		shaderUID;
		};
		enum {
			msClear /*= ~SpatialTreeIter::msFilterAll*/, msVisible0 = 0x01, msVisible1 = 0x02, msVisible2 = 0x04, msVisible3 = 0x08, msStaticPhys = 0x0f, msDynaPhys = 0x10
		};
		SpatialTreeIter				mStaticTreeWalker;
		rmt::Vector					mEpsilonOffset;
		float						mDrawDist;
		bool						mRenderAll;
		char						pad[3];
		SpatialTree*				mpStaticTree;
		tShader*					mpTempShader;
		std::vector<IEntityDSG*>	mpZSortsPassShadowCasters;
		std::vector<zSortBlah>		mpZSorts;
		std::vector<IEntityDSG*>	mpZSortsPass2;
		ReserveArray<IEntityDSG*>	mShadowCastersPass1;
		FixedArray<rmt::Vector4>	mCamPlanes;
		rmt::Vector					mViewVector;
		rmt::Vector					mViewPosn;
		float						mViewSinInv;
		float						mViewSinSqr;
		float						mViewCosSqr;
	};

	struct DynaLoadListDSG {
		tName								mGiveItAFuckinName;
		SwapArray<WorldSphereDSG*>			mWorldSphereElems;
		SwapArray<StaticEntityDSG*>			mSEntityElems;
		SwapArray<StaticPhysDSG*>			mSPhysElems;
		SwapArray<IntersectDSG*>			mIntersectElems;
		SwapArray<DynaPhysDSG*>				mDPhysElems;
		SwapArray<FenceEntityDSG*>			mFenceElems;
		SwapArray<AnimCollisionEntityDSG*>	mAnimCollElems;
		SwapArray<AnimEntityDSG*>			mAnimElems;
		SwapArray<TriggerVolume*>			mTrigVolElems;
		SwapArray<RoadSegment*>				mRoadSegmentElems;
		SwapArray<PathSegment*>				mPathSegmentElems;
	};
	struct WorldRenderLayer : RenderLayer {
		WorldScene*					mpWorldScene;
		SwapArray<WorldSphereDSG*>	mWorldSpheres;
		SwapArray<DynaLoadListDSG>	mStaticLoadLists;
		SwapArray<DynaLoadListDSG*>	mpLoadLists;
		int							mnLoadListRefs;
		int							mCurLoadIndex;
		DynaLoadState				mDynaLoadState;
		tUID						mCurLoadUID;
		bool						mQdDump;
		char						pad[3];
		unsigned int				mQdDeletionStart;
		SwapArray<tRefCounted*>*	mpQdDeletionList;
		bool						mMirror;
		char						pad2[3];
		rmt::Matrix					mMirrorMatrix;
	};

	enum enClasstypeID {};
	namespace ParticleEnum {
		enum ParticleID {};
	}
	namespace BreakablesEnum {
		enum BreakableID {};
	}
	class CollisionAttributes : tRefCounted {
		char						mp_Sound[32];
		char						mp_Animation[32];
		ParticleEnum::ParticleID	mp_Particle;
		BreakablesEnum::BreakableID	mBreakableID;
		float						mMass;
		float						mFriction;
		float						mElasticity;
		enClasstypeID				mClasstypeid;
		sim::PhysicsProperties*		mPhizProp;
	};
	struct CollisionEntityDSG {
		short					mPersistentObjectID;
		char					pad[2];
		unsigned				lastUpdate;
		CollisionAttributes*	mpCollisionAttributes;
		bool					mWasParticleEffectTriggered;
		char					pad2[3];
		RenderEnums::LayerEnum	mRenderLayer;
	};

	struct Smoother {
		float	rollingAverage;
		float	factor;
	};
	struct tCompositeDrawable {
		struct DrawableEffectElement {};
	};
	struct tGeometry {};
	struct tBillboardQuadGroup {};
	struct tMultiController {};
	struct LensFlareDSG {};
	struct WorldSphereDSG : IEntityDSG {
		bool							mbActive;
		char							pad[3];
		rmt::Vector						mPosn;
		tCompositeDrawable*				mpCompDraw;
		SwapArray<tGeometry*>			mpGeos;
		SwapArray<tBillboardQuadGroup*>	mpBillBoards;
		tMultiController*				mpMultiCon;
		LensFlareDSG*					mpFlare;
	};
	struct StaticEntityDSG :IEntityDSG {
		rmt::Vector	mPosn;
		int			mIsGeo;
		tDrawable*	mpDrawstuff;
	};
	struct StaticPhysDSG : CollisionEntityDSG {
		rmt::Box3D		mBBox;
		rmt::Sphere		mSphere;
		rmt::Vector		mPosn;
		sim::SimState*	mpSimStateObj;
		tDrawable*		mpShadow;
		rmt::Matrix*	mpShadowMatrix;
	};
	struct IntersectDSG : IEntityDSG {
		ReserveArray<int>			mTriIndices;
		ReserveArray<rmt::Vector>	mTriPts;
		ReserveArray<rmt::Vector>	mTriNorms;
		ReserveArray<unsigned char>	mTerrainType;
		int							mnPrimGroups;
		rmt::Box3D					mBox3D;
		rmt::Sphere					mSphere;
		rmt::Vector					mPosn;
	};
	struct DynaPhysDSG : StaticPhysDSG {
		bool		mIsHit;
		char		pad[3];
		Smoother	mPastLinear;
		Smoother	mPastAngular;
		int			mGroundPlaneIndex;
		int			mGroundPlaneRefs;
	};
	struct FenceEntityDSG : CollisionEntityDSG {
		rmt::Vector	mStartPoint;
		rmt::Vector	mEndPoint;
		rmt::Vector	mNormal;
	};
	struct poser {
		struct PoseEngine {};
	};
	struct RootMatrixDriver {};
	struct ActionButton {
		struct AnimSwitch {};
	};
	struct AnimCollisionEntityDSG : CollisionEntityDSG {
		tCompositeDrawable*			mpDrawable;
		sim::SimStateArtiuclated*	mpSimStateArticulated;
		tMultiController*			mpAnimController;
		poser::PoseEngine*			mpPoseEngine;
		RootMatrixDriver*			mpRootMatrixDriver;
		rmt::Matrix					mTransform;
		float						mfDirection;
		ActionButton::AnimSwitch*	mpActionButton;
		rmt::Box3D					mBoundingBox;
		rmt::Sphere					mBoundingSphere;
	};
	struct AnimEntityDSG : IEntityDSG {
		bool					mbAddToUpdate;
		char					pad[3];
		int						mTrackSeparately;
		tCompositeDrawable*		mpDrawable;
		rmt::Matrix				mTransform;
		rmt::Matrix				mTransformOffset;
		tMultiController*		mpMultiController;
		rmt::Box3D				mBoundingBox;
		rmt::Sphere				mBoundingSphere;
		bool					mIsVisible : 1;
		bool					mIsAnimationPlaying : 1;
		bool					debugflag : 1;
		char					pad[3];
		SwapArray<tCompositeDrawable::DrawableEffectElement*>	mEffectElements;
		RenderEnums::LayerEnum	mRenderLayer;
		rmt::Box3D*				mParticleSystemBoundingBox;
		unsigned int			m_TimeLastParticleUpdate;
		float					mTimeSinceLastUpdate;
		unsigned				updateIndex;
	};
	struct TriggerVolume {};
	struct Intersection {};
	struct ILaneInformation {};
	struct ILaneControl {};
	struct TrafficVehicle {};
	struct Lane : ILaneInformation, ILaneControl{
		TrafficVehicle*				mWaitingVehicle;
		SwapArray<TrafficVehicle*>	mTrafficVehicles;
		Road*						mpParentRoad;
		float						mfSpeedLimit;
		int							mDesiredDensity;
	};
	struct Road {
		Intersection*	mpSourceIntersection;
		Intersection*	mpDestinationIntersection;
		Lane*			mLaneList;
		unsigned int	mnLanes;
		RoadSegment**	mppRoadSegmentArray;
		unsigned int	mnMaxRoadSegments;
		unsigned int	mmnRoadSegments;
		rmt::Box3D		mBox;
		rmt::Sphere		mSphere;
		unsigned int	mSpeed;
		unsigned int	mDestiny;
		unsigned int	mDifficulty;
		bool			mIsShortCut;
		char			pad[3];
		float			mLength;
	};
	struct RoadSegment : IEntityDSG {
		Road*		mRoad;
		unsigned	mSegmentIndex;
		rmt::Vector	mCorners[4];
		rmt::Vector	mEdgeNormals[4];
		rmt::Vector	mNormal;
		float		mfSegmentLength;
		float		mfLaneWidth;
		float		mfRadius;
		float		mfAngle;
		rmt::Sphere	mSphere;
	};
	struct Path {
		bool			mIsClosed;
		char			pad[3];
		int				mNumPathSegments;
		int				mNumPeds;
		PathSegment**	mPathSegments;
	};
	struct PathSegment : IEntityDSG {
		rmt::Vector	mStartPos;
		rmt::Vector	mEndPos;
		Path*		mParentPath;
		int			mIndexToParentPath;
		float		mRadius;
	};

	template <typename T>
	struct ContiguousBinNode {
		T	mpData;
		int	mSubTreeSize;
		int	mParentOffset;
	};
	struct SpatialNode {
		AAPlane3f							mSubDivPlane;
		SwapArray<StaticEntityDSG*>			mSEntityElems;
		SwapArray<StaticPhysDSG*>			mSPhysElems;
		SwapArray<DynaPhysDSG*>				mDPhysElems;
		SwapArray<IntersectDSG*>			mIntersectElems;
		SwapArray<FenceEntityDSG*>			mFenceElems;
		SwapArray<AnimCollisionEntityDSG*>	mAnimCollElems;
		SwapArray<AnimEntityDSG*>			mAnimElems;
		SwapArray<TriggerVolume*>			mTrigVolElems;
		SwapArray<RoadSegment*>				mRoadSegmentElems;
		SwapArray<PathSegment*>				mPathSegmentElems;
		BoxPts								mBBox;
	};
	struct SpatialTreeIter {
		SwapArray<SpatialNode*>			mCurNodes;
		SwapArray<SpatialNode*>			mCurAlwaysVisNodes;
		SwapArray<SpatialNode*>*		mpCurNodeList;
		ContiguousBinNode<SpatialNode>*	mpRootNode;
		ContiguousBinNode<SpatialNode>*	mpCurNode;
		int								mCurNodeOffset;
		FixedArray<int>					mNodemarks;
		int								mCurMarkFilter, mCurNodeI;
		BoxPts							mBBox;
	};


	enum pddiPrimType {
		PDDI_PRIM_TRIANGLES,
		PDDI_PRIM_TRISTRIP,
		PDDI_PRIM_LINES,
		PDDI_PRIM_LINESTRIP,
		PDDI_PRIM_POINTS
	};
	enum pddiPixelFormat {
		PDDI_PIXEL_UNKNOWN,
		PDDI_PIXEL_RGB565,
		PDDI_PIXEL_ARGB1555,
		PDDI_PIXEL_RGB555,
		PDDI_PIXEL_ARGB4444,
		PDDI_PIXEL_RGB888,
		PDDI_PIXEL_ARGB8888,
		PDDI_PIXEL_PAL8,
		PDDI_PIXEL_PAL4,
		PDDI_PIXEL_LUM8,
		PDDI_PIXEL_DUDV88,
		PDDI_PIXEL_DXT1,
		PDDI_PIXEL_DXT2,
		PDDI_PIXEL_DXT3,
		PDDI_PIXEL_DXT4,
		PDDI_PIXEL_DXT5,
		PDDI_PIXEL_Z32,
		PDDI_PIXEL_Z24,
		PDDI_PIXEL_Z16,
		PDDI_PIXEL_Z8,
		PDDI_PIXEL_PS2_4BIT,
		PDDI_PIXEL_PS2_8BIT,
		PDDI_PIXEL_PS2_16BIT,
		PDDI_PIXEL_PS2_32BIT,
		PDDI_PIXEL_GC_4BIT,
		DDI_PIXEL_GC_8BIT,
		PDDI_PIXEL_GC_16BIT,
		PDDI_PIXEL_GC_32BIT,
		PDDI_PIXEL_GC_DXT1
	};
	enum pddiTextureType {
		PDDI_TEXTYPE_RGB,
		PDDI_TEXTYPE_PALETTIZED,
		PDDI_TEXTYPE_LUMINANCE,
		PDDI_TEXTYPE_BUMPMAP,
		DDI_TEXTYPE_DXT1,
		PDDI_TEXTYPE_DXT2,
		PDDI_TEXTYPE_DXT3,
		PDDI_TEXTYPE_DXT4,
		PDDI_TEXTYPE_DXT5,
		PDDI_TEXTYPE_IPU,
		PDDI_TEXTYPE_Z,
		PDDI_TEXTYPE_LINEAR,
		PDDI_TEXTYPE_RENDERTARGET,
		PDDI_TEXTYPE_PS2_4BIT,
		PDDI_TEXTYPE_PS2_8BIT,
		PDDI_TEXTYPE_PS2_16BIT,
		PDDI_TEXTYPE_PS2_32BIT,
		PDDI_TEXTYPE_GC_4BIT,
		PDDI_TEXTYPE_GC_8BIT,
		PDDI_TEXTYPE_GC_16BIT,
		PDDI_TEXTYPE_GC_32BIT,
		PDDI_TEXTYPE_GC_DXT1
	};
	enum pddiTextureUsageHint {
		PDDI_USAGE_STATIC,
		PDDI_USAGE_DYNAMIC,
		PDDI_USAGE_NOCACHE
	};

	const int MAX_TEXTUREFORMATS = 32;
	const int PDDI_MAX_LIGHTS = 8;
	const int PDDI_MAX_STATS = 40;
	const int PDDI_MAX_MATRIX_STACKS = 5;

	struct pddiObject {
		uintptr_t	vfptr;
		int			refCount;
		int			lastError;
	};
	struct pddiShader : pddiObject {};
	struct tShader : tEntity {
		bool		mTranslucent;
		char		pad[3];
		pddiShader*	shader;
	};

	struct tInventory { /**/ };
	struct tLoadManager { /**/ };
	struct tMatrixStack { /**/ };
	struct pddiDevice : pddiObject { /**/ };
	struct pddiRenderContext : pddiObject { /**/ };
	struct pddiDisplay : pddiObject {
		bool	m_ForceVSync;
		bool	m_only60;
		char	pad[2];
	};
	struct pddiExtHardwareSkinning { /**/ };
	struct pddiExtVertexProgram { /**/ };
	struct tContext {
		tInventory*					Inventory;
		tLoadManager*				loadManager;
		unsigned					FrameCount;
		tMatrixStack*				stack;
		tView*						View;
		rmt::Matrix					CVM;
		rmt::Matrix					CVMI;
		rmt::Matrix					CWM;
		tColour						clearColour;
		float						clearDepth;
		unsigned					clearStencil;
		unsigned					clearMask;
		int							nLights;
		pddiDevice*					RenderDevice;
		pddiRenderContext*			RenderContext;
		pddiDisplay*				RenderDisplay;
		pddiExtHardwareSkinning*	skinning;
		pddiExtVertexProgram*		vertexProgram;
		bool						inFrame;
		char						pad[3];
	};

	struct pddiPrimBufferStream { /**/ };
	struct pddiPrimBuffer : pddiObject { /**/ };
	struct pddiLibInfo {
		int		versionMajor;
		int		versionMinor;
		int		versionBuild;
		int		libID;
		char	description[256];
	};
	typedef struct {
		int	width;
		int	height;
		int	bpp;
	} pddiModeInfo;
	struct pddiDisplayInfo {
		int				id;
		char			description[256];
		unsigned		pci;
		unsigned		vendor;
		bool			fullscreenOnly;
		char			pad[3];
		unsigned		caps;
		int				nDisplayModes;
		pddiModeInfo*	modeInfo;
	};
	struct pddiVertexComponentWidth {
		unsigned char	uv0;
		unsigned char	uv1;
		unsigned char	uv2;
		unsigned char	uv3;
		unsigned char	uv4;
		unsigned char	uv5;
		unsigned char	uv6;
		unsigned char	uv7;
		unsigned char	position;
		unsigned char	pad[3];
	};
	struct pddiPrimBufferDesc {
		pddiPrimType				primType;
		unsigned					vertexFormat;
		pddiVertexComponentWidth	componentWidth;
		unsigned					nVertex;
		unsigned					nIndex;
		bool						memoryImaged;
		bool						deformed;
		char						pad[2];
		char*						vertexProgram;
		unsigned					matrixCount;
	};
	struct pddiLockInfo {
		int				width;
		int				height;
		int				depth;
		int				volDepth;
		pddiPixelFormat	format;
		bool			native;
		char			pad[3];
		int				rgbaLShift[4];
		int				rgbaRShift[4];
		unsigned		rgbaMask[4];
		int				pitch;
		int				slice;
		void*			bits;
		void*			palette;
	};
	struct pddiTextureDesc {
		int						xSize;
		int						ySize;
		int						zSize;
		int						bpp;
		int						alphaDepth;
		int						nMip;
		pddiTextureType			type;
		pddiTextureUsageHint	usageHind;
		bool					volume;
		bool					memImage;
	};
	
	template <typename T>
	struct pddiStack { int	depth; int	top;
		T*	stack;
	};
	struct pddiMatrixStack : pddiStack<pddiMatrix> {};
	struct pddiViewState { /**/ };
	struct pddiRenderState { /**/ };
	struct pddiLightingState { /**/ };
	struct pddiFogState { /**/ };
	struct pddiStencilState { /**/ };
	struct pddiContextState {
		unsigned						currentFrame;
		unsigned						inFrame;
		unsigned						contextFlags;
		pddiMatrixStack*				matrixStack[PDDI_MAX_MATRIX_STACKS];
		pddiStack<pddiViewState>*		viewStateStack;
		pddiStack<pddiRenderState>*		renderStateStack;
		pddiStack<pddiLightingState>*	lightingStateStack;
		pddiStack<pddiFogState>*		fogStateStack;
		pddiStack<pddiStencilState>*	stencilSTateStack;
		pddiViewState*					viewState;
		pddiRenderState*				renderState;
		pddiLightingState*				lightingState;
		pddiFogState*					fogState;
		pddiStencilState*				stencilState;
	};
	struct pddiFont { /**/ };
	struct pddiBaseContext {
		pddiDisplay*		display;
		pddiDevice*			device;
		pddiContextState	state;
		float				stats[PDDI_MAX_STATS];
		pddiFont*			font;
		bool				displayStats;
		bool				supressPrimTally;
		char				pad[2];
	};
	struct d3dSurface {};
	enum pddiDisplayMode {
		PDDI_DISPLAY_FULLSCREEN,
		PDDI_DISPLAY_WINDOW,
		PDDI_DISPLAY_FULLSCREEN_PAL
	};

	struct pddiDisplayInit {
		int				xsize;
		int				ysize;
		int				bpp;
		unsigned		bufferMask;
		int				nColourBuffer;
		int				nSamples;
		pddiDisplayMode	displayMode;
		bool			lockToVsync;
		bool			asyncSwap;
		bool			enableSnapshot;
	};
	struct d3dDisplay : pddiDisplay {
		LPDIRECT3D8				d3d;
		LPDIRECT3DDEVICE8		d3dDevice;
		D3DPRESENT_PARAMETERS	d3dpp;
		D3DCAPS8				d3dCaps;
		D3DGAMMARAMP			initialGammaRamp;
		D3DFORMAT				colourBufferFormat;
		D3DFORMAT				depthBufferFormat;
		bool					reset;
		bool					lockToVsync;
		bool					asyncSwap;
		char					pad[1];
		d3dSurface*				backBuffer;
		pddiDisplayInit			displayInit;
		int						nColourBuffer;
		unsigned				bufferMask;
		int						displayHeight;
		int						displayWidth;
		int						displayDepth;
		float					gammaR;
		float					gammaG;
		float					gammaB;
		unsigned				rtHeight;
		unsigned				rtWidth;
		LPDIRECT3DTEXTURE8		snapshot;
		bool					widescreen;
		char					pad2[3];
	};
	struct d3dDevice : pddiDevice {
		pddiRenderContext*	displayInfo;
		int					nDisplay;
		pddiDisplayInfo*	displayInfo;
		LPDIRECT3D8			direct3D;
	};
	struct d3dState { /**/ };
	struct pddiState { /**/ };
	struct pddiCullMode { /**/ };
	struct pddiCompareMode { /**/ };
	struct pddiFillMode { /**/ };
	struct pddiRenderState : pddiState {
		pddiCullMode	cullMode;
		bool			zEnbaled;
		char			pad[3];
		pddiCompareMode	zCompare;
		bool			zWrite;
		char			pad2[3];
		pddiFillMode	fillMode;
		bool			redWrite;
		bool			greenWrite;
		bool			blueWrite;
		bool			alphaWrite;
	};
	struct d3dDevCaps { /**/ };
	struct d3dShader { /**/ };
	struct d3dPrimStream { /**/ };
	struct d3dVertexProgramManager { /**/ };
	struct d3dExtGammaControl { /**/ };
	struct d3dExtAntialiasControl { /**/ };
	struct d3dExtGlobalProjectedTexture { /**/ };
	struct d3dExtReadPixels { /**/ };
	struct d3dContext : pddiBaseContext {
		bool							polyClear;
		char							pad[3];
		d3dState*						d3dstate;
		d3dDisplay*						device;
		d3dDevCaps						devCaps;
		d3dShader*						defaultShader;
		LPDIRECT3DDEVICE8				d3d;
		LPDIRECT3DSURFACE8				zBuffer;
		D3DVIEWPORT8					viewPort;
		D3DFORMAT						validTextures[MAX_TEXTUREFORMATS];
		d3dPrimStream*					stream;
		d3dVertexProgramManager*		vertprogs;
		pddiMatrix						lightViewMatrix[PDDI_MAX_LIGHTS];
		bool							lightStateChanged[PDDI_MAX_LIGHTS];
		bool							anyLightStateChanged;
		char							pad2[2];
		__int64							beginTime;
		float							ticksToMs;
		d3dExtGammaControl*				extGammaControl;
		d3dExtAntialiasControl*			extAntialiasControl;
		d3dExtGlobalProjectedTexture*	extGlobalProjectedTexture;
		d3dExtReadPixels*				extReadPixels;
	};
}

#pragma pack(pop)