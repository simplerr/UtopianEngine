/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

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

namespace Vulkan 
{
	class Texture;


	class UIOverlay 
	{
	private:
		void prepareResources();
		void updateCommandBuffers();
	public:
		UIOverlay(uint32_t width, uint32_t height, Vulkan::Renderer* renderer);
		~UIOverlay();

		void update();
		void resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers);

		void submit(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo);

		Vulkan::CommandBuffer* GetCommandBuffer() const;

		bool header(const char* caption);
		bool checkBox(const char* caption, bool* value);
		bool checkBox(const char* caption, int32_t* value);
		bool inputFloat(const char* caption, float* value, float step, uint32_t precision);
		bool sliderFloat(const char* caption, float* value, float min, float max);
		bool sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
		bool comboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
		bool button(const char* caption);
		void text(const char* formatstr, ...);

	private:
		Vulkan::TextureLoader* mTextureLoader;
		Vulkan::Texture* mTexture;
		Vulkan::Renderer* mRenderer;
		Vulkan::CommandBuffer* mCommandBuffer;
		Vulkan::Buffer mVertexBuffer;
		Vulkan::Buffer mIndexBuffer;
		Vulkan::ImguiEffect mImguiEffect;

		ImDrawVert* mMappedVertices;
		ImDrawIdx* mMappedIndices;

		uint32_t width;
		uint32_t height;

		int32_t vertexCount = 0;
		int32_t indexCount = 0;
		
		bool visible = true;
		float scale = 1.0f;
	};
}