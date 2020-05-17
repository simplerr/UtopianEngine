#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <glm/glm.hpp>
#include <chrono>
#include <vulkan/vulkan.h>

#include "imgui/imgui.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/ImguiEffect.h"
#include "vulkan/handles/Buffer.h"

namespace Utopian
{
	class Texture;
	class Texture2;

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
		ImTextureID AddTexture(VkImageView imageView, const VkSampler = VK_NULL_HANDLE, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		ImTextureID AddTexture(const Vk::Texture& texture, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

	private:
		Utopian::Vk::VulkanApp* mVulkanApp;
		Utopian::Vk::CommandBuffer* mCommandBuffer;
		Utopian::Vk::Buffer mVertexBuffer;
		Utopian::Vk::Buffer mIndexBuffer;
		SharedPtr<Vk::ImguiEffect> mImguiEffect;
		SharedPtr<Vk::DescriptorPool> mTextureDescriptorPool;
		SharedPtr<Vk::Sampler> mSampler;
		SharedPtr<Vk::Texture> mTexture;

		ImDrawVert* mMappedVertices;
		ImDrawIdx* mMappedIndices;

		int32_t mVertexCount = 0;
		int32_t mIndexCount = 0;
		
		float mScale = 1.0f;

		// Note: This should be handled by the Timer component.
		// But since the calls to ImGui::NewFrame() and ImGui::Render() currently are not 
		// called at the same frequency as UIOverlay::Update.
		std::chrono::high_resolution_clock::time_point mLastFrameTime;
		double mDeltaTime;

		std::vector<VkDescriptorSet> mTextureDescriptorsToFree;

		static bool mImguiVisible;
	};
}