#pragma once

#include "vulkan/VulkanInclude.h"
#include "Handle.h"

namespace Utopian::Vk
{
	class CommandBuffer : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		~CommandBuffer();

		void Create(CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		void Begin();
		void Begin(RenderPass* renderPass, VkFramebuffer frameBuffer);
		void End();
		void Flush(CommandPool* commandPool = nullptr, bool free = false);
		void Cleanup(CommandPool* commandPool);

		void CmdBeginRenderPass(VkRenderPassBeginInfo* renderPassBeginInfo, VkSubpassContents subpassContents);
		void CmdEndRenderPass();
		void CmdSetViewPort(float width, float height);
		void CmdSetScissor(uint32_t width, uint32_t height);
		void CmdSetScissor(const VkRect2D& rect);
		void CmdBindPipeline(PipelineLegacy* pipeline);
		void CmdBindPipeline(Pipeline2* pipeline);
		void CmdBindPipeline(ComputePipeline* pipeline);
		void CmdBindPipeline(VkPipeline pipeline);
		void CmdBindPipeline(Pipeline* pipeline);
		void CmdBindDescriptorSet(PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet);
		void CmdBindDescriptorSet(EffectLegacy* effect, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet = 0);
		void CmdBindDescriptorSet(VkPipelineLayout pipelineLayout, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet = 0);
		void CmdBindDescriptorSet(PipelineInterface* pipelineInterface, uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets, VkPipelineBindPoint bindPoint, uint32_t firstSet = 0);
		void CmdPushConstants(PipelineLayout* pipelineLayout, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdPushConstants(EffectLegacy* effect, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdPushConstants(PipelineInterface* pipelineInterface, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, Buffer* buffer);
		void CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void CmdBindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType);
		void CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
		void CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void CmdDispatch(uint32_t x, uint32_t y, uint32_t z);

		bool IsActive();
		void ToggleActive();
	
	private:
		CommandPool* mCommandPool; // The command pool this command buffer comes from 
		bool mActive;
	};
}
