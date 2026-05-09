#pragma once

#include <vector>
#include <stack>
#include <d3dx8.h>

#pragma pack(push, 1)
/* std::vector size changes depending on build config
   needs to be a consistent 0x10 bytes */
template <typename T = uint32_t>
struct _std_vector_proxy {
	std::uintptr_t _MyProxy;
	T* _Myfirst;
	T* _Mylast;
	T* _Myend;

	size_t size() const {
		if (!_Myfirst || !_Mylast) return 0;
		return (size_t)(_Mylast - _Myfirst);
	}
	bool at(size_t index) const {
		if (index >= size()) return false;
		return false; //
	}
};
static_assert(sizeof(_std_vector_proxy<>) == 0x10, "std::vector proxy size != 0x10");

namespace Shar {
	struct tView;
	struct pddiDevice;
	struct d3dDevice;
	struct SpatialNode;

	namespace rmt {
		struct Vector { float x, y, z; };
		struct Vector4 { float x, y, z, w; };
		struct Quaternion { float w, x, y, z; };
		struct Matrix { float m[4][4]; };
		struct Box3D { Vector low, high; };
		struct Sphere { Vector center; float radius; };
	}
	using pddiMatrix = rmt::Matrix;

	namespace sim {
		struct ManualSimState { /**/ };
		struct CollisionVolume { /**/ };
		struct CollisionObjet { /**/ };
		struct SphereVolume { /**/ };
		struct CylinderVolume { /**/ };
		struct OBBoxVolume { /**/ };
		struct WallVolume { /**/ };
	}

	template <typename T> struct SwapArray { uint32_t mSize, mUseSize; T* mpData; T mSwapT; };
	template <typename T> struct NodeSwapArray { uint32_t mSize, mUseSize; T* mpData; T mSwapT; };
	template <typename T> struct ReserveArray { uint32_t mSize, mUseSize; T* mpData; };
	template <typename T> struct FixedArray { uint32_t mSize; T* mpData; };
	template <typename T> struct ContiguoutBinNode { /**/ };

	struct tColour { uint32_t c; };
	using pddiColour = tColour;

	using radTime64 = int64_t;
	struct tUidUnaligned { uint32_t u0, u1; };
	using radKey = tUidUnaligned;
	using tUID = radKey;
	struct tName { tUID uid; };

	struct IRefCount { std::uintptr_t vfptr; };
	struct radLoadObject : IRefCount { uint32_t m_refCount; };
	struct tRefCounted : radLoadObject {};
	struct tEntity : tRefCounted { tName name; };

	struct ISpatialProxyAA { std::uintptr_t vfptr; };
	struct Vector3f : rmt::Vector {};
	struct Bounds3f { Vector3f mMin, mMax; };
	struct BoxPts : ISpatialProxyAA { Bounds3f mBounds; };
	struct SphereSP : BoxPts { rmt::Vector mCenter; float mRadius; };
	struct AAPlane3f { int8_t mAxis; float mPosn; };

	struct EventListener { std::uintptr_t vfptr; };

	struct tDrawable : tEntity { struct ShaderCallback { std::uintptr_t vfptr; }; };
	struct IEntityDSG : tDrawable, tDrawable::ShaderCallback {
		float			mRank;
		bool			mTranslucent;
		int8_t pad[3];
		tName			mShaderName;
		SpatialNode*	mpSpatialNode;
	};

	struct RenderEnums {
		enum LayerEnum : uint32_t {
			GUI = 0x00000000,
			PresentationSlot,
			LevelSlot,
			MissionSlot1,
			MissionSlot2,
			numLayers,
			LayerOnlyMask = 0x000000FF
		};
	};
	struct CollisionEntityDSG : IEntityDSG {
		int16_t						mPersistentObjectID;
		int8_t pad[2];
		uint32_t					lastUpdate;
		struct CollisionAttributes*	mpCollisionAttributes;
		bool						mWasParticleEffectTriggered;
		int8_t pad2[3];
		RenderEnums::LayerEnum		mRenderLayer;
	};
	struct StaticPhysDSG : CollisionEntityDSG {
		rmt::Box3D				mBBox;
		rmt::Sphere				mSphere;
		rmt::Vector				mPosn;
		sim::ManualSimState*	mpSimStateObj;
		tDrawable*				mpShadow;
		rmt::Matrix*			mpShadowMatrix;
	};
	struct Smoother { float rollingAverage; float factor; };
	struct DynaPhysDSG : StaticPhysDSG {
		bool		mIsHit;
		int8_t pad[3];
		Smoother	mPastLinear;
		Smoother	mPastAngular;
		int32_t		mGroundPlaneIndex;
		int32_t		mGroundPlaneRefs;
	};
	struct FenceEntityDSG : CollisionEntityDSG {
		rmt::Vector	mStartPoint;
		rmt::Vector	mEndPoint;
		rmt::Vector	mNormal;
	};
	
