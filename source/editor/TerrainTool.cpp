#include "editor/TerrainTool.h"
#include "vulkan/EffectManager.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/Effect.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/RendererUtility.h"
#include "core/Terrain.h"
#include "Input.h"
#include "Camera.h"

namespace Utopian
{
   TerrainTool::TerrainTool(const SharedPtr<Terrain>& terrain, Vk::Device* device)
   {
		mTerrain = terrain;
		mDevice = device;

		SetupBlendmapBrushEffect();
		SetupHeightmapBrushEffect();

		RenderBlendmapBrush();
		RenderHeightmapBrush();

		Vk::gEffectManager().RegisterRecompileCallback(&TerrainTool::EffectRecompiledCallback, this);
   }

   TerrainTool::~TerrainTool()
   {

   }

   void TerrainTool::Update()
   {
      glm::vec3 cameraPos = gRenderer().GetMainCamera()->GetPosition();
		static glm::vec3 intersection = glm::vec3(0.0);

		if (gInput().KeyPressed(VK_LBUTTON))
		{
			Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
			intersection = mTerrain->GetIntersectPoint(ray);
			brushPos = mTerrain->TransformToUv(intersection.x, intersection.z);

			RenderBlendmapBrush();
		}

		if (gInput().KeyPressed(VK_RBUTTON))
		{
			Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
			intersection = mTerrain->GetIntersectPoint(ray);
			brushPos = mTerrain->TransformToUv(intersection.x, intersection.z);

			RenderHeightmapBrush();
			mTerrain->RenderNormalmap();
		}

		float height = mTerrain->GetHeight(cameraPos.x, cameraPos.z);
   }

	void TerrainTool::EffectRecompiledCallback(std::string name)
	{
		RenderBlendmapBrush();
		RenderHeightmapBrush();

		//mTerrain->RetrieveHeightmap();
	}

   void TerrainTool::SetupBlendmapBrushEffect()
	{
		blendmapBrushRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, 256, 256);
		blendmapBrushRenderTarget->AddColorAttachment(mTerrain->GetBlendmapImage(), VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL);
		blendmapBrushRenderTarget->SetClearColor(1, 1, 1, 1);
		blendmapBrushRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/blendmap_brush.frag";
		mBlendmapBrushEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, blendmapBrushRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mBlendmapBrushEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mBlendmapBrushEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mBlendmapBrushEffect->CreatePipeline();

		brushBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mBlendmapBrushEffect->BindUniformBuffer("UBO_brush", &brushBlock);
	}

	void TerrainTool::SetupHeightmapBrushEffect()
	{
		heightmapBrushRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mTerrain->GetMapResolution(), mTerrain->GetMapResolution());
		heightmapBrushRenderTarget->AddColorAttachment(mTerrain->GetHeightmapImage(), VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL);
		heightmapBrushRenderTarget->SetClearColor(1, 1, 1, 1);
		heightmapBrushRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/heightmap_brush.frag";
		mHeightmapBrushEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapBrushRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mHeightmapBrushEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mHeightmapBrushEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		gRendererUtility().SetAdditiveBlending(mHeightmapBrushEffect->GetPipeline());

		mHeightmapBrushEffect->CreatePipeline();

		mHeightmapBrushEffect->BindUniformBuffer("UBO_brush", &brushBlock);
	}

   void TerrainTool::RenderBlendmapBrush()
	{
		brushBlock.data.brushPos = brushPos;
		brushBlock.UpdateMemory();

		blendmapBrushRenderTarget->Begin("Blendmap brush pass");
		Vk::CommandBuffer* commandBuffer = blendmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mBlendmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mBlendmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		blendmapBrushRenderTarget->End();
	}

	void TerrainTool::RenderHeightmapBrush()
	{
		brushBlock.data.brushPos = brushPos;
		brushBlock.UpdateMemory();

		heightmapBrushRenderTarget->Begin("Heightmap brush pass");
		Vk::CommandBuffer* commandBuffer = heightmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mHeightmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapBrushRenderTarget->End();
	}
}