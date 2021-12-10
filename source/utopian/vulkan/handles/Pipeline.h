#pragma once

#include "Handle.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include <vector>

namespace Utopian::Vk
{
   enum BlendingType
   {
      BLENDING_NONE,
      BLENDING_ADDITIVE,
      BLENDING_ALPHA
   };

   struct PipelineDesc
   {
      PipelineDesc();

      /**
       * Adds a tesselation state to the pipeline description.
       */
      void AddTessellationState(uint32_t patchSize);

      /**
       * Overrides the vertex description parsed from the shader.
       * The vertex layout parsed from the shader only works if there
       * only is one binding. For example when using an instance buffer we
       * need two bindings (one for vertices, one for instances) so this needs to be used.
       */
      void OverrideVertexInput(SharedPtr<VertexDescription> vertexDescription);

      VkPipelineRasterizationStateCreateInfo rasterizationState = {};
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
      VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
      SharedPtr<VertexDescription> overriddenVertexDescription = nullptr;
      SharedPtr<VkPipelineTessellationStateCreateInfo> tessellationCreateInfo = nullptr;
      BlendingType blendingType = BLENDING_NONE;
   };

   /** Wrapper for VkPipeline. */
   class Pipeline : public Handle<VkPipeline>
   {
   public:
      /**
       * Creates the pipeline.
       * @param pipelineDesc the description of the pipeline to be created.
       * @param renderPass the renderpass related to the pipeline.
       * @note does not create the pipeline object, that is done in Create().
       */
      Pipeline(const PipelineDesc& pipelineDesc, Device* device, RenderPass* renderPass);

      /**
       * Creates the pipeline.
       * @param shader Contains the vertex description and the shader stages.
       * @param pipelineInterface Contains the pipeline layout.
       */
      void Create(Shader* shader, PipelineInterface* pipelineInterface);

      /** Returns true if Create() has been called. */
      bool IsCreated() const;

      /** Returns true if compute pipeline. */
      bool IsComputePipeline() const;

   private:
      void SetAdditiveBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState);
      void SetAlphaBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState);
      void CreateComputePipeline(Shader* shader, PipelineInterface* pipelineInterface);
      void CreateGraphicsPipeline(Shader* shader, PipelineInterface* pipelineInterface);

      RenderPass* mRenderPass;
      PipelineDesc mPipelineDesc;
      bool mComputePipeline;
      bool mCreated;
   };
}
