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

namespace Utopian::Vk 
{
	class Texture;


	class UIOverlay 
	{
	public:
		UIOverlay(uint32_t width, uint32_t height, Utopian::Vk::VulkanApp* vulkanApp);
		~UIOverlay();

		void Update();
		void NewFrame();
		void Render();
		void Resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers);

		void PrepareResources();
		void UpdateCommandBuffers();

		ImTextureID AddTexture(VkImageView imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Utopian::Vk::CommandBuffer* GetCommandBuffer() const;

		// Utility helper functions
		static void TextV(const char* format, ...);
		static void BeginWindow(std::string label, glm::vec2 position, float itemWidth);
		static void EndWindow();
		void ToggleVisible();

	private:
		Utopian::Vk::VulkanApp* mVulkanApp;
		Utopian::Vk::CommandBuffer* mCommandBuffer;
		Utopian::Vk::Buffer mVertexBuffer;
		Utopian::Vk::Buffer mIndexBuffer;
		SharedPtr<Vk::ImguiEffect> mImguiEffect;
		SharedPtr<DescriptorPool> mTextureDescriptorPool;
		SharedPtr<Vk::Sampler> mSampler;
		Utopian::Vk::Texture* mTexture;

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
	};
}