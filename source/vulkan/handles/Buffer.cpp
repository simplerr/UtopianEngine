#include "vulkan/handles/Buffer.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace Vulkan
{
	Buffer::Buffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
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
		uint32_t tmp;
		memAllocInfo.memoryTypeIndex = mDevice->GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &tmp); // [NOTE] This is really weird
		memAllocInfo.memoryTypeIndex = tmp;
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
	
	Buffer::~Buffer()
	{
		vkDestroyBuffer(mDevice->GetVkDevice(), mBuffer, nullptr);
		vkFreeMemory(mDevice->GetVkDevice(), mMemory, nullptr);
	}

	void Buffer::MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data)
	{
		VulkanDebug::ErrorCheck(vkMapMemory(mDevice->GetVkDevice(), mMemory, offset, size, flags, data));
	}

	void Buffer::UnmapMemory()
	{
		vkUnmapMemory(mDevice->GetVkDevice(), mMemory);
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