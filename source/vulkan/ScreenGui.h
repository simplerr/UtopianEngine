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
			TextureQuad(uint32_t l, uint32_t t, uint32_t w, uint32_t h, uint32_t layer) {
				left = l;
				top = t;
				width = w;
				height = h;
				this->layer = layer;
			}

			uint32_t left;
			uint32_t top;
			uint32_t width;
			uint32_t height;
			uint32_t layer; // Layer 0 is rendered last and gets on top of every other layer.
			Utopian::Vk::DescriptorSet* descriptorSet;
		};

		ScreenGui(Renderer* renderer);

		void Render(Renderer* renderer, CommandBuffer* commandBuffer);

		void AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler, uint32_t layer = 0u);
		void AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture, uint32_t layer = 0u);
	private:
		Utopian::Vk::Renderer* mRenderer;
		Utopian::Vk::ScreenQuadEffect mEffect;

		std::vector<TextureQuad> mQuadList;

		const uint32_t NUM_MAX_LAYERS = 3u;
	};
}
