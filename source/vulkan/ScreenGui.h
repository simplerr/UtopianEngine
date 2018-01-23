#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include "vulkan/ScreenQuadEffect.h"
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class ScreenGui
	{
	public:
		struct TextureQuad
		{
			TextureQuad(uint32_t l, uint32_t t, uint32_t w, uint32_t h) {
				left = l;
				top = t;
				width = w;
				height = h;
			}

			uint32_t left;
			uint32_t top;
			uint32_t width;
			uint32_t height;
			Vulkan::DescriptorSet* descriptorSet;
		};

		struct Vertex
		{
			glm::vec3 pos;
			glm::vec2 uv;
		};

		ScreenGui(Renderer* renderer);

		void Render(Renderer* renderer, CommandBuffer* commandBuffer);

		void AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Vulkan::Image* image, Vulkan::Sampler* sampler);
		void AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Vulkan::Texture* texture);
	private:
		Vulkan::Renderer* mRenderer;
		Vulkan::ScreenQuadEffect mEffect;
		Vulkan::Buffer* mVertexBuffer;
		Vulkan::Buffer* mIndexBuffer;

		std::vector<TextureQuad> mQuadList;
	};
}
