#pragma once

#include "shar.h"
#include "vertex.h"

extern IDirect3DDevice8* g_device;
extern D3DXCOLOR g_Color_FenceOpaqueActive;

namespace mod {
	class Game : public Shar::Game { public: };

	class Win32Platform : public Shar::Win32Platform { public: };
	class d3dDevice : public Shar::d3dDevice { public: };
	class d3dDisplay : public Shar::d3dDisplay { public: };
	class d3dContext : public Shar::d3dContext { public: };
	class tContext : public Shar::tContext { public: };
	class tPlatform : public Shar::tPlatform { public: };
	class SpatialTreeIter : public Shar::SpatialTreeIter { public: };
	class DynaLoadListDSG : public Shar::DynaLoadListDSG { public: };
	class WorldScene : public Shar::WorldScene { public: };

	class WorldRenderLayer : public Shar::WorldRenderLayer {
	public:
		auto GetWorldScene() -> WorldScene&;
	};

	class RenderManager : public Shar::RenderManager {
	public:
		static auto Get() -> RenderManager*;
		auto GetWorldRenderLayer() -> WorldRenderLayer*;
	};

	class Character : public Shar::Character {
	public:
		static auto GetInstance() -> Character*;
		auto GetPosition() -> D3DXVECTOR3* {
			return (D3DXVECTOR3*)mpPuppet->m_Engine->m_RootBlender->m_RootTransform.m_Matrix.m[3];
		}
		auto GetFacing() -> D3DXVECTOR3 {
			D3DXVECTOR3 dest{};
			D3DXVECTOR3 src = { 0.f, 0.f, -1.f };
			D3DXMATRIX m = *reinterpret_cast<D3DXMATRIX*>(&mpPuppet->m_Engine->m_RootBlender->m_RootTransform.m_Matrix);
			D3DXVec3TransformNormal(&dest, &src, &m);

			return dest;
		}
	};
}

DWORD dwDecl[] = {
	D3DVSD_STREAM(0),
	D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
	D3DVSD_SKIP(1),
	D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),
	D3DVSD_REG(D3DVSD3_DIFFUSE, D3DVSDT_D3DCOLOR),
	D3DVSD_END()
};
