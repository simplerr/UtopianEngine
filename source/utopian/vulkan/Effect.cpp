#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/Texture.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/EffectManager.h"
#include "core/renderer/Renderer.h"
#include "Effect.h"

namespace Utopian::Vk
{
	Effect::Effect(Device* device, RenderPass* renderPass, const EffectCreateInfo& effectCreateInfo)
	{
		mShaderCreateInfo = effectCreateInfo.shaderDesc;
		mRenderPass = renderPass;
		mDevice = device;
		mDescriptorPool = std::make_shared<DescriptorPool>(device);

		mPipeline = std::make_shared<Pipeline>(effectCreateInfo.pipelineDesc, mDevice, mRenderPass);

		CreatePipeline();
	}

	Effect::~Effect()
	{

	}

	SharedPtr<Effect> Effect::Create(Device* device, RenderPass* renderPass, const EffectCreateInfo& effectCreateInfo)
	{
		SharedPtr<Effect> effect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderPass, effectCreateInfo);

		return effect;
	}

	void Effect::SetShaderCreateInfo(const ShaderCreateInfo& shaderCreateInfo)
	{
		mShaderCreateInfo = shaderCreateInfo;
	}

	void Effect::CreatePipeline()
	{
		// Note: Todo: Hack: the code in the if-statement should be in the constructor and not here
		// but since there needs to a constructor that does not take a ShaderCreateInfo argument
		// mShaderCreateInfo is only set after SetShaderCreateInfo() has been called.
		if (mShader == nullptr)
		{
			mShader = gShaderFactory().CreateShaderOnline(mShaderCreateInfo);

			assert(mShader);

			CreatePipelineInterface(mShader.get(), mDevice);
		}

		mPipeline->Create(mShader.get(), mPipelineInterface.get());
	}

	bool Effect::RecompileShader()
	{
		bool result = false;
		SharedPtr<Shader> shader = gShaderFactory().CreateShaderOnline(mShaderCreateInfo);

		if (shader != nullptr)
		{
			mShader = shader;

		   // Destroy pipeline if already created
         if (mPipeline->IsCreated())
            gRenderer().QueueDestroy(mPipeline->GetVkHandle());

			CreatePipeline();
			result = true;
		}

		return result;
	}

	void Effect::CreatePipelineInterface(Shader* shader, Device* device)
	{
		mPipelineInterface = std::make_shared<PipelineInterface>(mDevice);

		for (int i = 0; i < shader->compiledShaders.size(); i++)
		{
			// Uniform blocks
			for (auto& iter : shader->compiledShaders[i]->reflection.uniformBlocks)
			{
				mPipelineInterface->AddUniformBuffer(iter.second.set, iter.second.binding, VK_SHADER_STAGE_ALL);
				mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
			}

			// Storage buffers
			for (auto& iter : shader->compiledShaders[i]->reflection.storageBuffers)
			{
				mPipelineInterface->AddStorageBuffer(iter.second.set, iter.second.binding, VK_SHADER_STAGE_ALL);
				mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
			}

			// Combined image samplers
			for (auto& iter : shader->compiledShaders[i]->reflection.combinedSamplers)
			{
				mPipelineInterface->AddCombinedImageSampler(iter.second.set, iter.second.binding, VK_SHADER_STAGE_ALL, iter.second.arraySize);
				mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, iter.second.arraySize);
			}

			// Push constants
			for (auto& iter : shader->compiledShaders[i]->reflection.pushConstants)
			{
				mPipelineInterface->AddPushConstantRange(iter.second.size, VK_SHADER_STAGE_ALL);
			}
		}

		mPipelineInterface->Create();
		mDescriptorPool->Create();

		for (uint32_t set = 0; set < mPipelineInterface->GetNumDescriptorSets(); set++)
		{
			mDescriptorSets.push_back(DescriptorSet(device, this, set, mDescriptorPool.get()));
			mVkDescriptorSets.push_back(mDescriptorSets[set].GetVkHandle());	// Todo
		}
	}

	void Effect::BindUniformBuffer(std::string name, const VkDescriptorBufferInfo* bufferInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindUniformBuffer(name, bufferInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindStorageBuffer(std::string name, const VkDescriptorBufferInfo* bufferInfo)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindStorageBuffer(name, bufferInfo);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindUniformBuffer(std::string name, const ShaderBuffer& shaderBlock)
	{
		BindUniformBuffer(name, shaderBlock.GetDescriptor());
	}

	void Effect::BindCombinedImage(std::string name, const Texture& texture)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, texture.GetDescriptor(), 1u);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindCombinedImage(std::string name, const Image& image, const Sampler& sampler)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, image, sampler);
		descriptorSet.UpdateDescriptorSets();
	}

	void Effect::BindCombinedImage(std::string name, const TextureArray& textureArray)
	{
		DescriptorSet& descriptorSet = mDescriptorSets[mShader->NameToSet(name)];
		descriptorSet.BindCombinedImage(name, textureArray.GetDescriptor(), textureArray.GetNumImages());
		descriptorSet.UpdateDescriptorSets();
	}

	const DescriptorSet& Effect::GetDescriptorSet(uint32_t set) const
	{
		if (set < 0 || set >= mDescriptorSets.size())
			assert(0);

		return mDescriptorSets[set];
	}

	PipelineInterface* Effect::GetPipelineInterface()
	{
		return mPipelineInterface.get();
	}

	Shader* Effect::GetShader() const
	{
		return mShader.get();
	}

	std::string Effect::GetVertexShaderPath() const
	{
		return mShaderCreateInfo.fragmentShaderPath;
	}

	Pipeline* Effect::GetPipeline()
	{
		return mPipeline.get();
	}

	const VkDescriptorSet* Effect::GetDescriptorSets() const
	{
		return mVkDescriptorSets.data();
	}

	uint32_t Effect::GetNumDescriptorSets() const
	{
		return mVkDescriptorSets.size();
	}

	ShaderCreateInfo Effect::GetShaderCreateInfo() const
	{
		return mShaderCreateInfo;
	}
}