#pragma once

#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "vulkan/Effect.h"

namespace Vulkan
{
	class ImguiEffect : public Effect
	{
	public:
		struct PushConstantBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		ImguiEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory(Device* device);

		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL
		Vulkan::Texture* mTexture;
		Vulkan::RenderPass* mRenderPass;
	};
}
