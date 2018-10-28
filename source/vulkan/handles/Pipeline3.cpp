#include "vulkan/ShaderFactory.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/CommandBuffer.h"
#include "Pipeline3.h"
#include "RenderPass.h"
#include "PipelineLayout.h"

namespace Utopian::Vk
{
	Pipeline3::Pipeline3(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, SharedPtr<Shader> shader)
	{
		Init(device, renderPass, vertexDescription, shader);
	}

	void Pipeline3::Init(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, SharedPtr<Shader> shader)
	{
		mPipeline = std::make_shared<Pipeline>(device, renderPass);
		mRenderPass = renderPass;
		mVertexDescription = vertexDescription;
		mShader = shader;

		CreatePipelineInterface(shader, device);
	}

	void Pipeline3::Create()
	{
		mPipeline->Create(mVertexDescription, mShader.get(), &mPipelineInterface);
	}

	void Pipeline3::CreatePipelineInterface(const SharedPtr<Shader>& shader, Device* device)
	{
		for (int i = 0; i < shader->compiledShaders.size(); i++)
		{
			// Uniform blocks
			for (auto& iter : shader->compiledShaders[i]->reflection.uniformBlocks)
			{
				mPipelineInterface.AddUniformBuffer(iter.second.set, iter.second.binding, shader->compiledShaders[i]->shaderStage);			// Eye ubo
				mDescriptorPool.AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
			}

			// Combined image samplers
			for (auto& iter : shader->compiledShaders[i]->reflection.combinedSamplers)
			{
				mPipelineInterface.AddCombinedImageSampler(iter.second.set, iter.second.binding, shader->compiledShaders[i]->shaderStage);	// Eye ubo
				mDescriptorPool.AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
			}
		}

		mPipelineInterface.CreateLayouts(device);
		mDescriptorPool.Create(device);

		for (uint32_t set = 0; set < mPipelineInterface.GetNumDescriptorSets(); set++)
		{
			mDescriptorSets.push_back(DescriptorSet(device, this, set, &mDescriptorPool));
			mVkDescriptorSets.push_back(mDescriptorSets[set].descriptorSet);	// Todo
		}
	}

	void Pipeline3::BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindUniformBuffer(name, bufferInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Pipeline3::BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindStorageBuffer(name, bufferInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Pipeline3::BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, imageInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Pipeline3::BindCombinedImage(std::string name, Image* image, Sampler* sampler)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, image, sampler);
		descriptorSet.UpdateDescriptorSets();
	}

	void Pipeline3::BindDescriptorSets(CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdBindDescriptorSet(GetPipelineInterface()->GetPipelineLayout(), mVkDescriptorSets.size(), mVkDescriptorSets.data(), VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
	}

	PipelineInterface* Pipeline3::GetPipelineInterface()
	{
		return &mPipelineInterface;
	}

	SharedPtr<Shader> Pipeline3::GetShader()
	{
		return mShader;
	}
	
	Pipeline* Pipeline3::GetPipeline()
	{
		return mPipeline.get();
	}
}