#include "UniformBuffer.h"
#include "VulkanBase.h"
#include "vulkan/handles/Buffer.h"

namespace Vulkan
{
	UniformBuffer::~UniformBuffer()
	{
		delete mBuffer;
	}

	void UniformBuffer::CreateBuffer(VulkanBase* vulkanBase, VkMemoryPropertyFlagBits propertyFlags)
	{
		mBuffer = new Buffer(vulkanBase->GetDevice(), 
							 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
							 propertyFlags,
							 GetSize(),	// Virtual function
							 nullptr);

		// mBuffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
		// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
		mDescriptor.buffer = mBuffer->GetVkBuffer();
		mDescriptor.range = GetSize();
		mDescriptor.offset = 0;
	}
}