#pragma once

#include <array>
#include <vector>
#include <map>
#include <string>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/ShaderFactory.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
   /** Used as input when creating an Effect, contains parameters to control the entire pipeline setup. */
	struct EffectCreateInfo
	{
		ShaderCreateInfo shaderDesc;
		PipelineDesc pipelineDesc;
	};

	class Effect
	{
	public:
		/** @note CreatePipeline() must be called explicitly after constructor. */
		Effect(Device* device, RenderPass* renderPass, const EffectCreateInfo& effectCreateInfo);
		~Effect();

		/** Creates the Effect and adds it to the Effect manager. */
		static SharedPtr<Effect> Create(Device* device, RenderPass* renderPass, const EffectCreateInfo& effectCreateInfo);

		/** Loads the shaders from file again, compiles it, performs reflection and rebuilds the pipeline. */
		bool RecompileShader();

		/**
		 * Functions used to bind shader resources by name.
		 * @note the name must match the name in the GLSL shader.
		 * @note uses the internal shader reflection to perform name -> set ID mapping.
		 */
		void BindUniformBuffer(std::string name, const VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(std::string name, const VkDescriptorBufferInfo* bufferInfo);
		void BindUniformBuffer(std::string name, const ShaderBuffer& shaderBlock);
		void BindCombinedImage(std::string name, const Texture& texture);
		void BindCombinedImage(std::string name, const Image& image, const Sampler& sampler);
		void BindCombinedImage(std::string name, const TextureArray& textureArray);

		/**
		 * Returns a descriptor set by index.
		 * Needs to be used if you want to bind additional descriptor sets that not are part of the Effect itself,
		 * for example Meshes contains their own descriptor set for their texture.
		 * @note This should only be used in rare cases.
		 */
		const DescriptorSet& GetDescriptorSet(uint32_t set) const;

		/** Returns a pointer to the Pipeline object. It is expected to be modified. */
		Pipeline* GetPipeline();

		const VkDescriptorSet* GetDescriptorSets() const;
		ShaderCreateInfo GetShaderCreateInfo() const;
		std::string GetVertexShaderPath() const;
		uint32_t GetNumDescriptorSets() const;
		Shader* GetShader() const;
		PipelineInterface* GetPipelineInterface();
	protected:
		void SetShaderCreateInfo(const ShaderCreateInfo& shaderCreateInfo);

		SharedPtr<Pipeline> mPipeline;
	private:
		void CreatePipeline();
		void CreatePipelineInterface(Shader*, Device* device);

		RenderPass* mRenderPass = nullptr;
		Device* mDevice = nullptr;
		SharedPtr<Shader> mShader;
		SharedPtr<PipelineInterface> mPipelineInterface;
		std::vector<DescriptorSet> mDescriptorSets;
		std::vector<VkDescriptorSet> mVkDescriptorSets;
		ShaderCreateInfo mShaderCreateInfo;
		SharedPtr<DescriptorPool> mDescriptorPool;
	};
}
