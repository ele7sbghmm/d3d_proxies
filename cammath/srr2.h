#pragma once

#include <p3d/pointcamera.hpp>
#include <camera/supercammanager.h>
#include <render/Culling/BoxPts.h>
#include <render/RenderManager/RenderManager.h>
//#include <render/RenderManager/WorldRenderLayer.h>
#include <worldsim/coins/coinmanager.h>
#include <worldsim/avatarmanager.h>

inline auto ppCoinManager = reinterpret_cast<CoinManager**>(0x6c8450);
inline auto ppSuperCamCentral = reinterpret_cast<SuperCamCentral**>(0x6c906c);
inline auto ppAvatarManager = reinterpret_cast<AvatarManager**>(0x6c84e4);
inline auto ppRenderManager = reinterpret_cast<RenderManager**>(0x6c87b4);

//WorldRenderLayer* GetWorldRenderLayer() {
//	RenderManager* rm = *ppRenderManager;
//	if (!rm) return nullptr;
//	if (!rm->mpRenderLayers) return nullptr;
//	if (!rm->mpRenderLayers[RenderEnums::LevelSlot]) return nullptr;
//	return (WorldRenderLayer*)rm->mpRenderLayers[RenderEnums::LevelSlot];
//}
//WorldScene* GetWorldScene() {
//	WorldRenderLayer* wrl = GetWorldRenderLayer();
//	if (!wrl) return nullptr;
//	if (!wrl->mpWorldScene) return nullptr;
//	return wrl->mpWorldScene;
//}
