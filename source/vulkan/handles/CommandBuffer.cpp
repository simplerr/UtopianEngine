#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "vulkan/handles/PipelineLegacy.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/EffectLegacy.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "RenderPass.h"

namespace Utopian::Vk
{
	CommandBuffer::CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin)
		: Handle(device, nullptr)
	{
		mCommandPool = commandPool;
		mActive = true;

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool->GetVkHandle();
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(GetVkDevice(), &allocateInfo, &mHandle));

		// If requested, also start the new command buffer
		if (begin)
		{
			Begin();
		}
	}

	CommandBuffer::~CommandBuffer()
	{
		// [NOTE] Maybe not needed, if the command pool frees all it's command buffers
		vkFreeCommandBuffers(GetVkDevice(), mCommandPool->GetVkHandle(), 1, &mHandle);
	}

	void CommandBuffer::Create(CommandPool* commandPool, VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool->GetVkHandle();
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(GetVkDevice(), &allocateInfo, &mHandle));

		// If requested, also start the new command buffer
		if (begin)
		{
			Begin();
		}
	}

	void CommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mHandle, &beginInfo));
	}

	void CommandBuffer::Begin(RenderPass* renderPass, VkFramebuffer frameBuffer)
	{
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = renderPass->GetVkHandle();
		inheritanceInfo.framebuffer = frameBuffer;

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mHandle, &beginInfo));
	}

	void CommandBuffer::End()
	{
		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mHandle));
	}

	void CommandBuffer::Flush(VkQueue queue, CommandPool* commandPool, bool free)
	{
		if (mHandle == VK_NULL_HANDLE)
		{
			return;
		}

		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mHandle));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mHandle;

		VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(queue));	// [TODO] Wait for fence instead?

		if (free)
		{
			Cleanup(commandPool);
		}
	}

	void CommandBuffer::Cleanup(CommandPool* commandPool)
	{
		vkFreeCommandBuffers(GetVkDevice(), commandPool->GetVkHandle(), 1, &mHandle);
	}

	void CommandBuffer::CmdBeginRenderPass(VkRenderPassBeginInfo* renderPassBeginInfo, VkSubpassContents subpassContents)
	{
		vkCmdBeginRenderPass(mHandle, renderPassBeginInfo, subpassContents);
	}

	void CommandBuffer::CmdEndRenderPass()
	{
		vkCmdEndRenderPass(mHandle);
	}

	void CommandBuffer::CmdSetViewPort(float width, float height)
	{
		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(mHandle, 0, 1, &viewport);
	}

	void CommandBuffer::CmdSetScissor(uint32_t width, uint32_t height)
	{
		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(mHandle, 0, 1, &scissor);		
	}

	void CommandBuffer::CmdSetScissor(const VkRect2D& rect)
	{
		vkCmdSetScissor(mHandle, 0, 1, &rect);
	}

	void CommandBuffer::CmdBindPipeline(PipelineLegacy* pipeline)
	{
		vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkHandle());
	}

	void CommandBuffer::CmdBindPipeline(Pipeline2* pipeline)
	{
		vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkHandle());
	}

	void CommandBuffer::CmdBindPipeline(ComputePipeline* pipeline)
	{
		vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetVkHandle());
	}

	void CommandBuffer::CmdBindPipeline(VkPipeline pipeline)
	{
		vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::CmdBindPipeline(Pipeline* pipeline)
	{
		vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkHandle());
	}

	void CommandBuffer::CmdBindDescriptorSet(PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet)
	{
		vkCmdBindDescriptorSets(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSet->descriptorSet, 0, NULL);
	}

	void CommandBuffer::CmdBindDescriptorSet(EffectLegacy* effect, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet)
	{
		vkCmdBindDescriptorSets(mHandle, bindPoint, effect->GetPipelineLayout(), firstSet, descriptorSetCount, descriptorSets, 0, NULL);
	}

	void CommandBuffer::CmdBindDescriptorSet(VkPipelineLayout pipelineLayout, uint32_t descriptorSetCount, VkDescriptorSet * descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet)
	{
		vkCmdBindDescriptorSets(mHandle, bindPoint, pipelineLayout, firstSet, descriptorSetCount, descriptorSets, 0, NULL);
	}

	void CommandBuffer::CmdPushConstants(PipelineLayout* pipelineLayout, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data)
	{
		vkCmdPushConstants(mHandle, pipelineLayout->GetVkHandle(), shaderStageFlags, 0, size, data);
	}

	void CommandBuffer::CmdPushConstants(EffectLegacy* effect, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data)
	{
		vkCmdPushConstants(mHandle, effect->GetPipelineLayout(), shaderStageFlags, 0, size, data);
	}

	void CommandBuffer::CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers)
	{
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(mHandle, firstBinding, bindingCount, buffers, offsets);		
	}

	void CommandBuffer::CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, Buffer* buffer)
	{
		VkDeviceSize offsets[1] = { 0 };
		VkBuffer vkBuffer = buffer->GetVkBuffer();
		vkCmdBindVertexBuffers(mHandle, firstBinding, bindingCount, &vkBuffer, offsets);		
	}

	void CommandBuffer::CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(mHandle, buffer, offset, indexType);
	}

	void CommandBuffer::CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(mHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(mHandle, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::CmdDispatch(uint32_t x, uint32_t y, uint32_t z)
	{
		vkCmdDispatch(mHandle, x, y, z);
	}

	bool CommandBuffer::IsActive()
	{
		return mActive;
	}

	void CommandBuffer::ToggleActive()
	{
		mActive = !mActive;
	}
}
