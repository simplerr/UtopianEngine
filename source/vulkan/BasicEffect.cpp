#include "vulkan/BasicEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline.h"

namespace Vulkan
{
	BasicEffect::BasicEffect(Renderer* renderer)
		: Effect(renderer)
	{
		uniformBuffer.Create(renderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSetLayout = new Vulkan::DescriptorSetLayout(renderer->GetDevice());
		mDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		mDescriptorSetLayout->Create();

		mPipelineLayout = new Vulkan::PipelineLayout(renderer->GetDevice());
		mPipelineLayout->AddDescriptorSetLayout(mDescriptorSetLayout);
		mPipelineLayout->AddPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstantBlock));
		mPipelineLayout->Create();

		mDescriptorPool = new Vulkan::DescriptorPool(renderer->GetDevice());
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();

		mDescriptorSet = new Vulkan::DescriptorSet(renderer->GetDevice(), mDescriptorSetLayout, mDescriptorPool);
		mDescriptorSet->AllocateDescriptorSets();
		mDescriptorSet->BindUniformBuffer(0, &uniformBuffer.GetDescriptor());
		mDescriptorSet->UpdateDescriptorSets();

		mVertexDescription = new Vulkan::VertexDescription();
		mVertexDescription->AddBinding(0, sizeof(BasicVertex), VK_VERTEX_INPUT_RATE_VERTEX);					
		mVertexDescription->AddAttribute(0, Vulkan::Vec3Attribute());	

		Vulkan::Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/basic/basic.vert.spv", "data/shaders/basic/basic.frag.spv");
		mBasicPipeline = new Vulkan::Pipeline(renderer->GetDevice(), mPipelineLayout, renderer->GetRenderPass(), mVertexDescription, shader);
		mBasicPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		//mPipeline->mRasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		mBasicPipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mBasicPipeline->Create();
	}
}