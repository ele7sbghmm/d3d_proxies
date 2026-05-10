#pragma once

#include <Minhook.h>
#pragma comment(lib, "libMinhook.x86.lib")

#include "shar.h"

class hook
{
public:
    void* address{};
    void* detour{};
    void* trampoline{};

    hook(void* addr, void* det) : detour(det), address(addr) {}
    void inject() {
        MH_CreateHook(address, detour, &trampoline);
        MH_EnableHook(address);
    }
};

namespace hooks
{
    std::string stringify_fences()
    {
        RenderManager* rm = RenderManager::GetInstance();
        if (!rm) return {};

        SwapArray<DynaLoadListDSG*> sw = rm->wrl->mLoadLists;
        if (!sw.mUseSize) return {};

        SwapArray<FenceEntityDSG*> sw_fences = sw.mpData[0]->mFenceElems;
        if (!sw_fences.mUseSize) return {};

        std::string body;
        body.reserve(256 * 1024);

        body += "[";
        for (int i = 0; i < sw_fences.mUseSize; ++i)
        {
            if (i > 0)
                body += ",";
            body += sw_fences.mpData[i]->stringify();
        }
        body += "]";

        return body;
    }

    //hook render_translucent((void*)0x49cbd0, enable_ready);
}
