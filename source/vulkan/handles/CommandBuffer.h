#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace Vulkan
{
	class Device;
	class CommandPool;
	class RenderPass;
	class Renderer;
	class Pipeline;
	class Pipeline2;
	class ComputePipeline;
	class PipelineLayout;
	class DescriptorSet;
	class Buffer;

	class CommandBuffer : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBufferLevel level, bool begin = false);
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
		void CmdBindPipeline(Pipeline2* pipeline);
		void CmdBindPipeline(ComputePipeline* pipeline);
		void CmdBindDescriptorSet(PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet);
		void CmdPushConstants(PipelineLayout* pipelineLayout, VkShaderStageFlags shaderStageFlags, uint32_t size, const void* data);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers);
		void CmdBindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, Buffer* buffer);
		void CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

		bool IsActive();
		void ToggleActive();
	
	private:
		CommandPool* mCommandPool; // The command pool this command buffer comes from 
		bool mActive;
	};
}
