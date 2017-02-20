#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;
	class CommandPool;
	class RenderPass;
	class VulkanApp;
	class Pipeline;
	class PipelineLayout;
	class DescriptorSet;

	class CommandBuffer : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer(VkDevice device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		~CommandBuffer();

		void Create(CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
		void Begin();
		void Begin(RenderPass* renderPass, VkFramebuffer frameBuffer);
		void End();
		void Flush(VkQueue queue, CommandPool* commandPool, bool free = false);
		void Cleanup(CommandPool* commandPool);

		void CmdSetViewPort(float width, float height);
		void CmdSetScissor(uint32_t width, uint32_t height);
		void CmdBindPipeline(Pipeline* pipeline);
		void CmdBindDescriptorSet(PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet);
		void CmdPushConstants(PipelineLayout* pipelineLayout, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers);
		void CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
	
	private:
		CommandPool* mCommandPool; // The command pool this command buffer comes from 
	};
}
