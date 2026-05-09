// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <windows.h>
#include <cstdio>
#include <ranges>
#include <variant>

#include <d3dx8.h>
#include <MinHook.h>
#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "libMinHook.x86.lib")

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imconfig.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>
#include <implot.h>
#include <implot_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx8.h>


#endif //PCH_H
