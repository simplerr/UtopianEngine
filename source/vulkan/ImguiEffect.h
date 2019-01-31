#pragma once

#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "vulkan/Effect.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class ImguiEffect : public Effect
	{
	public:
		struct PushConstantBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		ImguiEffect(Device* device, RenderPass* renderPass);

		virtual void UpdateMemory();

		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL
		Utopian::Vk::Texture* mTexture;
		Utopian::Vk::RenderPass* mRenderPass;
		SharedPtr<VertexDescription> mVertexDescription;
	};
}
