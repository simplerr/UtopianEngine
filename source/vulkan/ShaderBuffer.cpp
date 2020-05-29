#include "ShaderBuffer.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Buffer.h"

namespace Utopian::Vk
{
	ShaderBuffer::~ShaderBuffer()
	{
		delete mBuffer;
	}

	void ShaderBuffer::Create(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags)
	{
		mBuffer = new Buffer(device, 
							 usageFlags,
							 propertyFlags,
							 GetSize(),	// Virtual function
							 nullptr);

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