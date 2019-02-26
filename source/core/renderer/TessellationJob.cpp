#include "core/renderer/TessellationJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Renderer.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/UIOverlay.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/QueryPool.h"
#include "Camera.h"
#include "Input.h"
#include <random>

namespace Utopian
{
	TessellationJob::TessellationJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		image = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
		depthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddColorAttachment(image);
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/tessellation/tessellation.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/tessellation.frag";
		shaderCreateInfo.tescShaderPath = "data/shaders/tessellation/tessellation.tesc";
		shaderCreateInfo.teseShaderPath = "data/shaders/tessellation/tessellation.tese";
		shaderCreateInfo.geometryShaderPath = "data/shaders/tessellation/tessellation.geom";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), shaderCreateInfo);

		//mEffect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		mEffect->GetPipeline()->AddTessellationState(4);

		mEffect->CreatePipeline();

		viewProjectionBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		settingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_settings", &settingsBlock);

		Vk::Texture* diffuseTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_11_DIF.jpg");
		Vk::Texture* normalTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_11_NRM.jpg");
		Vk::Texture* displacementTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_11_DISP.jpg");

		diffuseArray.AddTexture(diffuseTexture->imageView, renderTarget->GetSampler());
		normalArray.AddTexture(normalTexture->imageView, renderTarget->GetSampler());
		displacementArray.AddTexture(displacementTexture->imageView, renderTarget->GetSampler());

		diffuseTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_17_DIF.jpg");
		normalTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_17_NRM.jpg");
		displacementTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_17_DISP.jpg");

		diffuseArray.AddTexture(diffuseTexture->imageView, renderTarget->GetSampler());
		normalArray.AddTexture(normalTexture->imageView, renderTarget->GetSampler());
		displacementArray.AddTexture(displacementTexture->imageView, renderTarget->GetSampler());

		diffuseTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_21_DIF.jpg");
		normalTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_21_NRM.jpg");
		displacementTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_21_DISP.jpg");

		diffuseArray.AddTexture(diffuseTexture->imageView, renderTarget->GetSampler());
		normalArray.AddTexture(normalTexture->imageView, renderTarget->GetSampler());
		displacementArray.AddTexture(displacementTexture->imageView, renderTarget->GetSampler());

		mEffect->BindCombinedImage("samplerDiffuse", &diffuseArray);
		mEffect->BindCombinedImage("samplerNormal", &normalArray);
		mEffect->BindCombinedImage("samplerDisplacement", &displacementArray);

		const uint32_t size = 640;
		//gScreenQuadUi().AddQuad(size + 20, height - (size + 310), size, size, image.get(), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(0u, 0u, width, height, image.get(), renderTarget->GetSampler(), 1u);

		mQueryPool = std::make_shared<Vk::QueryPool>(device);
	}

	TessellationJob::~TessellationJob()
	{
	}

	void TessellationJob::Init(const std::vector<BaseJob*>& renderers)
	{
		SunShaftJob* sunShaftJob = static_cast<SunShaftJob*>(renderers[JobGraph::SUN_SHAFT_INDEX]);
		SetWaitSemaphore(sunShaftJob->GetCompletedSemahore());
	
		GeneratePatches(128.0, 128);
		GenerateTerrainMaps();

		Vk::gEffectManager().RegisterRecompileCallback(&TessellationJob::EffectRecomiledCallback, this);
	}
	
	void TessellationJob::EffectRecomiledCallback(std::string name)
	{
		RenderHeightmap();
		RenderNormalmap();
		RenderBlendmap();
		RenderBlendmapBrush();
		RenderHeightmapBrush();

		//RetrieveHeightmap();
	}

	void TessellationJob::GenerateTerrainMaps()
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

		sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler->Create();

		// Bind terrain height and normal maps
		mEffect->BindCombinedImage("samplerHeightmap", heightmapImage.get(), sampler.get());
		mEffect->BindCombinedImage("samplerNormalmap", normalImage.get(), sampler.get());
		mEffect->BindCombinedImage("samplerBlendmap", blendmapImage.get(), sampler.get());

		// Testing
		RetrieveHeightmap();

		testSampler = std::make_shared<Vk::Sampler>(mDevice, false);
		testSampler->createInfo.magFilter = VK_FILTER_NEAREST;
		testSampler->createInfo.minFilter = VK_FILTER_NEAREST;
		testSampler->Create();

		/*const uint32_t size = 440;
		gScreenQuadUi().AddQuad(300 + 20, mHeight - (size + 310), size, size, heightmapImage.get(), heightmapRenderTarget->GetSampler());
		gScreenQuadUi().AddQuad(300 + size + 20, mHeight - (size + 310), size, size, normalImage.get(), normalRenderTarget->GetSampler());
		gScreenQuadUi().AddQuad(300 + 2 * size + 20, mHeight - (size + 310), size, size, blendmapImage.get(), blendmapRenderTarget->GetSampler());
		gScreenQuadUi().AddQuad(300 + 2 * size + 20, mHeight - (2 * size + 310), size, size, hostImage.get(), testSampler.get());*/
	}

	void TessellationJob::RetrieveHeightmap()
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

		//std::vector<glm::vec4> pixels;
		for (uint32_t y = 0; y < hostImage->GetHeight(); y++)
		{
			glm::vec4* row = (glm::vec4*)data;
			for (uint32_t x = 0; x < hostImage->GetHeight(); x++)
			{
				//pixels.push_back(*(row));
				heightmap.push_back((*row).x);
				row++;
			}
			data += subResourceLayout.rowPitch;
		}

		vkUnmapMemory(mDevice->GetVkDevice(), hostImage->GetDeviceMemory());
	}

	void TessellationJob::SetupHeightmapEffect()
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
	
	void TessellationJob::SetupNormalmapEffect()
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

	void TessellationJob::SetupBlendmapEffect()
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

		mBlendmapEffect->BindCombinedImage("samplerHeightmap", heightmapImage.get(), heightmapRenderTarget->GetSampler());
		mBlendmapEffect->BindCombinedImage("samplerNormalmap", normalImage.get(), heightmapRenderTarget->GetSampler());
		mBlendmapEffect->BindUniformBuffer("UBO_settings", &settingsBlock);
	}

	void TessellationJob::SetupBlendmapBrushEffect()
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

	void TessellationJob::SetupHeightmapBrushEffect()
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

	void TessellationJob::RenderHeightmap()
	{
		heightmapRenderTarget->Begin("Heightmap pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = heightmapRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapEffect->GetPipeline());
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapRenderTarget->End();
	}

	void TessellationJob::RenderNormalmap()
	{
		normalRenderTarget->Begin("Normalmap pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = normalRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mNormalmapEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mNormalmapEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		normalRenderTarget->End();
	}

	void TessellationJob::RenderBlendmap()
	{
		blendmapRenderTarget->Begin("Blendmap pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = blendmapRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mBlendmapEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mBlendmapEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		blendmapRenderTarget->End();
	}

	void TessellationJob::RenderBlendmapBrush()
	{
		brushBlock.data.brushPos = brushPos;
		brushBlock.UpdateMemory();

		blendmapBrushRenderTarget->Begin("Blendmap brush pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = blendmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mBlendmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mBlendmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		blendmapBrushRenderTarget->End();
	}

	void TessellationJob::RenderHeightmapBrush()
	{
		brushBlock.data.brushPos = brushPos;
		brushBlock.UpdateMemory();

		heightmapBrushRenderTarget->Begin("Heightmap brush pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = heightmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mHeightmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapBrushRenderTarget->End();
	}

	void TessellationJob::GeneratePatches(float cellSize, int numCells)
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
				vertex.Tex = glm::vec2((float)x / (numCells-1), (float)z / (numCells-1));
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

	void TessellationJob::Render(const JobInput& jobInput)
	{
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.data.time = gTimer().GetTime();

		const Frustum& frustum = gRenderer().GetMainCamera()->GetFrustum();
		memcpy(viewProjectionBlock.data.frustumPlanes, frustum.planes.data(), sizeof(glm::vec4) * 6);

		viewProjectionBlock.UpdateMemory();

		settingsBlock.data.viewportSize = glm::vec2(mWidth, mHeight);
		settingsBlock.data.tessellationFactor = jobInput.renderingSettings.tessellationFactor;
		settingsBlock.data.edgeSize = 200.0f;
		settingsBlock.data.amplitude = jobInput.renderingSettings.terrainAmplitude;
		settingsBlock.data.textureScaling = jobInput.renderingSettings.terrainTextureScaling;
		settingsBlock.data.wireframe = jobInput.renderingSettings.terrainWireframe;
		settingsBlock.UpdateMemory();

		// Todo: Note: Hack: The blendmap depends on rendering settings that are not uploaded to GPU until here
		static bool first = true;
		if (first) {
			RenderBlendmap();
			RenderBlendmapBrush();
			first = false;
		}

		renderTarget->BeginCommandBuffer("Tessellation pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		mQueryPool->Reset(commandBuffer);

		renderTarget->BeginRenderPass();

		if (IsEnabled())
		{
			mQueryPool->Begin(commandBuffer);

			glm::mat4 world = glm::mat4();
			Vk::PushConstantBlock pushConsts(world);
			commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);

			Vk::Mesh* mesh = mQuadModel->mMeshes[0];
			commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);

			mQueryPool->End(commandBuffer);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());

		mQueryPool->RetreiveResults();
	}

	void TessellationJob::Update()
	{
		// Display Actor creation list
		Vk::UIOverlay::BeginWindow("Tessellation statistics", glm::vec2(300.0f, 10.0f), 400.0f);

		Vk::UIOverlay::TextV("VS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::INPUT_ASSEMBLY_VERTICES_INDEX));
		Vk::UIOverlay::TextV("TC invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_CONTROL_SHADER_PATCHES_INDEX));
		Vk::UIOverlay::TextV("TE invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX));
		Vk::UIOverlay::TextV("FS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::FRAGMENT_SHADER_INVOCATIONS_INDEX));

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

		Vk::UIOverlay::TextV("Terrain cam pos: %.4f, %.4f", TransformToUv(cameraPos.x, 0).x, TransformToUv(0, cameraPos.z).y);
		Vk::UIOverlay::TextV("Height: %.2f", height);
		Vk::UIOverlay::TextV("Intersection: %.2f, %.2f, %.2f", intersection.x, intersection.y, intersection.z);
		Vk::UIOverlay::TextV("Brush pos: %.4f, %.4f", brushPos.x, brushPos.y);

		if (ImGui::Button("Brush stroke"))
		{
			brushPos.x += 0.05;
			RenderBlendmapBrush();
			RenderHeightmapBrush();
			RenderNormalmap();
		}

		Vk::UIOverlay::EndWindow();
	}

	glm::vec2 TessellationJob::TransformToUv(float x, float z)
	{
		// Transform to terrain UV coordinates [0, 1]
		glm::vec2 uv = glm::vec2(x, z);
		uv += terrainSize / 2.0f;
		uv /= terrainSize;

		// To get correct UV coordinates
		uv = glm::vec2(1.0, 1.0) - uv;
		return uv;
	}

	float TessellationJob::GetHeight(float x, float z)
	{
		float height = -1.0f;
		
		glm::vec2 uv = TransformToUv(x, z);

		uint32_t col = floorf(uv.x * hostImage->GetWidth());
		uint32_t row = floorf(uv.y * hostImage->GetHeight());

		if (row >= 0 && col >= 0 && row < hostImage->GetWidth() && col < hostImage->GetHeight())
		{
			height = heightmap[row * hostImage->GetWidth() + col] * settingsBlock.data.amplitude * -1; // Todo: Fix amplitude and -1
		}

		return height;
	}

	glm::vec3 TessellationJob::GetIntersectPoint(Ray ray)
	{
		Ray shorterRay = LinearSearch(ray);

		// This is good enough accuracy for now
		return shorterRay.origin;
		//return BinarySearch(shorterRay);
	}

	Ray TessellationJob::LinearSearch(Ray ray)
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

	glm::vec3 TessellationJob::BinarySearch(Ray ray)
	{
		float accuracy = 10.0f;
		float heightAtStartingPoint = GetHeight(ray.origin.x, ray.origin.z);
		float currentError = ray.origin.y - heightAtStartingPoint;
		int counter = 0;
		while (currentError > accuracy && counter < 1000)
		{
			counter++;
			//ray.direction /= 2.0f;

			glm::vec3 nextPoint = ray.origin + ray.direction;
			float heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);

			// Is the next point above the terrain?
			if (nextPoint.y > heightAtNextPoint)
			{
				ray.origin = nextPoint;
				currentError = ray.origin.y - heightAtNextPoint;
			}
		}

		// Return infinity if the ray dont strike anything
		if (counter >= 1000)
			return glm::vec3(std::numeric_limits<float>::infinity(), 0, 0);

		return ray.origin;
	}
}
