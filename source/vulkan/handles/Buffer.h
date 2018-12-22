#pragma once

#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Buffer
	{
	public:
		Buffer();
		Buffer(Device* device,
		 	   VkBufferUsageFlags usageFlags,
			   VkMemoryPropertyFlags memoryPropertyFlags,
			   VkDeviceSize size,
			   void* data);
		~Buffer();

		void Create(Device* device,
					VkBufferUsageFlags usageFlags,
					VkMemoryPropertyFlags memoryPropertyFlags,
					VkDeviceSize size,
					void* data = nullptr);

		void Destroy();

		void MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data);
		void UnmapMemory();
		void UpdateMemory(void* data, VkDeviceSize size);

		VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		VkBuffer GetVkBuffer();
		VkDeviceMemory GetVkMemory();
	private:
		Device* mDevice;

		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		bool mMapped = false;
	};
}
