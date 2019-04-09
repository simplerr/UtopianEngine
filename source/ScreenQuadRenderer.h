#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"
#include "utility/Module.h"
#include "vulkan/Effect.h"

namespace Utopian
{
	class ScreenQuad
	{
	public:
		ScreenQuad(uint32_t l, uint32_t t, uint32_t w, uint32_t h, uint32_t layer) {
			left = l;
			top = t;
			width = w;
			height = h;
			visible = true;
			this->layer = layer;
		}

		void SetVisible(bool visible) {
			this->visible = visible;
		}

		uint32_t left;
		uint32_t top;
		uint32_t width;
		uint32_t height;
		uint32_t layer; // Layer 0 is rendered last and gets on top of every other layer.
		bool visible;
		Utopian::Vk::DescriptorSet* descriptorSet;
	};

	class ScreenQuadRenderer : public Module<ScreenQuadRenderer>
	{
		struct PushConstantBlock
		{
			glm::mat4 world;
		};

	public:
		ScreenQuadRenderer(Vk::VulkanApp* vulkanApp);

		void Render(Vk::VulkanApp* vulkanApp);
		void ToggleVisible(uint32_t layer);
		void SetVisible(uint32_t layer, bool visible);

		SharedPtr<ScreenQuad> AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler, uint32_t layer = 0u);
		SharedPtr<ScreenQuad> AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, VkImageView imageView, Utopian::Vk::Sampler* sampler, uint32_t layer = 0u);
		SharedPtr<ScreenQuad> AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture, uint32_t layer = 0u);
	private:
		void CreateQuadBuffers();

		SharedPtr<Utopian::Vk::Effect> mEffect;
		SharedPtr<Utopian::Vk::DescriptorPool> mDescriptorPool;
		Utopian::Vk::VulkanApp* mVulkanApp;
		Utopian::Vk::CommandBuffer* mCommandBuffer;

		struct {
			Vk::Buffer* vertexBuffer;
			Vk::Buffer* indexBuffer;
		} mScreenQuad;

		std::vector<SharedPtr<ScreenQuad>> mQuadList;

		const uint32_t NUM_MAX_LAYERS = 3u;
	};

	ScreenQuadRenderer& gScreenQuadUi();
}
