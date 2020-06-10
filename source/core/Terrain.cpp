#include "core/Terrain.h"
#include "core/renderer/RendererUtility.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/Effect.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/StaticModel.h"
#include "ScreenQuadRenderer.h"
#include "vulkan/Mesh.h"
#include "core/renderer/Renderer.h"
#include "core/physics/Physics.h"
#include "Input.h"
#include "Camera.h"
#include "ImGuiRenderer.h"

namespace Utopian
{
	Terrain::Terrain(Vk::Device* device)
	{
		mDevice = device;

		GeneratePatches(128.0, 512);
		GenerateTerrainMaps();

		Vk::gEffectManager().RegisterRecompileCallback(&Terrain::EffectRecomiledCallback, this);

		// This is used by both TerrainTool and GBufferTerrainJob
		// TerrainTool is responsible for updating it
		mBrushBlock = std::make_shared<Terrain::BrushBlock>();
		mBrushBlock->Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		// Load materials
		AddMaterial("grass", "data/textures/ground/grass_diffuse.ktx", "data/textures/ground/grass_normal.ktx", "data/textures/ground/grass_displacement.ktx");
		AddMaterial("rock", "data/textures/ground/rock_diffuse.ktx", "data/textures/ground/rock_normal.ktx", "data/textures/ground/rock_displacement.ktx");
		AddMaterial("dirt", "data/textures/ground/dirt_diffuse.ktx", "data/textures/ground/dirt_normal.ktx", "data/textures/ground/dirt_displacement.ktx");

		// Add heightmap to physics world
		gPhysics().SetHeightmap(heightmap.data(), MAP_RESOLUTION, mAmplitudeScaling, terrainSize);
	}

	void Terrain::Update()
	{
		// Experimentation
		if (gInput().KeyPressed('U'))
			gPhysics().SetHeightmap(heightmap.data(), MAP_RESOLUTION, mAmplitudeScaling, terrainSize);

		//gRenderer().GetMainCamera()->SetPosition(glm::vec3(cameraPos.x, height + 500, cameraPos.z));

		/*Vk::UIOverlay::TextV("Terrain cam pos: %.4f, %.4f", TransformToUv(cameraPos.x, 0).x, TransformToUv(0, cameraPos.z).y);
		Vk::UIOverlay::TextV("Height: %.2f", height);
		Vk::UIOverlay::TextV("Intersection: %.2f, %.2f, %.2f", intersection.x, intersection.y, intersection.z);
		Vk::UIOverlay::TextV("Brush pos: %.4f, %.4f", brushPos.x, brushPos.y);*/
		// Display Actor creation list
		ImGuiRenderer::BeginWindow("Terrain debug", glm::vec2(300.0f, 10.0f), 400.0f);

		ImVec2 textureSize = ImVec2(256, 256);
		ImGui::BeginGroup();
		ImGui::Text("Heightmap");
		ImGui::Image(mDebugDescriptorSets.heightmap, textureSize);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Text("Normal map");
		ImGui::Image(mDebugDescriptorSets.normalmap, textureSize);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		ImGui::Text("Blend map");
		ImGui::Image(mDebugDescriptorSets.blendmap, textureSize);
		ImGui::EndGroup();

		ImGuiRenderer::EndWindow();
	}

	void Terrain::AddMaterial(std::string name, std::string diffuse, std::string normal, std::string displacement)
	{
		TerrainMaterial material;
		material.diffuse = Vk::gTextureLoader().LoadTexture(diffuse);
		material.normal = Vk::gTextureLoader().LoadTexture(normal);
		material.displacement = Vk::gTextureLoader().LoadTexture(displacement);
		mMaterials[name] = material;
	}

	void Terrain::GenerateTerrainMaps()
	{
		SetupHeightmapEffect();
		SetupNormalmapEffect();
		SetupBlendmapEffect();

		RenderHeightmap();
		RenderNormalmap();
		RenderBlendmap();

		// Testing
		RetrieveHeightmap();

		// Add debug render targets
		ImGuiRenderer* imGuiRenderer = gRenderer().GetUiOverlay();
		mDebugDescriptorSets.heightmap = imGuiRenderer->AddImage(*heightmapImage);
		mDebugDescriptorSets.normalmap = imGuiRenderer->AddImage(*normalImage);
		mDebugDescriptorSets.blendmap = imGuiRenderer->AddImage(*blendmapImage);
	}

	void Terrain::EffectRecomiledCallback(std::string name)
	{
		RenderHeightmap();
		RenderNormalmap();
		RenderBlendmap();

		RetrieveHeightmap();
	}

	void Terrain::RetrieveHeightmap()
	{
		gRendererUtility().SaveToFile(mDevice, heightmapImage, "screen.ppm", 256, 256);

		hostImage = gRendererUtility().CreateHostVisibleImage(mDevice, heightmapImage, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32_SFLOAT);

		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(mDevice->GetVkDevice(), hostImage->GetVkHandle(), &subResource, &subResourceLayout);

		const char* data;
		hostImage->MapMemory((void**)&data);
		data += subResourceLayout.offset;

		assert(subResourceLayout.rowPitch == MAP_RESOLUTION * sizeof(float));
		assert(subResourceLayout.size == MAP_RESOLUTION * MAP_RESOLUTION * sizeof(float));

		// Since the image tiling is linear we can use memcpy
		memcpy(heightmap.data(), data, subResourceLayout.size);

		hostImage->UnmapMemory();

		// Note: Todo: Hidden dependency to Renderer
		gRenderer().UpdateInstanceAltitudes();
	}

