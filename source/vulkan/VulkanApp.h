#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "VulkanBase.h"
#include "VertexDescription.h"
#include "ShaderBuffer.h"

namespace Utopian
{
	class ImGuiRenderer;
}

namespace Utopian::Vk
{
	// Todo: Move from here
	struct PushConstantBlock {
		PushConstantBlock() {
			world = glm::mat4();
			//worldInvTranspose = glm::mat4();
			// color = glm::vec4(1.0f);
			// textureTiling = glm::vec2(1.0f, 1.0f);
		}

		PushConstantBlock(glm::mat4 _world, glm::vec4 _color = glm::vec4(1.0f), glm::vec2 tiling = glm::vec2(1.0f, 1.0f)) {
			world = _world;
			color = _color;

			// textureTiling = tiling;

			// Note: This needs to be done to have the physical world match the rendered world.
			// See https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/ for more information.
			world[3][0] = -world[3][0];
			world[3][1] = -world[3][1];
			world[3][2] = -world[3][2];
		}
		
		glm::mat4 world;
		glm::vec4 color; // r, g, b, brightness

	 	// These exceeds the 128 byte limit
		// glm::vec2 textureTiling;
		// glm::vec2 pad;
	};

	/**
	 * Handles the creation of the Vulkan instance, device, swap chain and
	 * submits the primary command buffer including all the secondary ones.
	 * 
	 * By creating a VulkanApp object the application is ready to start configuring and
	 * rendering to the window using Vulkan.
	 */
	class VulkanApp : public VulkanBase
	{
	public:
		VulkanApp(Window* window);
		~VulkanApp();

		void Prepare();

		/**
		 * Adds a secondary command buffer that will be executed by
		 * VulkanApp::Render().
		 * 
		 * @note For example one secondary command buffer is used by the
		 * ScreenQuadUi and ImGui.
		 */
		void AddSecondaryCommandBuffer(CommandBuffer* commandBuffer);

		/** Submits the primary command buffer to the graphics queue. */
		virtual void Render();

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void UpdateImgui();
		void ToggleUi();

		ImGuiRenderer* GetUiOverlay();

		void SetImguiRenderer(ImGuiRenderer* imguiRenderer);
		void SetClearColor(glm::vec4 color);
		glm::vec4 GetClearColor();

	private:
		/**
		 * Collects all the secondary command buffers that has been added by
		 * the application and executes them from the primary command buffer.
		 */
		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);

	private:
		CommandBuffer*					mPrimaryCommandBuffer;
		std::vector<CommandBuffer*>		mSecondaryCommandBuffers;
		glm::vec4						mClearColor;
		ImGuiRenderer*					mImGuiRenderer = nullptr;
	};
}	// VulkanLib namespace
