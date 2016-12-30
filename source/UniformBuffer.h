#pragma once
#include <vulkan/vulkan.h>
#include "VulkanBase.h"

namespace VulkanLib
{
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

		void CreateBuffer(VulkanBase* vulkanBase, VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			vulkanBase->CreateBuffer(
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				propertyFlags,
				GetSize(),	// Virtual function
				nullptr,
				&mBuffer,
				&mMemory);

			// mBuffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
			// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
			mDescriptor.buffer = mBuffer;
			mDescriptor.range = GetSize();
			mDescriptor.offset = 0;
		}

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
