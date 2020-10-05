#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "imgui/imgui.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/Effect.h"
#include "utility/Timer.h"

namespace Utopian
{
	class Texture;

	class ImGuiRenderer 
	{
	public:
		ImGuiRenderer(Vk::VulkanApp* vulkanApp, uint32_t width, uint32_t height);
		~ImGuiRenderer();

		/** Uploads the ImGui generated vertex and index buffers to the GPU. Also records to a secondary command buffer. */
		void Render();
		void NewFrame();
		void EndFrame();
		void Resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers);
		void PrepareResources();
		static bool IsMouseInsideUi();
		static bool IsKeyboardCapture();
		static void KeydownCallback(char key);
		ImTextureID AddImage(const Vk::Image& image);
		void FreeTexture(ImTextureID textureId);

		Utopian::Vk::CommandBuffer* GetCommandBuffer() const;

		// Utility helper functions
		static void TextV(const char* format, ...);
		static void BeginWindow(std::string label, glm::vec2 position, float itemWidth);
		static void EndWindow();
		void ToggleVisible();

		// Frees UI textures.
		// Needs to be called when the textures are not in an active command buffer
		void GarbageCollect();

	private:
		void UpdateCommandBuffers();
		void ResetKeyStates();
		void StartDockSpace();
		void SetDarkTheme();
		void SetLightTheme();

	private:
		struct PushConstantBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		Vk::VulkanApp* mVulkanApp;
		SharedPtr<Vk::CommandBuffer> mCommandBuffer;
		SharedPtr<Vk::Buffer> mVertexBuffer;
		SharedPtr<Vk::Buffer> mIndexBuffer;
		SharedPtr<Vk::Effect> mImguiEffect;
		SharedPtr<Vk::DescriptorPool> mTextureDescriptorPool;
		SharedPtr<Vk::VertexDescription> mVertexDescription;
		SharedPtr<Vk::Sampler> mSampler;
		SharedPtr<Vk::Texture> mTexture;

		ImDrawVert* mMappedVertices;
		ImDrawIdx* mMappedIndices;

		int32_t mVertexCount = 0;
		int32_t mIndexCount = 0;

		float mScale = 1.0f;
		Timestamp mLastFrameTime;
		std::vector<VkDescriptorSet> mTextureDescriptorsToFree;
		static bool mImguiVisible;
	};
}