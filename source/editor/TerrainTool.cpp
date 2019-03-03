#include "editor/TerrainTool.h"
#include "vulkan/EffectManager.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/Effect.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/UIOverlay.h"
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

		// Temporary:
		brushSettings.mode = 0;
		brushSettings.operation = 0;
		brushSettings.strength = 240.0f;
		brushSettings.radius = 500.0f;
   }

   TerrainTool::~TerrainTool()
   {

   }

   void TerrainTool::Update()
   {
		RenderUi();
		glm::vec3 cameraPos = gRenderer().GetMainCamera()->GetPosition();
		static glm::vec3 intersection = glm::vec3(0.0);

		Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
		intersection = mTerrain->GetIntersectPoint(ray);
		brushSettings.position = mTerrain->TransformToUv(intersection.x, intersection.z);
		brushSettings.radius += gInput().MouseDz();
		
		// Terrain needs to know about the brush settings to render the debug circle
		mTerrain->SetBrushSettings(brushSettings);

		if (brushSettings.mode == 1) // Blend
		{
			if (gInput().KeyDown(VK_LBUTTON))
			{
				RenderBlendmapBrush();
			}
		}
		else if (brushSettings.mode == 0) // Height
		{
			if (gInput().KeyDown(VK_LBUTTON) || gInput().KeyDown(VK_RBUTTON))
			{
				brushSettings.operation = gInput().KeyDown(VK_LBUTTON) ? 0 : 1;
				RenderHeightmapBrush();
				mTerrain->RenderNormalmap();
				mTerrain->RenderBlendmap();
				mTerrain->RetrieveHeightmap();
			}
		}

		float height = mTerrain->GetHeight(cameraPos.x, cameraPos.z);
   }

   void TerrainTool::RenderUi()
   {
	   // Display Actor creation list
	   Vk::UIOverlay::BeginWindow("Terrain tool", glm::vec2(1500.0f, 850.0f), 200.0f);

	   ImGui::Combo("Brush mode", &brushSettings.mode, "Height\0Blend\0");
	   ImGui::SliderFloat("Brush radius", &brushSettings.radius, 0.0f, 10000.0f);
	   ImGui::SliderFloat("Brush strenth", &brushSettings.strength, 0.0f, 299.0f);

	   Vk::UIOverlay::EndWindow();
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
		brushBlock.data.brushPos = brushSettings.position;
		brushBlock.data.radius = brushSettings.radius / mTerrain->GetTerrainSize();
		brushBlock.data.strength = brushSettings.strength;
		brushBlock.data.mode = brushSettings.mode;
		brushBlock.data.operation = brushSettings.operation;
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
		brushBlock.data.brushPos = brushSettings.position;
		brushBlock.data.radius = brushSettings.radius / mTerrain->GetTerrainSize();
		brushBlock.data.strength = brushSettings.strength;
		brushBlock.data.mode = brushSettings.mode;
		brushBlock.data.operation = brushSettings.operation;
		brushBlock.UpdateMemory();

		heightmapBrushRenderTarget->Begin("Heightmap brush pass");
		Vk::CommandBuffer* commandBuffer = heightmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mHeightmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapBrushRenderTarget->End();
	}
}