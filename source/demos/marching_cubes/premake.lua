
-- =========================================
-- ========= Marching cubes demo ===========
-- =========================================
project "Marching Cubes Demo"
   kind "WindowedApp"
   targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
   objdir "%{wks.location}/bin/%{cfg.buildcfg}"
   location "%{wks.location}/"

   -- Files
   files
   {
      "**.hpp",
      "**.h",
      "**.cpp",
   }

   -- Includes
   root = "../../../"
   includedirs { root .. "external/bullet3" }
   includedirs { root .. "external/luaplus" }
   includedirs { root .. "external/luaplus/lua53-luaplus/src" }
   includedirs { root .. "external/glslang/StandAlone" }
   includedirs { root .. "external/glslang" }
   includedirs { root .. "external/glm" }
   includedirs { root .. "external/gli" }
   includedirs { root .. "external/assimp" }
   includedirs { root .. "external" }
   includedirs { root .. "source/utopian" }
   includedirs { root .. "source" }

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