#include "UniformBuffer.h"
#include "VulkanBase.h"

namespace VulkanLib
{
	void UniformBuffer::CreateBuffer(VulkanBase* vulkanBase, VkMemoryPropertyFlagBits propertyFlags)
	{
		vulkanBase->CreateBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			propertyFlags,
			GetSize(),	// Virtual function
			nullptr,
			&mBuffer,
			&mMemory);

		// mBuffer will not be used by itself, it's the VkWriteDescriptorSet.pBufferInfo that points to our uniformBuffer.descriptor
		// so here we need to point uniformBuffer.descriptor.buffer to uniformBuffer.buffer
		mDescriptor.buffer = mBuffer;
		mDescriptor.range = GetSize();
		mDescriptor.offset = 0;
	}
}