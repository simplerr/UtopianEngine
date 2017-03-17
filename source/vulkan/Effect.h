#pragma once

#include <vulkan\vulkan.h>

namespace Vulkan
{
	class Renderer;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Pipeline;
	class PipelineLayout;
	class VertexDescription;
	class Shader;

	class Effect
	{
	public:
		Effect(Renderer* renderer) 
		{

		};
	//private:

		DescriptorPool* mDescriptorPool;
		DescriptorSetLayout* mDescriptorSetLayout;
		DescriptorSet* mDescriptorSet;
		Pipeline* mGeometryPipeline;
		Pipeline* mBasicPipeline;
		PipelineLayout* mPipelineLayout;
		VertexDescription* mVertexDescription;
		Shader* mShader;
	};
}
