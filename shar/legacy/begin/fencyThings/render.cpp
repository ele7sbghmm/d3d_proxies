#include "pch.h"

#include "render.h"
#include "statics.h"

RenderManager* RenderManager::Get() { return *Statics::RENDER_MANAGER; }

WorldRenderLayer* WorldRenderLayer::Get() { return RenderManager::Get()->mpRenderLayers_WRL; }

WorldScene* WorldScene::Get() { return WorldRenderLayer::Get()->mpWorldScene; }
