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
      "_DEBUG",
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
      defines { "DEBUG" }
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
      flags { "MultiProcessorCompile", "LinkTimeOptimization" }
      symbols "Off"
      optimize "Full"

-- =========================================
-- ================ Engine =================
-- =========================================
project "Engine"
      kind "StaticLib"
      targetdir "bin/%{cfg.buildcfg}"
   
      -- Files
      files
      {
         -- Utopian
         "source/utopian/**.hpp",
         "source/utopian/**.h",
         "source/utopian/**.cpp",
         "external/vk_mem_alloc.h",
         "external/stb_image.h",
         "external/stb_image_write.h",
         "external/ktx.h",
         "external/ktxvulkan.h",
         "external/im3d/*.h",
         "external/im3d/*.cpp",
         "external/imgui/*.h",
         "external/imgui/*.cpp",
         "external/LegitProfiler/*.h",
         "external/LegitProfiler/*.cpp",
         "external/tinygltf/tiny_gltf.h",
         "external/tinygltf/json.hpp",
         "external/nativefiledialog/nfd.h"
      }
   
      removefiles { "**/marching_cubes_legacy/**" }
   
      -- Includes
      includedirs { "external/bullet3-2.88" }
      includedirs { "external/luaplus" }
      includedirs { "external/luaplus/lua53-luaplus/src" }
      includedirs { "external/glslang/StandAlone" }
      includedirs { "external/glslang" }
      includedirs { "external/glm" }
      includedirs { "external/gli" }
      includedirs { "external/assimp" }
      includedirs { "external/tinygltf" }
      includedirs { "external" }
      includedirs { "source/utopian" }
      includedirs { "source" }
   
      -- Libraries
      libdirs { "libs/assimp" }
      libdirs { "libs/bullet3-2.88" }
      libdirs { "libs/glslang" }
      libdirs { "libs/luaplus" }
      libdirs { "libs/vulkan" }
      libdirs { "libs/ktx" }
      libdirs { "libs/OpenMesh" }
      libdirs { "libs/nativefiledialog" }
   
      -- "Debug"
      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"
         debugformat "c7"
         optimize "Speed"
         links { "BulletCollision_x64_debug" }
         links { "BulletDynamics_x64_debug" }
         links { "BulletSoftBody_x64_debug" }
         links { "LinearMath_x64_debug" }
         links { "lua53-luaplus-static.debug" }
         links { "GenericCodeGend" }
         links { "glslangd" }
         links { "HLSLd" }
         links { "MachineIndependentd" }
         links { "OGLCompilerd" }
         links { "OSDependentd" }
         links { "SPIRVd" }
         links { "SPVRemapperd" }
         links { "vulkan-1" }
         links { "assimp-vc142-mtd" }
         links { "libktx.gl" }
         links { "OpenMeshCored" }
         links { "OpenMeshToolsd" }
         links { "nfd_d" }
   
      -- "Release"
      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"

-- =========================================
-- ================ Editor =================
-- =========================================
project "Editor"
   kind "WindowedApp"
   targetdir "bin/%{cfg.buildcfg}"

   -- Files
   files
   {
      -- Editor
      "source/editor/**.hpp",
      "source/editor/**.h",
      "source/editor/**.cpp",
   }

   removefiles { "**/marching_cubes_legacy/**" }

   -- Includes
   includedirs { "external/bullet3-2.88" }
   includedirs { "external/luaplus" }
   includedirs { "external/luaplus/lua53-luaplus/src" }
   includedirs { "external/glslang/StandAlone" }
   includedirs { "external/glslang" }
   includedirs { "external/glm" }
   includedirs { "external/gli" }
   includedirs { "external/assimp" }
   includedirs { "external" }
   includedirs { "source/utopian" }
   includedirs { "source" }

   -- Libraries

   links
   {
      "Engine"
   }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      debugformat "c7"

   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- =========================================
-- ============ Raytracing demo ============
-- =========================================
project "Raytrace Demo"
   kind "WindowedApp"
   targetdir "bin/%{cfg.buildcfg}"

   -- Files
   files
   {
      -- Editor
      "source/demos/raytracing/**.hpp",
      "source/demos/raytracing/**.h",
      "source/demos/raytracing/**.cpp",
   }

   -- Includes
   includedirs { "external/bullet3-2.88" }
   includedirs { "external/luaplus" }
   includedirs { "external/luaplus/lua53-luaplus/src" }
   includedirs { "external/glslang/StandAlone" }
   includedirs { "external/glslang" }
   includedirs { "external/glm" }
   includedirs { "external/gli" }
   includedirs { "external/assimp" }
   includedirs { "external" }
   includedirs { "source/utopian" }
   includedirs { "source" }

   -- Libraries

   links
   {
      "Engine"
   }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      debugformat "c7"

   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- =========================================
-- ============ Raytracing demo ============
-- =========================================
project "Marching Cubes Demo"
   kind "WindowedApp"
   targetdir "bin/%{cfg.buildcfg}"

   -- Files
   files
   {
      -- Editor
      "source/demos/marching_cubes/**.hpp",
      "source/demos/marching_cubes/**.h",
      "source/demos/marching_cubes/**.cpp",
   }

   -- Includes
   includedirs { "external/bullet3-2.88" }
   includedirs { "external/luaplus" }
   includedirs { "external/luaplus/lua53-luaplus/src" }
   includedirs { "external/glslang/StandAlone" }
   includedirs { "external/glslang" }
   includedirs { "external/glm" }
   includedirs { "external/gli" }
   includedirs { "external/assimp" }
   includedirs { "external" }
   includedirs { "source/utopian" }
   includedirs { "source" }

   -- Libraries

   links
   {
      "Engine"
   }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      debugformat "c7"

   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- =========================================
-- ============ PBR demo ============
-- =========================================
project "PBR Demo"
   kind "WindowedApp"
   targetdir "bin/%{cfg.buildcfg}"

   -- Files
   files
   {
      -- Editor
      "source/demos/pbr/**.hpp",
      "source/demos/pbr/**.h",
      "source/demos/pbr/**.cpp",
   }

   -- Includes
   includedirs { "external/bullet3-2.88" }
   includedirs { "external/luaplus" }
   includedirs { "external/luaplus/lua53-luaplus/src" }
   includedirs { "external/glslang/StandAlone" }
   includedirs { "external/glslang" }
   includedirs { "external/glm" }
   includedirs { "external/gli" }
   includedirs { "external/assimp" }
   includedirs { "external" }
   includedirs { "source/utopian" }
   includedirs { "source" }

   -- Libraries

   links
   {
      "Engine"
   }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      debugformat "c7"

   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"