#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{
	class Device;
	class Buffer;

	/*
		Base class for uniform and storage buffer.
	*/
	class ShaderBuffer
	{
	public:
		virtual ~ShaderBuffer();

		// [NOTE] This has to be called after elements have been added to vectors, since GetSize() needs to return the correct size
		// Creates a VkBuffer and maps it to a VkMemory (VulkanBase::CreateBuffer())
		void Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		void MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data);
		void UnmapMemory();

		// This is where the data gets transfered to device memory w/ vkMapMemory,vkUnmapMemory and memcpy
		virtual void UpdateMemory(VkDevice device) = 0;

		virtual int GetSize() = 0;

		VkDescriptorBufferInfo GetDescriptor() { return mDescriptor; }
		Buffer* GetBuffer() { return mBuffer; }

	protected:
		Buffer* mBuffer;
		VkDescriptorBufferInfo mDescriptor;
	};
}
