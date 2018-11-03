#include "vulkan/handles/Buffer.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace Utopian::Vk
{
	Buffer::Buffer()
	{
	}

	Buffer::Buffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
	{
		Create(device, usageFlags, memoryPropertyFlags, size, data);
	}
	
	Buffer::~Buffer()
	{
		Destroy();
	}

	void Buffer::Create(Device * device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data)
	{
		mDevice = device;

		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.pNext = NULL;
		memAllocInfo.allocationSize = 0;
		memAllocInfo.memoryTypeIndex = 0;

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;
		bufferCreateInfo.flags = 0;

		VkDevice vkDevice = mDevice->GetVkDevice();
		VulkanDebug::ErrorCheck(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &mBuffer));

		vkGetBufferMemoryRequirements(vkDevice, mBuffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		mDevice->GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &memAllocInfo.memoryTypeIndex);
		VulkanDebug::ErrorCheck(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &mMemory));

		if (data != nullptr)
		{
			void *mapped;
			VulkanDebug::ErrorCheck(vkMapMemory(vkDevice, mMemory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			vkUnmapMemory(vkDevice, mMemory);
		}
		VulkanDebug::ErrorCheck(vkBindBufferMemory(vkDevice, mBuffer, mMemory, 0));
	}

	void Buffer::Destroy()
	{
		if (mBuffer != VK_NULL_HANDLE)
			vkDestroyBuffer(mDevice->GetVkDevice(), mBuffer, nullptr);

		if (mMemory != VK_NULL_HANDLE)
		vkFreeMemory(mDevice->GetVkDevice(), mMemory, nullptr);
	}

	void Buffer::MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data)
	{
		VulkanDebug::ErrorCheck(vkMapMemory(mDevice->GetVkDevice(), mMemory, offset, size, flags, data));
		mMapped = true;
	}

	void Buffer::UnmapMemory()
	{
		if (mMapped)
			vkUnmapMemory(mDevice->GetVkDevice(), mMemory);

		mMapped = false;
	}

	VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = mMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(mDevice->GetVkDevice(), 1, &mappedRange);
	}

	VkBuffer Buffer::GetVkBuffer()
	{
		return mBuffer;
	}

	VkDeviceMemory Buffer::GetVkMemory()
	{
		return mMemory;
	}
}