workspace "p5_workspace"
    configurations { "Debug" }
    platforms { "x86" }
    location "build"

project "p5_project"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    targetname "version"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"

    files { "src/**.h", "src/**.cpp", "src/exports.def" }

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
    
    filter {}