	const int32_t MAX_PLAYERS = 4;
	struct WorldScene { /**/ };
	struct SpatialTreeIter { /**/ };
	struct RenderLayer {
		std::uintptr_t						vfptr;
		enum eExportedState				mExportedState;
		enum eExportedState				mPreviousstate;
		tView*							mpView[MAX_PLAYERS];
		float							mAlpha;
		SwapArray<struct tDrawable*>	mpGuts;
		bool							mIsBeginView : 1;
		int8_t pad[3];
		uint32_t						mNumViews;
	};
	struct WorldSphereDSG { /**/ };
	struct DynaLoadListDSG {
		tName										mGiveItAFuckinName;
		SwapArray<WorldSphereDSG*>					mWorldSphereElems;
		SwapArray<struct StaticEntityDSG*>			mSEntityElems;
		SwapArray<StaticPhysDSG*>					mSPhysElems;
		SwapArray<DynaPhysDSG*>						mDPhysElems;
		SwapArray<struct IntersectDSG*>				mIntersectElems;
		SwapArray<FenceEntityDSG*>					mFenceElems;
		SwapArray<struct AnimCollisionEntityDSG*>	mAnimCollElems;
		SwapArray<struct AnimEntityDSG*>			mAnimElems;
		SwapArray<struct TriggerVolume*>			mTrigVolElems;
		SwapArray<struct RoadSegment*>				mRoadSegmentElems;
		SwapArray<struct PathSegment*>				mPathSegmentElems;
	};
	struct WorldRenderLayer : RenderLayer {
		WorldScene*					mpWorldScene;
		SwapArray<WorldSphereDSG*>	mWorldSpheres;
		SwapArray<DynaLoadListDSG>	mStaticLoadLists;
		SwapArray<DynaLoadListDSG*>	mLoadLists;
		int32_t						mnLoadListRefs;
		int32_t						mCurLoadIndex;
		enum DynaLoadState			mDynaLoadState;
		tUID						mCurLoadUID;
		bool						mQdDump;
		int8_t pad[3];
		uint32_t					mQdDeletionStart;
		SwapArray<tRefCounted*>*	mpQdDeletionList;
		bool						mMirror;
		int8_t pad2[3];
		rmt::Matrix					mMirrorMatrix;
	};
	struct ChunkListenerCallback { std::uintptr_t vfptr; };
	struct LoadingManager {
		struct ProcessRequestCallback { std::uintptr_t vfptr; };
	};

	struct RenderManager : ChunkListenerCallback, LoadingManager::ProcessRequestCallback, EventListener {
		SwapArray<tRefCounted*> mEntityDeletionList;
		RenderLayer* mpRenderLayers[RenderEnums::numLayers];
		/**/
	};

	const int32_t PDDI_MAX_MATRIX_STACKS = 5;
	const int32_t PDDI_MAX_STATS = 40;
	const int32_t MAX_TEXTUREFORMATS = 32;
	const int32_t PDDI_MAX_LIGHTS = 8; 
	const int32_t P3D_MAX_CONTEXTS = 4;

	struct pddiObject { std::uintptr_t vfptr; int32_t refCount; int32_t lastError; };

