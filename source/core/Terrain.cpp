#include "core/Terrain.h"
#include "core/renderer/RendererUtility.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/Effect.h"
#include "vulkan/EffectManager.h"
#include "vulkan/StaticModel.h"
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/Mesh.h"
#include "core/renderer/Renderer.h"
#include "Input.h"
#include "Camera.h"

namespace Utopian
{
	Terrain::Terrain(Vk::Device* device)
	{
		mDevice = device;

		GeneratePatches(128.0, 128);
		GenerateTerrainMaps();

		Vk::gEffectManager().RegisterRecompileCallback(&Terrain::EffectRecomiledCallback, this);
	}

	void Terrain::Update()
	{
		glm::vec3 cameraPos = gRenderer().GetMainCamera()->GetPosition();
		static glm::vec3 intersection = glm::vec3(0.0);

		if (gInput().KeyPressed(VK_LBUTTON))
		{
			Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
			intersection = GetIntersectPoint(ray);
			brushPos = TransformToUv(intersection.x, intersection.z);

			RenderBlendmapBrush();
		}
		if (gInput().KeyPressed(VK_RBUTTON))
		{
			Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
			intersection = GetIntersectPoint(ray);
			brushPos = TransformToUv(intersection.x, intersection.z);

			RenderHeightmapBrush();
			RenderNormalmap();
		}

		float height = GetHeight(cameraPos.x, cameraPos.z);
		//gRenderer().GetMainCamera()->SetPosition(glm::vec3(cameraPos.x, height + 500, cameraPos.z));

		/*Vk::UIOverlay::TextV("Terrain cam pos: %.4f, %.4f", TransformToUv(cameraPos.x, 0).x, TransformToUv(0, cameraPos.z).y);
		Vk::UIOverlay::TextV("Height: %.2f", height);
		Vk::UIOverlay::TextV("Intersection: %.2f, %.2f, %.2f", intersection.x, intersection.y, intersection.z);
		Vk::UIOverlay::TextV("Brush pos: %.4f, %.4f", brushPos.x, brushPos.y);*/
	}

	void Terrain::GenerateTerrainMaps()
	{
		SetupHeightmapEffect();
		SetupNormalmapEffect();
		SetupBlendmapEffect();
		SetupBlendmapBrushEffect();
		SetupHeightmapBrushEffect();

		RenderHeightmap();
		RenderNormalmap();
		RenderBlendmap();
		RenderBlendmapBrush();
		RenderHeightmapBrush();

		// Testing
		RetrieveHeightmap();

		/*const uint32_t size = 440;
		const uint32_t height = 1260;
		gScreenQuadUi().AddQuad(300 + 20, height - (size + 310), size, size, heightmapImage.get(), heightmapRenderTarget->GetSampler());
		gScreenQuadUi().AddQuad(300 + size + 20, height - (size + 310), size, size, normalImage.get(), normalRenderTarget->GetSampler());
		gScreenQuadUi().AddQuad(300 + 2 * size + 20, height - (size + 310), size, size, blendmapImage.get(), blendmapRenderTarget->GetSampler());*/
	}

	void Terrain::EffectRecomiledCallback(std::string name)
	{
		RenderHeightmap();
		RenderNormalmap();
		RenderBlendmap();
		RenderBlendmapBrush();
		RenderHeightmapBrush();

		//RetrieveHeightmap();
	}

	void Terrain::RetrieveHeightmap()
	{
		//gRendererUtility().SaveToFile(mDevice, heightmapImage, "screen.ppm", 256, 256);

		hostImage = gRendererUtility().CreateHostVisibleImage(mDevice, heightmapImage, 1024, 1024, VK_FORMAT_R32G32B32A32_SFLOAT);

		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(mDevice->GetVkDevice(), hostImage->GetVkHandle(), &subResource, &subResourceLayout);

		const char* data;
		vkMapMemory(mDevice->GetVkDevice(), hostImage->GetDeviceMemory(), 0, VK_WHOLE_SIZE, 0, (void**)&data);
		data += subResourceLayout.offset;

		for (uint32_t y = 0; y < hostImage->GetHeight(); y++)
		{
			glm::vec4* row = (glm::vec4*)data;
			for (uint32_t x = 0; x < hostImage->GetHeight(); x++)
			{
				heightmap.push_back((*row).x);
				row++;
			}
			data += subResourceLayout.rowPitch;
		}

		vkUnmapMemory(mDevice->GetVkDevice(), hostImage->GetDeviceMemory());
	}

