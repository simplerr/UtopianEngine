workspace "UtopianEngine"
   configurations { "Debug", "Release" }
   language "C++"
   cppdialect "C++17"
   platforms "x64"
   characterset "ASCII"

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
      "NOMINMAX"
   }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG" }
      flags { "MultiProcessorCompile", "LinkTimeOptimization" }
      symbols "On"
    
   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      flags { "MultiProcessorCompile" }
      symbols "Off"
      optimize "Full"

project "UtopianEngine"
   kind "WindowedApp"
   targetdir "bin/%{cfg.buildcfg}"

   -- Files
   files
   {
      -- Utopian
      "source/**.hpp",
      "source/**.h",
      "source/**.cpp",
      "external/vk_mem_alloc.h",
      "external/stb_image.h",
      "external/im3d/*.h",
      "external/im3d/*.cpp",
      "external/imgui/*.h",
      "external/imgui/*.cpp",
      "external/LegitProfiler/*.h",
      "external/LegitProfiler/*.cpp"
   }

   removefiles { "**/marching_cubes_legacy/**" }

   -- Includes
   includedirs { "external/bullet3-2.88" }
   includedirs { "external/luaplus" }
   includedirs { "external/luaplus/lua53-luaplus/src" }
   includedirs { "external/glslang/StandAlone" }
   includedirs { "external/glslang" }
   includedirs { "source" }
   includedirs { "external/glm" }
   includedirs { "external/gli" }
   includedirs { "external/assimp" }
   includedirs { "external" }

   -- Libraries
   libdirs { "libs/assimp" }
   libdirs { "libs/bullet3-2.88" }
   libdirs { "libs/glslang" }
   libdirs { "libs/luaplus" }
   libdirs { "libs/vulkan" }

   -- "Debug"
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      debugformat "c7"
      links { "BulletCollision_x64_debug" }
      links { "BulletDynamics_x64_debug" }
      links { "BulletSoftBody_x64_debug" }
      links { "LinearMath_x64_debug" }
      links { "lua53-luaplus-static.debug" }
      links { "OSDependentd" }
      links { "glslangd" }
      links { "HLSLd" }
      links { "OGLCompilerd" }
      links { "SPIRVd" }
      links { "SPVRemapperd" }
      links { "vulkan-1" }
      links { "assimp" }

   -- "Release"
   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"