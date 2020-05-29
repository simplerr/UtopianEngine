#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
	Buffer::Buffer(Device* device)
		: Handle(device, nullptr)
	{
	}

	Buffer::Buffer(const BUFFER_CREATE_INFO& createInfo, Device* device)
		: Handle(device, nullptr)
	{
		Create(device, createInfo.usageFlags, createInfo.memoryPropertyFlags, createInfo.size, createInfo.data);
	}

	Buffer::Buffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
		: Handle(device, nullptr)
	{
		Create(device, usageFlags, memoryPropertyFlags, size, data);
	}
	
	Buffer::~Buffer()
	{
		Destroy();
	}

	void Buffer::Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
	{
		SetDebugName("Unnamed Buffer");
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
		Debug::ErrorCheck(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &mHandle));

		mAllocation = device->AllocateMemory(this, memoryPropertyFlags);

		if (data != nullptr)
		{
			UpdateMemory(data, size);
		}
	}

	void Buffer::Destroy()
	{
		if (mHandle != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(mDevice->GetVkDevice(), mHandle, nullptr);
			mDevice->FreeMemory(mAllocation);
		}
	}

	void Buffer::UpdateMemory(void* data, VkDeviceSize size)
	{
		void *mapped;
		MapMemory(&mapped);
		memcpy(mapped, data, size);
		UnmapMemory();
	}

	void Buffer::MapMemory(void** data)
	{
		mDevice->MapMemory(mAllocation, data);
		mMapped = true;
	}

	void Buffer::UnmapMemory()
	{
		if (mMapped)
			mDevice->UnmapMemory(mAllocation);

		mMapped = false;
	}

	void Buffer::Copy(CommandBuffer* commandBuffer, Image* destination, const std::vector<VkBufferImageCopy>& regions)
	{
		vkCmdCopyBufferToImage(
			commandBuffer->GetVkHandle(),
			GetVkHandle(),
			destination->GetVkHandle(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data()
		);
	}
}