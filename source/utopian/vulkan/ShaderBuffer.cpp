#include "ShaderBuffer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Buffer.h"

namespace Utopian::Vk
{
	ShaderBuffer::ShaderBuffer()
	{
		mBuffer = nullptr;
	}

	ShaderBuffer::~ShaderBuffer()
	{
		if (mBuffer != nullptr)
			delete mBuffer;
	}

	void ShaderBuffer::Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags)
	{
		BUFFER_CREATE_INFO createInfo;
		createInfo.usageFlags = usageFlags;
		createInfo.memoryPropertyFlags = propertyFlags;
		createInfo.data = nullptr;
		createInfo.size = GetSize();
		createInfo.name = "Shader buffer: " + GetDebugName();
		mBuffer = new Buffer(createInfo, device);

		// mBuffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
		// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
		mDescriptor.buffer = mBuffer->GetVkHandle();
		mDescriptor.range = GetSize();
		mDescriptor.offset = 0;
	}

	void ShaderBuffer::MapMemory(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data)
	{
		mBuffer->MapMemory(data);
	}

	void ShaderBuffer::UnmapMemory()
	{
		mBuffer->UnmapMemory();
	}
}