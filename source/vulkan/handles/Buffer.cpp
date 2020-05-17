#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
	Buffer::Buffer()
	{
	}

	Buffer::Buffer(const BUFFER_CREATE_INFO& createInfo, Device* device)
	{
		Create(device, createInfo.usageFlags, createInfo.memoryPropertyFlags, createInfo.size, createInfo.data);
	}

	Buffer::Buffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
	{
		Create(device, usageFlags, memoryPropertyFlags, size, data);
	}
	
	Buffer::~Buffer()
	{
		Destroy();
	}

	void Buffer::Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
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
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkDevice vkDevice = mDevice->GetVkDevice();
		Debug::ErrorCheck(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &mBuffer));

		vkGetBufferMemoryRequirements(vkDevice, mBuffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		mDevice->GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &memAllocInfo.memoryTypeIndex);
		Debug::ErrorCheck(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &mMemory));

		if (data != nullptr)
		{
			void *mapped;
			Debug::ErrorCheck(vkMapMemory(vkDevice, mMemory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			vkUnmapMemory(vkDevice, mMemory);
		}

		Debug::ErrorCheck(vkBindBufferMemory(vkDevice, mBuffer, mMemory, 0));
	}

	void Buffer::Destroy()
	{
		if (mBuffer != VK_NULL_HANDLE)
			vkDestroyBuffer(mDevice->GetVkDevice(), mBuffer, nullptr);

		if (mMemory != VK_NULL_HANDLE)
			vkFreeMemory(mDevice->GetVkDevice(), mMemory, nullptr);
	}

	void Buffer::UpdateMemory(void* data, VkDeviceSize size)
	{
		void *mapped;
		MapMemory(0, size, 0, &mapped);
		memcpy(mapped, data, size);
		UnmapMemory();
	}

	void Buffer::MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data)
	{
		Debug::ErrorCheck(vkMapMemory(mDevice->GetVkDevice(), mMemory, offset, size, flags, data));
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

	void Buffer::Copy(CommandBuffer* commandBuffer, Image* destination, const std::vector<VkBufferImageCopy>& regions)
	{
		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			commandBuffer->GetVkHandle(),
			GetVkBuffer(),
			destination->GetVkHandle(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data()
		);
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