	template <typename T>
	struct pddiStack { /**/ };
	struct pddiViewState { /**/ };
	struct pddiRenderState { /**/ };
	struct pddiLightingState { /**/ };
	struct pddiFogState { /**/ };
	struct pddiStencilState { /**/ };
	struct pddiDisplay {
		bool		m_ForceVSync;
		bool		m_only60;
		int8_t pad[2];
	};
	enum pddiDisplayMode : uint32_t {};
	struct pddiDisplayInit {
		int32_t			xsize;
		int32_t			ysize;
		int32_t			bpp;
		uint32_t		bufferMask;
		int32_t			nColourBuffer;
		int32_t			nSamples;
		pddiDisplayMode	displayMode;
		bool			lockToVsync;
		bool			asyncSwap;
		bool			enableSnapshot;
		uint8_t pad[1];
	};
	struct pddiDisplayInfo { /**/ };
	struct d3dDisplay {
		LPDIRECT3D8				d3d;
		LPDIRECT3DDEVICE8		d3dDevice;
		D3DPRESENT_PARAMETERS	d3dpp;
		D3DCAPS8				d3dCaps;
		D3DGAMMARAMP			initialGammaRamp;
		D3DFORMAT				colourBufferFormat;
		D3DFORMAT				depthBufferFormat;
		D3DDISPLAYMODE			origMode;
		LPDIRECT3DTEXTURE8		snapshot;
		bool					hwTrans;
		bool					hwVertexShader;
		pddiDisplayInit			displayInit;
		pddiDisplayInfo*		displayInfo;
		HWND					hWnd;
		bool					reset;
		bool					isActive;
		bool					ignoreResize;
		struct d3dSurface*		backBuffer;
		LPDIRECT3DSURFACE8		frontTmpBuffer;
		int32_t					adapterID;
		int32_t					nColourBuffer;
		uint32_t				bufferMask;
		pddiDisplayMode			displayMode;
		int32_t					displayHeight;
		int32_t					displayWidth;
		int32_t					displayDepth;
		bool					forceInit;
		bool					ignoreDisplayChange;
		uint32_t				windowStyle;
		float					gammaR;
		float					gammaG;
		float					gammaB;
		bool					lockToVsync;
		bool					asyncSwap;
	};
	struct pddiRenderContext : pddiObject {};
	struct pddiContextState {
		uint32_t						currentFrame;
		uint32_t						inFrame;
		uint32_t						contextFlags;
		struct pddiMatrixStack*			matrixStack[PDDI_MAX_MATRIX_STACKS];
		pddiStack<pddiViewState>*		viewStateStack;
		pddiStack<pddiRenderState>*		renderStateStack;
		pddiStack<pddiLightingState>*	lightingStateStack;
		pddiStack<pddiFogState>*		fogStateStack;
		pddiStack<pddiStencilState>*	stencilStateStack;
		pddiViewState*					viewState;
		pddiRenderState*				renderState;
		pddiLightingState*				lightingState;
		pddiFogState*					fogState;
		pddiStencilState*				stencilState;
	};
	struct pddiBaseContext : pddiRenderContext {
		pddiDisplay*		display;
		pddiDevice*			device;
		pddiContextState	state;
		float				stats[PDDI_MAX_STATS];
		struct pddiFont*	font;
		bool				displayStats;
		bool				supressPrimTally;
	};
	struct d3dContext : pddiBaseContext {
		bool									polyClear;
		int8_t pad[3];
		struct d3dState*						d3dState;
		d3dDisplay*								display;
		d3dDevice*								device;
		struct d3dDevCaps*						devCaps;
		struct d3dShader*						defaultShader;
		LPDIRECT3DDEVICE8						d3d;
		LPDIRECT3DSURFACE8						zBuffer;
		D3DVIEWPORT8							viewPort;
		D3DFORMAT								validTextures[MAX_TEXTUREFORMATS];
		struct d3dPrimStream*					stream;
		struct d3dVertexProgramManager*			vertprogs;
		pddiMatrix								lightViewMatrix[PDDI_MAX_LIGHTS];
		bool									lightStateChanged[PDDI_MAX_LIGHTS];
		bool									anyLightStateChanged;
		int8_t pad2[3];
		int64_t									beginTime;
		float									ticksToMs;
		struct d3dExtHardwareSkinning*			extHardwareSkinning;
		struct d3dExtGammaControl*				extGammaControl;
		struct d3dExtAntialiasControl*			extAntialiasControl;
		struct d3dExtGlobalProjectedTexture*	extGlobalProjectedTexture;
		struct d3dExtReadPixels*				extReadPixels;
		struct d3dExtFramebufferEffects*		extFramebufferEffects;
	};
	struct pddiDevice : pddiObject {};
	struct d3dDevice : pddiDevice {
		pddiRenderContext*	context;
		int32_t				nDisplay;
		pddiDisplayInfo*	displayInfo;
		LPDIRECT3D8			direct3D;
	};

