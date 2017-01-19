#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include "Handle.h"

namespace VulkanLib
{
	class VertexDescription;
	class RenderPass;
	class PipelineLayout;
	class Device;
	class Shader;

	class Pipeline : public Handle<VkPipeline>
	{
	public:
		Pipeline(Device* device, PipelineLayout* pipelineLayout, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader);

		void Create(PipelineLayout* pipelineLayout, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader);
	private:
	};
}
