
#ifndef PCH_H
#define PCH_H

#include "framework.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <cstdio>
#include <limits>

#include <d3d8.h>
#include <d3dx8.h>
#include <MinHook.h>

#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "libMinHook.x86.lib")

#endif //PCH_H
