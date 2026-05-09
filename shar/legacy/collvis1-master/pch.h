// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <memory>
#include <cstdio>
#include <cstdint>

#include <d3dx8.h>
#include <MinHook.h>
#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "libMinHook.x86.lib")

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx8.h>

#endif //PCH_H