	struct tContext {
		struct tInventory*				Inventory;
		struct tLoadManager*			loadManager;
		uint32_t						FrameCount;
		struct tMatrixStack*			stack;
		tView*							View;
		rmt::Matrix						CVM;
		rmt::Matrix						CVMI;
		rmt::Matrix						CWM;
		tColour							clearColour;
		float							clearDepth;
		uint32_t						clearStencil;
		uint32_t						clearMask;
		int32_t							nLights;
		pddiDevice*						RenderDevice;
		pddiRenderContext*				RenderContext;
		pddiDisplay*					RenderDisplay;
		struct pddiExtHardwareSkinning*	skinning;
		struct pddiExtVertexProgram*	vertexProgram;
		bool							inFrame;
		int8_t pad[3];
	};
	struct tPlatformContext {
		tContext*	context;
		void*		windowHandle;
		void*		pddiLib;
	};
	struct tPlatform {
		void*				instance;
		tContext*			currentContext;
		int32_t				nContexts;
		tPlatformContext	contexts[P3D_MAX_CONTEXTS];
	};

	struct GameConfigHandler {};
	struct IRadDriveErrorCallback { std::uintptr_t vfptr; };
	struct Platform : IRadDriveErrorCallback {
		enum ErrorState		mErrorState;
		bool				mPauseForError;
		int8_t pad[3];
		struct IRadDrive*	mpIRadDrive;
	};
	struct Win32Platform : Platform, GameConfigHandler {
		tPlatform*					mpPlatform;
		tContext*					mpContext;
		enum Resolution				mResolution;
		int32_t						mbpp;
		bool						mFullscreen;
		int8_t pad[3];
		int32_t						mScreenWidth;
		int32_t						mScreenHeight;
	};
	struct Context : EventListener { enum StateEnum m_state; };
	enum ContextEnum : uint32_t {
		CONTEXT_ENTRY,
		CONTEXT_BOOTUP,
		CONTEXT_FRONTEND,
		CONTEXT_LOADING_DEMO,
		CONTEXT_DEMO,
		CONTEXT_SUPERSPRINT_FE,
		CONTEXT_LOADING_SUPERSPRINT,
		CONTEXT_SUPERSPRINT,
		CONTEXT_LOADING_GAMEPLAY,
		CONTEXT_GAMEPLAY,
		CONTEXT_PAUSE,
		CONTEXT_EXIT,
		NUM_CONTEXTS
	};
	struct GameFlow {
			/* std::vector size is inconsistent */
		//using ContextEnumSequence		= std::vector<ContextEnum/*, s2alloc<ContextEnum>*/>;
		using ContextEnumSequence	= _std_vector_proxy<ContextEnum>;
		using ContextStack				= std::stack<ContextEnum, ContextEnumSequence>;

		std::uintptr_t			vfptr;
		struct IRadTimer*	mpITimer;
		ContextEnum			mCurrentContext;
		ContextEnum			mNextContext;
		//ContextStack		mContextStack; /* size == 0x10 */
		/**/int8_t _[0x10];
		Context*			mpContexts[NUM_CONTEXTS];
		bool				mQuickStartLoading : 1;
		int8_t pad[3];
	};
	struct Game {
		std::uintptr_t				vfptr;
		Win32Platform*			mpPlatform;
		struct IRadTimerList*	mpTimerList;
		GameFlow*				mpGameFlow;
		struct RenderFlow*		mpRenderFlow;
		uint32_t				mFrameCount;
		bool					mExitNow;
		int8_t pad[3];
		uint32_t				mDemoCount;
		uint32_t				mTimeMS;
	};
	struct tView { /**/ };
	
