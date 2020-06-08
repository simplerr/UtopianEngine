#pragma once

#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "Handle.h"

namespace Utopian::Vk
{
	/** Wrapper for VkCommandBuffer. See Vulkan specification for documentation. */
	class CommandBuffer : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer(Device* device, VkCommandBufferLevel level, bool begin = false);
		~CommandBuffer();

		/** Should be used for secondary command buffers. */
		void Begin(RenderPass* renderPass, VkFramebuffer frameBuffer);

		void Begin();
		void End();

		/** Uses the Queue from the Device to submit recorded commands. */
		void Flush(bool free = false);
		void Cleanup();
		void Submit(const SharedPtr<Semaphore>& waitSemaphore, const SharedPtr<Semaphore>& signalSemaphore);

		void CmdBeginRenderPass(VkRenderPassBeginInfo* renderPassBeginInfo, VkSubpassContents subpassContents);
		void CmdEndRenderPass();
		void CmdSetViewPort(float width, float height);
		void CmdSetScissor(uint32_t width, uint32_t height);
		void CmdSetScissor(const VkRect2D& rect);
		void CmdBindPipeline(ComputePipeline* pipeline);
		void CmdBindPipeline(VkPipeline pipeline);
		void CmdBindPipeline(const Pipeline* pipeline);
		void CmdBindDescriptorSet(const PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet);
		void CmdBindDescriptorSet(VkPipelineLayout pipelineLayout, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet = 0);
		void CmdBindDescriptorSet(const PipelineInterface* pipelineInterface, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet = 0);
		void CmdBindDescriptorSets(const SharedPtr<Effect>& effect, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
		void CmdPushConstants(const PipelineLayout* pipelineLayout, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdPushConstants(const PipelineInterface* pipelineInterface, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, Buffer* buffer);
		void CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void CmdBindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType);
		void CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
		void CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void CmdDispatch(uint32_t x, uint32_t y, uint32_t z);

		bool IsActive();
		void ToggleActive();
		void SetActive(bool active);
	
	private:
		bool mActive;
	};
}
