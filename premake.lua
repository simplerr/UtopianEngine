workspace "UtopianEngine"
   configurations { "Debug", "Release" }
   language "C++"
   cppdialect "C++17"
   platforms "x64"
   startproject "Editor"
   characterset "ASCII"
   buildoptions "/Zc:__cplusplus"

   -- Defines
   defines
   {
      "BT_USE_DOUBLE_PRECISION",
      "GLM_FORCE_CTOR_INIT",
      "WIN32",
      "_WINDOWS",
      "VK_USE_PLATFORM_WIN32_KHR",
      "_USE_MATH_DEFINES",
      "NOMINMAX",
      "_CRT_SECURE_NO_WARNINGS",
      "GLM_FORCE_RADIANS",
      "GLM_FORCE_RIGHT_HANDED",
      "GLM_FORCE_DEPTH_ZERO_TO_ONE"
      --"LUA_FLOAT_TYPE=1", -- Results in incorrect reading from scene.lua file
   }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG", "_DEBUG" }
      flags { "MultiProcessorCompile", }
      symbols "On"
      linkoptions { "-IGNORE:4099" } -- Ignore "Missing .pdb debug file" warnings for libs used
      linkoptions { "-IGNORE:4006" } -- Ignore "Already defined in" warnings (some Assimp function are defined twice)
      
      disablewarnings { "26812" } -- Ignore "Prefer enum class over enum"
      disablewarnings { "4715" } -- Ignore "Not all control paths return a value"
      disablewarnings { "26495" } -- Ignore "Always initialize a member variable"
    
   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      flags { "MultiProcessorCompile" } -- , "LinkTimeOptimization" }
      symbols "Off"
      optimize "Full"
      linkoptions { "-IGNORE:4006" } -- Ignore "Already defined in" warnings (some Assimp function are defined twice)

include "source/utopian/premake.lua"
include "source/editor/premake.lua"
include "source/demos/marching_cubes/premake.lua"
include "source/demos/pbr/premake.lua"
include "source/demos/raytracing/premake.lua"