	const int32_t MAX_ACTION_BUTTON_HANDLERS = 5;
	const int32_t CollisionData_MAX_COLLISIONS = 8;
	const int32_t Character_MAX_PROPS = 1;
	//namespace CollisionData { const int32_t MAX_COLLISIONS = 8; }
	struct CollisionData {
		rmt::Vector				mCollisionPosition;
		rmt::Vector				mCollisionNormal;
		float					mCollisionDistance;
		sim::CollisionVolume*	mpCollisionVolume;
	};
	namespace poser {
		struct Joint { /**/ };
		struct PoseEngine { /**/ };
		struct PoseDriver : tEntity {
			bool		m_IsEnabled;
			int8_t pad[3];
		};
		struct Transform {
			rmt::Matrix		m_Matrix;
			rmt::Quaternion	m_Quaternion;
			enum Dirty		m_Dirty;
		};
	}
	namespace choreo {
		template <typename T>
		struct BlendPriority { /**/ };
		struct RootBlender : poser::PoseDriver {
			int32_t							m_RootJointIndex;
			poser::Transform				m_RootTransform;
			poser::Transform				m_PrevRootTransform;
			poser::Transform				m_ParentTransform;
			rmt::Vector						m_GroundPoint;
			bool							m_IsGroundPointSet;
			int8_t pad[3];
			float							m_PrevGroundFixupWeight;
			float							m_GroundFixupWeight;
			float							m_DeltaTime;
			rmt::Vector						m_Velocity;
			float							m_TurningVelocity;
			float							m_MaxRootDrivers;
			int32_t							m_RootDriverCount;
			struct RootDriverStruct*		m_RootDrivers;
			int32_t							m_MaxBlenderPriorities;
			int32_t							m_TranslatePriorityCount;
			BlendPriority<rmt::Vector>*		m_TranslatePriorities;
			int32_t							m_RotatePriorityCount;
			BlendPriority<rmt::Quaternion>*	m_RotatePriorities;
			int32_t							m_GroundFixupPriorityCount;
			BlendPriority<float>*			m_GroundFixupPriorities;
		};
		struct Engine : tRefCounted {
			struct Rig*					m_Rig;
			poser::PoseEngine*			m_PoseEngine;
			RootBlender*				m_RootBlender;
			struct RootFixupDriver*		m_RootFixupDriver;
			struct JointBlender*		m_JointBlender;
			struct CompletePartition*	m_FootMaskPartition;
			struct ReplayBlender*		m_ReplayBlender;
			struct FootBlender**		m_FootBlenders;
			struct FootPlanter*			m_FootPlanter;
			float						m_DeltaTime;
		};
		struct Puppet : tEntity {
			Engine*					m_Engine;
			struct Bank*			m_Bank;
			int32_t					m_MaxDriverCount;
			int32_t					m_DriverCount;
			struct DriverStruct*	m_Drivers;
		};
	};
	namespace ActionButton {
		struct ButtonHandler { /**/ };
		struct AttachProp { /**/ };
	}
	namespace CharacterAi {
		struct StateManager { /**/ };
	}
	
