#pragma once
#include <functional>
#include "vulkan/VulkanApp.h"
#include "utility/Module.h"
#include "utility/Common.h"

namespace Utopian
{
	class Im3dRenderer;
	class ImGuiRenderer;

	class Engine : public Module<Engine>
	{
	public:
		Engine(SharedPtr<Vk::VulkanApp> vulkanApp);
		~Engine();

		/** Executes the main loop and calls Engine::Tick() every frame. */
		void Run();

		/** Handles Win32 messages and forwards them to needed modules. */
		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/** Registers a callback function to be called in Engine::Update(). */
		template<class ...Args>
		void RegisterUpdateCallback(Args &&...args)
		{
			mUpdateCallback = std::bind(std::forward<Args>(args)...);
		}

		/** Registers a callback function to be called in Engine::Render(). */
		template<class ...Args>
		void RegisterRenderCallback(Args &&...args)
		{
			mRenderCallback = std::bind(std::forward<Args>(args)...);
		}

		/** Registers a callback function to be called in Engine::Render(). */
		template<class ...Args>
		void RegisterDestroyCallback(Args &&...args)
		{
			mDestroyCallback = std::bind(std::forward<Args>(args)...);
		}

	private:
		/**
		 * Calls the per frame update function of all modules in the engine.
		 * 
		 * @note Also calls registered application callback functions.
		 */
		void Tick();

		/** Starts all the modules included in the engine. */
		void StartModules();

		/** Updates all modules included in the engine. */
		void Update();

		/** Renders all modules included in the engine. */
		void Render();

		/** Peeks and dispatches Win32 messages. */
		bool DispatchMessages();

	private:
		SharedPtr<Vk::VulkanApp> mVulkanApp;
		std::function<void()> mUpdateCallback;
		std::function<void()> mRenderCallback;
		std::function<void()> mDestroyCallback;
	};

	/** Returns an instance to the Engine module. */
	Engine& gEngine();
}
