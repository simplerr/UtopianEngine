#include "vulkan/ShaderFactory.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/CommandBuffer.h"
#include "Effect.h"
#include "RenderPass.h"
#include "PipelineLayout.h"

namespace Utopian::Vk
{
	Effect::Effect(Device* device, RenderPass* renderPass, std::string vertexShader, std::string fragmentShader)
	{
		Init(device, renderPass, vertexShader, fragmentShader);
	}

	void Effect::Init(Device* device, RenderPass* renderPass, std::string vertexShader, std::string fragmentShader)
	{
		mPipeline = std::make_shared<Pipeline>(device, renderPass);
		mRenderPass = renderPass;
		mShader = gShaderManager().CreateShaderOnline(vertexShader, fragmentShader);

		CreatePipelineInterface(mShader, device);
	}

	void Effect::CreatePipeline()
	{
		mPipeline->Create(mShader.get(), &mPipelineInterface);
	}

	void Effect::CreatePipelineInterface(const SharedPtr<Shader>& shader, Device* device)
	{
		for (int i = 0; i < shader->compiledShaders.size(); i++)
		{
			// Uniform blocks
			for (auto& iter : shader->compiledShaders[i]->reflection.uniformBlocks)
			{
				mPipelineInterface.AddUniformBuffer(iter.second.set, iter.second.binding, shader->compiledShaders[i]->shaderStage);
				mDescriptorPool.AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
			}

			// Combined image samplers
			for (auto& iter : shader->compiledShaders[i]->reflection.combinedSamplers)
			{
				mPipelineInterface.AddCombinedImageSampler(iter.second.set, iter.second.binding, shader->compiledShaders[i]->shaderStage);
				mDescriptorPool.AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
			}

			// Push constants
			for (auto& iter : shader->compiledShaders[i]->reflection.pushConstants)
			{
				mPipelineInterface.AddPushConstantRange(iter.second.size, shader->compiledShaders[i]->shaderStage);
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

	void Effect::BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindUniformBuffer(name, bufferInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindStorageBuffer(name, bufferInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, imageInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindCombinedImage(std::string name, Image* image, Sampler* sampler)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, image, sampler);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindDescriptorSets(CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdBindDescriptorSet(GetPipelineInterface()->GetPipelineLayout(), mVkDescriptorSets.size(), mVkDescriptorSets.data(), VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
	}

	DescriptorSet& Effect::GetDescriptorSet(uint32_t set)
	{
		if (set < 0 || set >= mDescriptorSets.size())
			assert(0);

		return mDescriptorSets[set];
	}

	PipelineInterface* Effect::GetPipelineInterface()
	{
		return &mPipelineInterface;
	}

	SharedPtr<Shader> Effect::GetShader()
	{
		return mShader;
	}
	
	Pipeline* Effect::GetPipeline()
	{
		return mPipeline.get();
	}
}