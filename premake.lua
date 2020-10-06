workspace "UtopianEngine"
   configurations { "Debug", "Release" }
   language "C++"
   cppdialect "C++17"
   platforms "x64"
   startproject "Editor"
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
         "external/LegitProfiler/*.cpp"
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
      libdirs { "libs/assimp" }
      libdirs { "libs/bullet3-2.88" }
      libdirs { "libs/glslang" }
      libdirs { "libs/luaplus" }
      libdirs { "libs/vulkan" }
      libdirs { "libs/ktx" }
   
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
         links { "libktx.gl" }
   
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
