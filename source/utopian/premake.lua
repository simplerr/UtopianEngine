-- =========================================
-- ================ Engine =================
-- =========================================
project "Engine"
      kind "StaticLib"
      targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
      objdir "%{wks.location}/bin/%{cfg.buildcfg}"
      location "%{wks.location}/"

      root = "../../"
   
      -- Files
      files
      {
         -- Utopian
         root .. "source/utopian/**.hpp",
         root .. "source/utopian/**.h",
         root .. "source/utopian/**.cpp",
         root .. "external/vk_mem_alloc.h",
         root .. "external/stb_image.h",
         root .. "external/stb_image_write.h",
         root .. "external/libktx/ktx.h",
         root .. "external/libktx/ktxvulkan.h",
         root .. "external/im3d/*.h",
         root .. "external/im3d/*.cpp",
         root .. "external/imgui/*.h",
         root .. "external/imgui/*.cpp",
         root .. "external/LegitProfiler/*.h",
         root .. "external/LegitProfiler/*.cpp",
         root .. "external/tinygltf/tiny_gltf.h",
         root .. "external/tinygltf/json.hpp",
         root .. "external/nativefiledialog/nfd.h",
         root .. "external/IconFontCppHeaders/*.h"
      }
   
      removefiles { "**/marching_cubes_legacy/**" }
   
      -- Includes
      includedirs { root .. "external/bullet3" }
      includedirs { root .. "external/luaplus" }
      includedirs { root .. "external/luaplus/lua53-luaplus/src" }
      includedirs { root .. "external/glslang/StandAlone" }
      includedirs { root .. "external/glslang" }
      includedirs { root .. "external/glm" }
      includedirs { root .. "external/gli" }
      includedirs { root .. "external/assimp" }
      includedirs { root .. "external/tinygltf" }
      includedirs { root .. "external/libktx" }
      includedirs { root .. "external" }
      includedirs { root .. "source/utopian" }
      includedirs { root .. "source" }
   
      -- Libraries
      libdirs { root .. "libs/assimp" }
      libdirs { root .. "libs/bullet3" }
      libdirs { root .. "libs/glslang" }
      libdirs { root .. "libs/luaplus" }
      libdirs { root .. "libs/vulkan" }
      libdirs { root .. "libs/libktx" }
      libdirs { root .. "libs/OpenMesh" }
      libdirs { root .. "libs/nativefiledialog" }
   
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
         links { "libktx.gl.debug" }
         links { "OpenMeshCored" }
         links { "nfd_d" }
   
      -- "Release"
      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "Full"
         links { "BulletCollision_x64_release" }
         links { "BulletDynamics_x64_release" }
         links { "BulletSoftBody_x64_release" }
         links { "LinearMath_x64_release" }
         links { "lua53-luaplus-static" }
         links { "GenericCodeGen" }
         links { "glslang" }
         links { "HLSL" }
         links { "MachineIndependent" }
         links { "OGLCompiler" }
         links { "OSDependent" }
         links { "SPIRV" }
         links { "SPVRemapper" }
         links { "vulkan-1" }
         links { "assimp-vc142-mt" }
         links { "libktx.gl.release" }
         links { "OpenMeshCore" }
         links { "nfd" }
