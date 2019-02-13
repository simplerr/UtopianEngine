#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"
#include <vector>

namespace Utopian::Vk
{
	/** 
	 * Wrapper for VkPipeline. 
	 * Sets up default values for the rasterization, input assembly, depth stencil
	 * and blend states. If other values are wanted the public member variables can
	 * be modified as needed before creating the pipeline.
	 */
	class Pipeline : public Handle<VkPipeline>
	{
	public:
		Pipeline(Device* device, RenderPass* renderPass);

		/**
		 * Creates the pipeline.
		 * @param shader Contains the vertex description and the shader stages.
		 * @param pipelineInterface Contains the pipeline layout.
		 */
		void Create(Shader* shader, PipelineInterface* pipelineInterface);

		/** 
		 * Overrides the vertex description parsed from the shader.
		 * The vertex layout parsed from the shader only works if there
		 * only is one binding. For example when using an instance buffer we
		 * need two bindings (one for vertices, one for instances) so this needs to be used.
		 */
		void OverrideVertexInput(SharedPtr<VertexDescription> vertexDescription);

		/** Returns true if Create() has been called. */
		bool IsCreated() const;

		void AddTessellationState(uint32_t patchSize);

		/** Public so they can be modified before calling Create(). */
		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentState;
	private:
		void InitDefaultValues(RenderPass* renderPass);

		RenderPass* mRenderPass;	
		SharedPtr<VertexDescription> mOverridenVertexDescription;
		SharedPtr<VkPipelineTessellationStateCreateInfo> mTessellationCreateInfo;
		bool mCreated;
	};
}
