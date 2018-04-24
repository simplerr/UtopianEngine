#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include "vulkan/ScreenQuadEffect.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
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
			Utopian::Vk::DescriptorSet* descriptorSet;
		};

		ScreenGui(Renderer* renderer);

		void Render(Renderer* renderer, CommandBuffer* commandBuffer);

		void AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler);
		void AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture);
	private:
		Utopian::Vk::Renderer* mRenderer;
		Utopian::Vk::ScreenQuadEffect mEffect;
		Utopian::Vk::Buffer* mVertexBuffer;
		Utopian::Vk::Buffer* mIndexBuffer;

		std::vector<TextureQuad> mQuadList;
	};
}
