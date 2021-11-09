#pragma once
#include "core/renderer/jobs/BaseJob.h"
#include "vulkan/VulkanPrerequisites.h"
#include "imgui/imgui.h"

namespace Utopian
{
   /**
    * Each render pass is defined as a Job that can have multiple inputs and outputs.
    * The inputs and outputs are typically render targets but can be any kind of data.
    * JobGraph handles the order and execution of these jobs.
    *
    * For example the SkydomeJob has the Depth buffer from the G-Buffer job as an input
    * to not render the skydome infront of all objects.
    */
   class JobGraph
   {
   public:
      struct GBufferDebugDescriptorSets
      {
         ImTextureID position;
         ImTextureID normal;
         ImTextureID normalView;
         ImTextureID albedo;
         ImTextureID pbr;
      };

      enum JobIndex
      {
         GBUFFER_TERRAIN_INDEX = 0,
         GBUFFER_INDEX,
         SSAO_INDEX,
         BLUR_INDEX,
         SHADOW_INDEX,
         DEFERRED_INDEX,
         //GRASS_INDEX,
         //SKYBOX_INDEX,
         SKYDOME_INDEX,
         SUN_SHAFT_INDEX,
         OPAQUE_COPY_INDEX,
         GEOMETRY_THICKNESS_INDEX,
         WATER_INDEX,
         SSR_INDEX,
         FRESNEL_INDEX,
         DEBUG_INDEX,
         IM3D_INDEX,
         BLOOM_INDEX,
         DOF_INDEX,
         OUTLINE_INDEX,
         TONEMAP_INDEX,
         //PIXEL_DEBUG_INDEX,
         FXAA_INDEX
      };

      enum DebugChannel {NONE, POSITION, NORMAL, NORMAL_VIEW, ALBEDO, PBR};

      JobGraph(Vk::VulkanApp* vulkanApp, Terrain* terrain, Vk::Device* device, const RenderingSettings& renderingSettings);
      ~JobGraph();

      void AsynchronousResourceLoading();

      /** Renders all jobs added to the graph. */
      void Render(const SceneInfo& sceneInfo, const RenderingSettings& renderingSettings);
      void Update();
      void EnableJob(JobIndex jobIndex, bool enabled);

      void SetDebugChannel(DebugChannel debugChannel);

      const GBuffer& GetGBuffer() const;

   private:
      /** Adds a job to the graph. */
      void AddJob(BaseJob* job);
   private:
      std::vector<BaseJob*> mJobs;
      GBuffer mGBuffer;
      
      GBufferDebugDescriptorSets mDebugDescriptorSets;
      DebugChannel mDebugChannel = DebugChannel::NONE;
   };
}