	void Terrain::SetupHeightmapEffect()
	{
		heightmapImage = std::make_shared<Vk::ImageColor>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32G32B32A32_SFLOAT, "Terrain heightmap image");
		heightmapImage->SetFinalLayout(VK_IMAGE_LAYOUT_GENERAL);

		heightmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION);
		heightmapRenderTarget->AddWriteOnlyColorAttachment(heightmapImage, VK_IMAGE_LAYOUT_GENERAL);
		heightmapRenderTarget->SetClearColor(1, 1, 1, 1);
		heightmapRenderTarget->Create();

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/heightmap.frag";
		effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
      effectDesc.pipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mHeightmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapRenderTarget->GetRenderPass(), effectDesc);
	}

	void Terrain::SetupNormalmapEffect()
	{
		normalImage = std::make_shared<Vk::ImageColor>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32G32B32A32_SFLOAT, "Terrain normalmap image");

		normalRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION);
		normalRenderTarget->AddWriteOnlyColorAttachment(normalImage);
		normalRenderTarget->SetClearColor(1, 1, 1, 1);
		normalRenderTarget->Create();

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/normalmap.frag";
		effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
      effectDesc.pipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mNormalmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, normalRenderTarget->GetRenderPass(), effectDesc);

		mNormalmapEffect->BindCombinedImage("samplerHeightmap", *heightmapImage, *heightmapRenderTarget->GetSampler());
	}

	void Terrain::SetupBlendmapEffect()
	{
		blendmapImage = std::make_shared<Vk::ImageColor>(mDevice, 256, 256, VK_FORMAT_R32G32B32A32_SFLOAT, "Terrain blendmap image");
		blendmapImage->SetFinalLayout(VK_IMAGE_LAYOUT_GENERAL); // Special case since it needs to be used both as color attachment and descriptor

		blendmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, 256, 256);
		blendmapRenderTarget->AddWriteOnlyColorAttachment(blendmapImage, VK_IMAGE_LAYOUT_GENERAL);
		blendmapRenderTarget->SetClearColor(1, 1, 1, 1);
		blendmapRenderTarget->Create();

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/blendmap.frag";
		effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
      effectDesc.pipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mBlendmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, blendmapRenderTarget->GetRenderPass(), effectDesc);

		settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mBlendmapEffect->BindUniformBuffer("UBO_settings", settingsBlock);

		mBlendmapEffect->BindCombinedImage("samplerHeightmap", *heightmapImage, *heightmapRenderTarget->GetSampler());
		mBlendmapEffect->BindCombinedImage("samplerNormalmap", *normalImage, *heightmapRenderTarget->GetSampler());
		mBlendmapEffect->BindUniformBuffer("UBO_settings", settingsBlock);
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

	void Terrain::GeneratePatches(float cellSize, int numCells)
	{
		terrainSize = cellSize * (numCells - 1);

		mQuadModel = new Vk::StaticModel();
		Vk::Mesh* mesh = new Vk::Mesh(mDevice);
		mesh->SetDebugName("Terrain patches");

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
		float stepSize = 30.0f;
		glm::vec3 nextPoint = ray.origin + ray.direction * stepSize;
		float heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);
		int counter = 0;
		const int numMaxSteps = 2000;
		while (heightAtNextPoint < nextPoint.y && counter < numMaxSteps)
		{
			counter++;
			ray.origin = nextPoint;
			nextPoint = ray.origin + ray.direction * stepSize;
			heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);
		}

		// Return infinity if the ray dont strike anything
		if (counter >= numMaxSteps)
			ray.direction.x = std::numeric_limits<float>::infinity();

		return ray;
	}

	glm::vec3 Terrain::GetNormal(float x, float z)
	{
		return glm::vec3(0, 1, 0);
	}

	SharedPtr<Vk::Image>& Terrain::GetHeightmapImage()
	{
		return heightmapImage;
	}

	SharedPtr<Vk::Image>& Terrain::GetNormalmapImage()
	{
		return normalImage;
	}

	SharedPtr<Vk::Image>& Terrain::GetBlendmapImage()
	{
		return blendmapImage;
	}

	Vk::Mesh* Terrain::GetMesh()
	{
		return mQuadModel->mMeshes[0];
	}

	uint32_t Terrain::GetMapResolution()
	{
		return MAP_RESOLUTION;
	}

	float Terrain::GetTerrainSize()
	{
		return terrainSize;
	}

	void Terrain::SetBrushBlock(const SharedPtr<BrushBlock> brushBlock)
	{
		mBrushBlock = brushBlock;
	}

	SharedPtr<Terrain::BrushBlock> Terrain::GetBrushBlock()
	{
		return mBrushBlock;
	}

	float Terrain::GetAmplitudeScaling()
	{
		return mAmplitudeScaling;
	}

	void Terrain::SetAmplitudeScaling(float amplitudeScaling)
	{
		mAmplitudeScaling = amplitudeScaling;
	}

	TerrainMaterial Terrain::GetMaterial(std::string material)
	{
		if (mMaterials.find(material) != mMaterials.end())
		{
			return mMaterials[material];
		}
		else
			assert(0);
	}
}
