#include "vulkan/BasicEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/PipelineInterface.h"

namespace Vulkan
{
	BasicEffect::BasicEffect(Renderer* renderer)
	{
		uniformBuffer.Create(renderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorPool = new Vulkan::DescriptorPool(renderer->GetDevice());
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();

		mVertexDescription = new Vulkan::VertexDescription();
		mVertexDescription->AddBinding(0, sizeof(BasicVertex), VK_VERTEX_INPUT_RATE_VERTEX);					
		mVertexDescription->AddAttribute(0, Vulkan::Vec4Attribute());	
		mVertexDescription->AddAttribute(1, Vulkan::Vec4Attribute());	

		Vulkan::Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/basic/basic.vert.spv", "data/shaders/basic/basic.frag.spv");
		mBasicPipeline = new Vulkan::Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);

		// EXPERIMENT
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(renderer->GetDevice());

		mBasicPipeline->SetPipelineInterface(&mPipelineInterface);

		VkDescriptorSetLayout setLayout0 = mPipelineInterface.GetDescriptorSetLayout(SET_0);

		mDescriptorSet = new Vulkan::DescriptorSet(renderer->GetDevice(), setLayout0, mDescriptorPool);
		mDescriptorSet->AllocateDescriptorSets();
		mDescriptorSet->BindUniformBuffer(0, &uniformBuffer.GetDescriptor());
		mDescriptorSet->UpdateDescriptorSets();

		mBasicPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mBasicPipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mBasicPipeline->Create();

	}
}