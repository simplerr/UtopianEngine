#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/PipelineLegacy.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/Effect.h"
#include "vulkan/EffectLegacy.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "RenderPass.h"

namespace Utopian::Vk
{
	CommandBuffer::CommandBuffer(Device* device, VkCommandBufferLevel level, bool begin)
		: Handle(device, nullptr)
	{
		mActive = true;

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = GetDevice()->GetCommandPool()->GetVkHandle();
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = level;

		Debug::ErrorCheck(vkAllocateCommandBuffers(GetVkDevice(), &allocateInfo, &mHandle));

		// If requested, also start the new command buffer
		if (begin)
		{
			Begin();
		}
	}

	CommandBuffer::~CommandBuffer()
	{
		// [NOTE] Maybe not needed, if the command pool frees all it's command buffers
		vkFreeCommandBuffers(GetVkDevice(), GetDevice()->GetCommandPool()->GetVkHandle(), 1, &mHandle);
	}

	 void CommandBuffer::Begin()
	 {
	 	VkCommandBufferBeginInfo beginInfo = {};
	 	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	 	beginInfo.flags = 0;
	 	beginInfo.pInheritanceInfo = nullptr;

	 	Debug::ErrorCheck(vkBeginCommandBuffer(mHandle, &beginInfo));
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

		Debug::ErrorCheck(vkBeginCommandBuffer(mHandle, &beginInfo));
	}

	void CommandBuffer::End()
	{
		Debug::ErrorCheck(vkEndCommandBuffer(mHandle));
	}

	void CommandBuffer::Flush(bool free)
	{
		assert(mHandle);

		Debug::ErrorCheck(vkEndCommandBuffer(mHandle));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mHandle;

		Queue* queue = GetDevice()->GetQueue();
		queue->Submit(this, nullptr, nullptr, nullptr);
		queue->WaitIdle();

		if (free)
		{
			Cleanup();
		}
	}

	void CommandBuffer::Submit(const SharedPtr<Semaphore>& waitSemaphore, const SharedPtr<Semaphore>& signalSemaphore)
	{
		assert(mHandle);

		Debug::ErrorCheck(vkEndCommandBuffer(mHandle));

		Queue* queue = GetDevice()->GetQueue();
		queue->Submit(this, nullptr, waitSemaphore, signalSemaphore);
	}

	void CommandBuffer::Cleanup()
	{
		vkFreeCommandBuffers(GetVkDevice(), GetDevice()->GetCommandPool()->GetVkHandle(), 1, &mHandle);
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

	void CommandBuffer::CmdBindPipeline(const Pipeline* pipeline)
	{
		if (!pipeline->IsCreated())
			assert(0);

		vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkHandle());
	}

	void CommandBuffer::CmdBindDescriptorSet(const PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet)
	{
		VkDescriptorSet descriptorSetVk = descriptorSet->GetVkHandle();
		vkCmdBindDescriptorSets(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSetVk, 0, NULL);
	}

	void CommandBuffer::CmdBindDescriptorSet(const EffectLegacy* effect, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet)
	{
		vkCmdBindDescriptorSets(mHandle, bindPoint, effect->GetPipelineLayout(), firstSet, descriptorSetCount, descriptorSets, 0, NULL);
	}

	void CommandBuffer::CmdBindDescriptorSet(VkPipelineLayout pipelineLayout, uint32_t descriptorSetCount, VkDescriptorSet * descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet)
	{
		vkCmdBindDescriptorSets(mHandle, bindPoint, pipelineLayout, firstSet, descriptorSetCount, descriptorSets, 0, NULL);
	}
	
	void CommandBuffer::CmdBindDescriptorSets(const SharedPtr<Effect>& effect, uint32_t firstSet, VkPipelineBindPoint bindPoint)
	{
		assert(effect->GetNumDescriptorSets() > 0);

		vkCmdBindDescriptorSets(mHandle,
		 						bindPoint,
								effect->GetPipelineInterface()->GetPipelineLayout(),
								firstSet,
								effect->GetNumDescriptorSets(),
								effect->GetDescriptorSets(),
								0, nullptr);
	}

	void CommandBuffer::CmdBindDescriptorSet(const PipelineInterface* pipelineInterface, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet)
	{
		vkCmdBindDescriptorSets(mHandle, bindPoint, pipelineInterface->GetPipelineLayout(), firstSet, descriptorSetCount, descriptorSets, 0, NULL);
	}

	void CommandBuffer::CmdPushConstants(const PipelineLayout* pipelineLayout, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data)
	{
		vkCmdPushConstants(mHandle, pipelineLayout->GetVkHandle(), shaderStageFlags, 0, size, data);
	}

	void CommandBuffer::CmdPushConstants(const EffectLegacy* effect, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data)
	{
		vkCmdPushConstants(mHandle, effect->GetPipelineLayout(), shaderStageFlags, 0, size, data);
	}

	void CommandBuffer::CmdPushConstants(const PipelineInterface* pipelineInterface, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data)
	{
		vkCmdPushConstants(mHandle, pipelineInterface->GetPipelineLayout(), shaderStageFlags, 0, size, data);
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

	void CommandBuffer::CmdBindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(mHandle, buffer->GetVkBuffer(), offset, indexType);
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