	struct Character : DynaPhysDSG {
		struct Prop {
			struct InstDynaPhysDSG*	mpProp;
			struct tPose*			mpPose;
		};
		bool						mbCollidedWithVehicle;
		bool						mbInAnyonesFrustrum;
		bool						mbSurfing;
		bool						mbAllowUnload;
		bool						mbIsPlayingIdleAnim;
		int8_t pad[3];
		int32_t						mPCCamFacing;
		rmt::Matrix					mPrevSimTransform;
		bool						mIsNPC;
		int8_t pad2[3];
		sim::ManualSimState*		mGroundPlaneSimState;
		sim::WallVolume*			mGroundPlaneWallVolume;
		int32_t						mCollisionAreaIndex;
		int8_t pad3[4];
		radTime64					mLastInteriorLoadCheck;
		struct CharacterController*	mpController;
		struct CharacterRenderable*	mpCharacterRenderable;
		choreo::Puppet*				mpPuppet;
		float						mfFacingDir;
		float						mfDesiredDir;
		float						mfSpeed;
		rmt::Vector					mVelocity;
		float						mfDesiredSpeed;
		bool						mbInCar;
		int8_t pad4[3];
		_std_vector_proxy<uint32_t>	mbWasFootPlanted;
		struct CharacterTarget*			mpCharacterTarget;
		struct ActionController*		mpActionController;
		ActionButton::ButtonHandler*	mpActionButtonHandlers[MAX_ACTION_BUTTON_HANDLERS];
		ActionButton::ButtonHandler*	mpCurrentActionButtonHandler;
		struct Vehicle*					mpTargetVehicle;
		float							mGroundY;
		rmt::Vector						mGroundNormal;
		enum eTerrainType				mTerrainType;
		bool							mInteriorTerrain;
		int8_t pad5[3];
		rmt::Vector						mRealGroundPos;
		rmt::Vector						mRealGroundNormal;
		CharacterAi::StateManager*		mpStateManager;
		float							mfRadius;
		bool							mbCollided;
		int8_t pad6[3];
		int32_t							mCurrentCollision;
		CollisionData					mCollisionData[8];
		bool							mbIsStanding;
		int8_t pad7[3];
		struct WalkerLocomotionAction*	mpWalkerLocomotion;
		struct JumpAction*				mpJumpLocomotion;
		sim::CollisionVolume*			mpStandingCollisionVolume;
		poser::Joint*					mpStandingJoint;
		rmt::Matrix						mParentTransform;
		rmt::Matrix						mInvParentTransform;
		float							mfGroundVerticalVelocity;
		float							mfGroundVerticalPosition;
		bool							mbTurbo;
		bool							mbIsJump;
		bool							mbSolveCollisions;
		int8_t pad8[1];
		Prop							mPropList[Character_MAX_PROPS];
		ActionButton::AttachProp*		mpPropHandler;
		int32_t							mPropJoint;
		bool							mVisible;
		int8_t pad9[3];
		WorldScene*						mpWorldScene;
		bool							m_IsSimpleShadow;
		int8_t pad10[3];
		float							mYAdjust;
		bool							mbBusy;
		bool							mbSimpleLoco;
		bool							mbNeedChoreoUpdate;
		int8_t pad11[1];
		tColour							mShadowColour;
		float							m_TimeLeftToShock;
		bool							m_IsBeingShocked;
		bool							mDoKickwave;
		int8_t pad12[2];
		tDrawable*						mKickwave;
		struct tFrameController*		mKickwaveController;
		bool							mAmbient;
		int8_t pad13[3];
		tUID							mAmbientLocator;
		struct AmbientDialogueTrigger*	mAmbientTrigger;
		rmt::Vector						mLastFramePos;
		bool							mbDoGroundIntersect;
		int8_t pad14[3];
		uint32_t						mIntersectFrame;
		bool							mAllowRockin;
		bool							mHasBeenHit;
		bool							mbSnapToGround;
		int8_t pad15[1];
		float							mSecondsSinceActionControllerUpdate;
		bool							mTooFarToUpdate;
		int8_t pad16[3];
		float							mSecondsSinceOnPostSimUpdate;
		enum Role						mRole;
		float							mScale;
		bool							mCollidedThisFrame;
		bool							mIsInSubstep;
		int8_t pad17[2];
		rmt::Vector						mLean;
		bool							mIsLisa;
		bool							mIsMarge;
		int8_t pad18[2];
		rmt::Vector						mLastGoodPosOverStatic;
		rmt::Vector						lameAssPosition;
		bool							mManaged;
		int8_t pad19[3];
	};
	struct Vehicle { /**/ };

	struct RoadManager {
		struct PathElement {
			enum ElementType	type;
			void* elem;
		};
	};

	struct Avatar {
		bool						mHasBeedUpdatedThisFrame;
		int8_t pad[3];
		RoadManager::PathElement	mLastPathElement;
		struct RoadSegment*			mLastRoadSegment;
		float						mLastRoadSegmentT;
		float						mLastRoadT;
		float						mDistToCurrentCollectible;
		int32_t						mCurrentCollectible;
		int32_t						mNumLapsCompleted;
		int32_t						mControllerId;
		int32_t						mPlayerId;
		Character*					mpCharacter;
		Vehicle*					mpVehicle;
		struct AvatarInputManager*	mpAvatarInputManager;
	};
	struct AvatarManager : EventListener {
		Avatar* mAvatarArray[4];
		int32_t mNumAvatars;
	};
}

#pragma pack(pop)