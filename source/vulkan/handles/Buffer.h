#pragma once

#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class Buffer
	{
	public:
		Buffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data);
		~Buffer();

		void MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data);
		void UnmapMemory();

		VkBuffer GetVkBuffer();
		VkDeviceMemory GetVkMemory();
	private:
		Device* mDevice;

		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
	};
}
