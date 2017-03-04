#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{
	class VulkanBase;

	/*
	Base class that handles uniform buffer creation
	Derive your own uniform buffers from it and implement the memory mapping function
	*/
	class UniformBuffer
	{
	public:
		void Cleanup(VkDevice device)
		{
			// Cleanup uniform buffer
			vkDestroyBuffer(device, mBuffer, nullptr);
			vkFreeMemory(device, mMemory, nullptr);
		}

		// [NOTE] This has to be called after elements have been added to vectors, since GetSize() needs to return the correct size
		// Creates a VkBuffer and maps it to a VkMemory (VulkanBase::CreateBuffer())
		void CreateBuffer(VulkanBase* vulkanBase, VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// This is where the data gets transfered to device memory w/ vkMapMemory,vkUnmapMemory and memcpy
		virtual void UpdateMemory(VkDevice device) = 0;

		virtual int GetSize() = 0;

		VkDescriptorBufferInfo GetDescriptor() { return mDescriptor; }

	protected:
		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo mDescriptor;
	};
}
