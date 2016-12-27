# Engine requirements

This document describes in general terms what the goal of the Engine is and what it should support once _completed_.
The more detailed requirements of each part of the engine is located in the software requirements file.

## Renderer
* The renderer uses Vulkan API
	- 3D and 2D rendering
	- Does not have any OpenGL or DirectX implementation 
	- Works on Linux, Windows and Android 
* It renders primitives using a lighting shader
	- Extendable with more shaders and pipelines
* Multithreaded draw calls
	- Primitives are stored in a logical way for multithreaded draw calls
* Support for many different rendering techniques
	- Terrain rendering, instancing, particle systems, blur, bloom, shadow mapping, skeletal animation
	- Text rendering
	- Post process effects
* The renderer can be used without the `RenderSystem`

## Game world
* The game world contains all the actors

## Entity-Component-System
* The higher level design paradigm is the Entity-Component-System (ESC) design pattern
* ECS is not used in the lower level graphics objects
* Logic is separated from the Components into Systems
* Examples of Systems
	- `RenderSystem, InputSystem, CollisionSystem, PhysicsSystem, AISystem`
	
## Render system
* Handles the rendering of Entities that have a mesh component
* Internally contains the Vulkan renderer

## Physics systems
* Simulates physics on the Entities that have physics component

## Input system
* Takes user input and updates the components that have a input component

## Entities & Components
* Every object in the game world is an Entitie
* Entities are made up of different Components
	- These components can be changed during runtime, to modify the Entities behavior
* Full scripting support in Lua	
* Available components
	- translation, static mesh, animated mesh, point light, spot light, boundig box, physics, AI

## Lua scripting
* Entities and their behavior can be scripted in Lua
* Simple games can be created entirely in Lua
	- 2D platform game, Tetris etc.

## Generic and applicaton specific parts
* Common and generic code are located in the core engine
* Application specific rendering techniques are supported
	- Special shaders, pipelines etc. can be written in the appliation code
	- Custom terrain representations is located in application code
	- These are moved to the engine core if reusable
	
## Level editor (low priority)
* Simple levels can be created inside the level editor
* Support for terrain editing tools
* Actors can be placed in the world
