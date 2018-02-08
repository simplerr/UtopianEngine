#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <glm/glm.hpp>
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
		UIOverlay(uint32_t width, uint32_t height, Utopian::Vk::Renderer* renderer);
		~UIOverlay();

		void Update();
		void Resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers);

		void PrepareResources();
		void UpdateCommandBuffers();

		Utopian::Vk::CommandBuffer* GetCommandBuffer() const;

		static void TextV(const char* format, ...);

	private:
		Utopian::Vk::Renderer* mRenderer;
		Utopian::Vk::CommandBuffer* mCommandBuffer;
		Utopian::Vk::Buffer mVertexBuffer;
		Utopian::Vk::Buffer mIndexBuffer;
		Utopian::Vk::ImguiEffect mImguiEffect;

		ImDrawVert* mMappedVertices;
		ImDrawIdx* mMappedIndices;

		int32_t mVertexCount = 0;
		int32_t mIndexCount = 0;
		
		bool mVisible = true;
		float mScale = 1.0f;
	};
}