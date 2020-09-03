# Utopian Engine

This project is developed to learn how renderers and game engine works as well as acting as a base for research/experimentation. The intention is to make my own games using it and also once more mature make it convenient for others to use as well.

It is written C++ and uses Vulkan as the rendering API.

## Media

![Image](data/printscreens/readme-print.png)
![Image](data/printscreens/terrain-texturing-2.png)
![Image](data/printscreens/water-ssr-8.png)

## Features
+ Vulkan backend
+ Deferred shading
+ Casacade shadow mapping
+ Screen Space Reflections
+ Normal mapping
+ God rays
+ SSAO
+ Instancing
+ Runtime shader compilation
+ Shader reflection
+ Basic Lua scripting
+ Terrain generation
+ Skydome with sun
+ ImGui user interface
+ ECS layer
+ Terrain tesselation
+ Displacement mapping
+ Water

## Planned features
+ Atmospheric scattering
+ HDR
+ Particle systems
+ LOD
+ Clouds

## Building

Currently the engine only runs on Windows.

Run the `generate_workspace.bat` script to generate a Visual Studio 2019 solution.
If you want to generate a solution for another VS version run `tools/premake5.exe` manually.

Pre compiled libraries for MSVC x64 Debug are included in the `libs/` folder.