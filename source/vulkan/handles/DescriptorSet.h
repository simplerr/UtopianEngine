#pragma once

#include <vector>
#include <map>
#include "vulkan/handles/Handle.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	/*
	Wraps VkDescriptorSetLayout and VkDescriptorSet to make them easier to work with
	*/
	class DescriptorSet
	{
	public:
		DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		DescriptorSet(Device* device, Effect* pipeline, uint32_t set, DescriptorPool* descriptorPool);
		DescriptorSet();
		~DescriptorSet();

		void Create(Device* device, Effect* pipeline, uint32_t set, DescriptorPool* descriptorPool);
		void UpdateDescriptorSets();

		void BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
		void BindCombinedImage(uint32_t binding, Image* image, Sampler* sampler);

		// Bind a uniform buffer by name, note that name must match the GLSL representation
		void BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo);
		void BindCombinedImage(std::string name, Image* image, Sampler* sampler);

		// NOTE: TODO: Legacy
		void UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);	// Will be used for changing the texture

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	private:
		Device* mDevice;
		DescriptorSetLayout* mSetLayout;
		std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
		std::map<int, VkDescriptorImageInfo> mImageInfoMap;

		// The shader that this descriptor set was created from.
		// This is used in order to get access to the shader reflection
		// to perform the string -> binding lookup.
		// Note: Maybe don't belong here.
		SharedPtr<Shader> mShader;
	};

	/*
	Wraps VkDescriptorPool to make it easier to work with
	Can be created directly from a descriptor layout vector
	*/
	class DescriptorPool : public Handle<VkDescriptorPool>
	{
	public:
		DescriptorPool(Device* device);
		DescriptorPool();
		void AddDescriptor(VkDescriptorType type, uint32_t count);
		void Create();
		void Create(Device* device);
	private:
		std::vector<VkDescriptorPoolSize> mDescriptorSizes;
	};
}
