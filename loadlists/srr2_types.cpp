#include "pch.h"

#include "srr2_types.h"

static uintptr_t RENDERMANAGER_GETINSTANCE        = 0x004a8e60;
static uintptr_t RENDERMANAGER_PWORLDRENDERLAYER  = 0x004a9680;
static uintptr_t AVATARMANAGER_GETINSTANCE        = 0x004d7a30;
static uintptr_t AVATARMANAGER_GETAVATARFORPLAYER = 0x004d7f40;
static uintptr_t AVATAR_GETPOSITION               = 0x004d76f0;  // eax this   esi pos
static uintptr_t SUPERCAMMANAGER_GETINSTANCE      = 0x0042a270;  // 
static uintptr_t SUPERCAMMANAGER_GETSCC           = 0x0042a2d0;  // ecx this   eax id(0)

auto RenderManager::GetInstance() -> RenderManager* {
	uintptr_t rm{};
	__asm {
		call RENDERMANAGER_GETINSTANCE
		mov rm, eax
	}
	return (RenderManager*)rm;
}
auto RenderManager::pWorldRenderLayer() -> WorldRenderLayer* {
	int wrlIndex = 2;
	uintptr_t wrl{};
	__asm {
		mov ecx, wrlIndex
		mov eax, this
		call RENDERMANAGER_PWORLDRENDERLAYER
		mov wrl, eax
	}
	return (WorldRenderLayer*)wrl;
}

auto AvatarManager::GetInstance() -> AvatarManager* {
	uintptr_t am{};
	__asm {
		call AVATARMANAGER_GETINSTANCE
		mov am, eax
	}
	return (AvatarManager*)am;
}
auto AvatarManager::GetAvatarForPlayer(size_t id) -> Avatar* {
	uintptr_t a{};
	__asm {
		mov eax, id
		mov ecx, this
		call AVATARMANAGER_GETAVATARFORPLAYER
		mov a, eax
	}
	return (Avatar*)a;
}
auto Avatar::GetPosition(rmt::Vector* pos) -> void {
	__asm {
		mov esi, pos
		mov eax, this
		call AVATAR_GETPOSITION
	}
}
auto SuperCamManager::GetInstance() -> SuperCamManager* {
	uintptr_t scm{};
	__asm {
		call SUPERCAMMANAGER_GETINSTANCE
		mov scm, eax
	}
	return (SuperCamManager*)scm;
}
auto SuperCamManager::GetSCC(size_t id) -> SuperCamCentral* {
	uintptr_t scc{};
	__asm {
		mov eax, id
		mov ecx, this
		call SUPERCAMMANAGER_GETSCC
		mov scc, eax
	}
	return (SuperCamCentral*)scc;
}