	void Terrain::SetupHeightmapEffect()
	{
		heightmapImage = std::make_shared<Vk::ImageColor>(mDevice, mapResolution, mapResolution, VK_FORMAT_R32G32B32A32_SFLOAT);
		heightmapImage->SetFinalLayout(VK_IMAGE_LAYOUT_GENERAL);

		heightmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mapResolution, mapResolution);
		heightmapRenderTarget->AddColorAttachment(heightmapImage, VK_IMAGE_LAYOUT_GENERAL);
		heightmapRenderTarget->SetClearColor(1, 1, 1, 1);
		heightmapRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/heightmap.frag";
		mHeightmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mHeightmapEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mHeightmapEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mHeightmapEffect->CreatePipeline();
	}

	void Terrain::SetupNormalmapEffect()
	{
		normalImage = std::make_shared<Vk::ImageColor>(mDevice, mapResolution, mapResolution, VK_FORMAT_R32G32B32A32_SFLOAT);

		normalRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mapResolution, mapResolution);
		normalRenderTarget->AddColorAttachment(normalImage);
		normalRenderTarget->SetClearColor(1, 1, 1, 1);
		normalRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/normalmap.frag";
		mNormalmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, normalRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mNormalmapEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mNormalmapEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mNormalmapEffect->CreatePipeline();

		mNormalmapEffect->BindCombinedImage("samplerHeightmap", heightmapImage.get(), heightmapRenderTarget->GetSampler());
	}

	void Terrain::SetupBlendmapEffect()
	{
		blendmapImage = std::make_shared<Vk::ImageColor>(mDevice, 256, 256, VK_FORMAT_R32G32B32A32_SFLOAT);
		blendmapImage->SetFinalLayout(VK_IMAGE_LAYOUT_GENERAL); // Special case since it needs to be used both as color attachment and descriptor

		blendmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, 256, 256);
		blendmapRenderTarget->AddColorAttachment(blendmapImage, VK_IMAGE_LAYOUT_GENERAL);
		blendmapRenderTarget->SetClearColor(1, 1, 1, 1);
		blendmapRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/blendmap.frag";
		mBlendmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, blendmapRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mBlendmapEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mBlendmapEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mBlendmapEffect->CreatePipeline();

		settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mBlendmapEffect->BindUniformBuffer("UBO_settings", &settingsBlock);

		mBlendmapEffect->BindCombinedImage("samplerHeightmap", heightmapImage.get(), heightmapRenderTarget->GetSampler());
		mBlendmapEffect->BindCombinedImage("samplerNormalmap", normalImage.get(), heightmapRenderTarget->GetSampler());
		mBlendmapEffect->BindUniformBuffer("UBO_settings", &settingsBlock);
	}

	void Terrain::SetupBlendmapBrushEffect()
	{
		blendmapBrushRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, 256, 256);
		blendmapBrushRenderTarget->AddColorAttachment(blendmapImage, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL);
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

	void Terrain::SetupHeightmapBrushEffect()
	{
		heightmapBrushRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mapResolution, mapResolution);
		heightmapBrushRenderTarget->AddColorAttachment(heightmapImage, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL);
		heightmapBrushRenderTarget->SetClearColor(1, 1, 1, 1);
		heightmapBrushRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/heightmap_brush.frag";
		mHeightmapBrushEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapBrushRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mHeightmapBrushEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mHeightmapBrushEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Enable additive blending
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].blendEnable = VK_TRUE;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		mHeightmapBrushEffect->GetPipeline()->blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;

		mHeightmapBrushEffect->CreatePipeline();

		mHeightmapBrushEffect->BindUniformBuffer("UBO_brush", &brushBlock);
	}

	void Terrain::RenderHeightmap()
	{
		heightmapRenderTarget->Begin("Heightmap pass");
		Vk::CommandBuffer* commandBuffer = heightmapRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapEffect->GetPipeline());
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapRenderTarget->End();
	}

	void Terrain::RenderNormalmap()
	{
		normalRenderTarget->Begin("Normalmap pass");
		Vk::CommandBuffer* commandBuffer = normalRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mNormalmapEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mNormalmapEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		normalRenderTarget->End();
	}

	void Terrain::RenderBlendmap()
	{
		settingsBlock.data.amplitudeScaling = mAmplitudeScaling;
		settingsBlock.UpdateMemory();

		blendmapRenderTarget->Begin("Blendmap pass");
		Vk::CommandBuffer* commandBuffer = blendmapRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mBlendmapEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mBlendmapEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		blendmapRenderTarget->End();
	}

	void Terrain::RenderBlendmapBrush()
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

	void Terrain::RenderHeightmapBrush()
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

	void Terrain::GeneratePatches(float cellSize, int numCells)
	{
		terrainSize = cellSize * (numCells - 1);

		mQuadModel = new Vk::StaticModel();
		Vk::Mesh* mesh = new Vk::Mesh(mDevice);

		// Vertices
		for (auto x = 0; x < numCells; x++)
		{
			for (auto z = 0; z < numCells; z++)
			{
				Vk::Vertex vertex;
				const float originOffset = (float)numCells * cellSize / 2.0f;
				vertex.Pos = glm::vec3(x * cellSize + cellSize / 2.0f - originOffset, 0.0f, z * cellSize + cellSize / 2.0f - originOffset);
				vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				vertex.Tex = glm::vec2((float)x / (numCells - 1), (float)z / (numCells - 1));
				mesh->AddVertex(vertex);
			}
		}

		// Indices
		const uint32_t w = (numCells - 1);
		for (auto x = 0; x < w; x++)
		{
			for (auto z = 0; z < w; z++)
			{
				uint32_t v1 = (x + z * numCells);
				uint32_t v2 = v1 + numCells;
				uint32_t v3 = v2 + 1;
				uint32_t v4 = v1 + 1;
				mesh->AddQuad(v1, v2, v3, v4);
			}
		}

		mesh->LoadTextures("data/textures/ground/grass2.tga");
		mesh->BuildBuffers(mDevice);
		mQuadModel->AddMesh(mesh);
	}

	glm::vec2 Terrain::TransformToUv(float x, float z)
	{
		// Transform to terrain UV coordinates [0, 1]
		glm::vec2 uv = glm::vec2(x, z);
		uv += terrainSize / 2.0f;
		uv /= terrainSize;

		// To get correct UV coordinates
		uv = glm::vec2(1.0, 1.0) - uv;
		return uv;
	}

	float Terrain::GetHeight(float x, float z)
	{
		float height = -1.0f;

		glm::vec2 uv = TransformToUv(x, z);

		uint32_t col = floorf(uv.x * hostImage->GetWidth());
		uint32_t row = floorf(uv.y * hostImage->GetHeight());

		if (row >= 0 && col >= 0 && row < hostImage->GetWidth() && col < hostImage->GetHeight())
		{
			height = heightmap[row * hostImage->GetWidth() + col] * mAmplitudeScaling * -1; // Todo: Fix amplitude and -1
		}

		return height;
	}

	glm::vec3 Terrain::GetIntersectPoint(Ray ray)
	{
		Ray shorterRay = LinearSearch(ray);

		// This is good enough accuracy for now
		return shorterRay.origin;
		//return BinarySearch(shorterRay);
	}

	Ray Terrain::LinearSearch(Ray ray)
	{
		float stepSize = 10.0f;
		glm::vec3 nextPoint = ray.origin + ray.direction * stepSize;
		float heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);
		int counter = 0;
		while (heightAtNextPoint < nextPoint.y && counter < 1000)
		{
			counter++;
			ray.origin = nextPoint;
			nextPoint = ray.origin + ray.direction * stepSize;
			heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);
		}

		// Return infinity if the ray dont strike anything
		if (counter >= 1000)
			ray.direction.x = std::numeric_limits<float>::infinity();

		return ray;
	}

	glm::vec3 Terrain::GetNormal(float x, float z)
	{
		return glm::vec3(0, 1, 0);
	}

	Vk::Image* Terrain::GetHeightmapImage()
	{
		return heightmapImage.get();
	}

	Vk::Image* Terrain::GetNormalmapImage()
	{
		return normalImage.get();
	}

	Vk::Image* Terrain::GetBlendmapImage()
	{
		return blendmapImage.get();
	}

    Vk::Mesh* Terrain::GetMesh()
    {
        return mQuadModel->mMeshes[0];
    }
}