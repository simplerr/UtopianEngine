#include "MarchingCubes.h"
#include <core/Profiler.h>
#include <glm/matrix.hpp>
#include <string>
#include <time.h>
#include <vulkan/vulkan_core.h>
#include "core/Input.h"
#include "core/Log.h"
#include "core/renderer/Renderer.h"
#include "core/Window.h"
#include "core/renderer/RendererUtility.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
#include "core/MiniCamera.h"
#include "core/Engine.h"
#include "LookupTables.h"
#include "Block.h"

bool operator<(BlockKey const& a, BlockKey const& b)
{
	if (a.x != b.x)
       return (a.x < b.x);

   if (a.y != b.y)
       return (a.y < b.y);

   return (a.z < b.z);
}

struct PushConstantBlock {
	glm::mat4 world;
	glm::vec4 color;

	PushConstantBlock(glm::mat4 _world, glm::vec4 _color = glm::vec4(1.0f)) {
		world = _world;
		color = _color;
	}
};

MarchingCubes::MarchingCubes(Utopian::Window* window)
	: mWindow(window)
{
	// Start Utopian Engine
	Utopian::gEngine().Start(window, "Marching Cubes demo");
	Utopian::gEngine().StartModules();
	Utopian::gEngine().RegisterUpdateCallback(&MarchingCubes::UpdateCallback, this);
	Utopian::gEngine().RegisterRenderCallback(&MarchingCubes::DrawCallback, this);
	Utopian::gEngine().RegisterDestroyCallback(&MarchingCubes::DestroyCallback, this);

	mVulkanApp = Utopian::gEngine().GetVulkanApp();

	Utopian::gProfiler().SetEnabled(true);

	InitResources();
}

MarchingCubes::~MarchingCubes()
{
	Utopian::gEngine().Destroy();
}

void MarchingCubes::DestroyCallback()
{
	// Free Vulkan resources
	mMarchingCubesEffect = nullptr;
	mTerrainEffect = nullptr;
	mTerrainEffectWireframe = nullptr;
	mTerrainCommandBuffer = nullptr;
	mTerrainRenderTarget = nullptr;
	mTerrainColorImage = nullptr;
	mTerrainPositionImage = nullptr;
	mTerrainDepthImage = nullptr;
	mTerrainCompletedSemaphore = nullptr;
	mEdgeTableTexture = nullptr;
	mTriangleTableTexture = nullptr;
	mNoiseSampler = nullptr;
	mNoiseEffect = nullptr;
	mSdfImage = nullptr;
	mBrushEffect = nullptr;
	mIntersectionEffect = nullptr;

	mMarchingInputParameters.GetBuffer()->Destroy();
	mTerrainInputParameters.GetBuffer()->Destroy();
	mTerrainSettings.GetBuffer()->Destroy();
	mCounterSSBO.GetBuffer()->Destroy();
	mBrushInputParameters.GetBuffer()->Destroy();
	mIntersectionOutputSSBO.GetBuffer()->Destroy();
	mIntersectionInputUBO.GetBuffer()->Destroy();

	for (auto& block : mBlockList)
		delete block.second;
}

void MarchingCubes::InitResources()
{
	uint32_t width = mWindow->GetWidth();
	uint32_t height = mWindow->GetHeight();
	Vk::Device* device = mVulkanApp->GetDevice();

	mCamera = std::make_shared<MiniCamera>(mOrigin + glm::vec3(1400, 1400, 1400), glm::vec3(25, 0, 25), 1, 50000, 10.0f, width, height);

	InitNoiseTextureEffect(device);
	InitBrushEffect(device);
	InitMarchingCubesEffect(device, width, height);
	InitTerrainEffect(device, width, height);
	InitIntersectionEffect(device, width, height);

	GenerateNoiseTexture();

	//gScreenQuadUi().AddQuad(50, 50, 256, 256, mSdfImage.get(), mNoiseSampler.get());
}

void MarchingCubes::InitNoiseTextureEffect(Vk::Device* device)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/generate_noise.comp";
	mNoiseEffect = Vk::Effect::Create(device, nullptr, effectDesc);

	mSdfImage = std::make_shared<Utopian::Vk::ImageStorage>(device, mNoiseTextureSize, mNoiseTextureSize, mNoiseTextureSize,
															"3D Noise Texture", VK_FORMAT_R32_SFLOAT);

	mNoiseEffect->BindImage("sdfImage", *mSdfImage);

	mNoiseSampler = std::make_shared<Utopian::Vk::Sampler>(device);
}

void MarchingCubes::InitBrushEffect(Vk::Device* device)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/terrain_brush.comp";
	mBrushEffect = Vk::Effect::Create(device, nullptr, effectDesc);

	mBrushInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mBrushInputParameters.data.brushSize = 8.0f;
	mBrushInputParameters.data.brushStrength = 50.0f;
	mBrushInputParameters.data.mode = 0; // Add

	mBrushEffect->BindImage("sdfImage", *mSdfImage);
	mBrushEffect->BindUniformBuffer("UBO_input", mBrushInputParameters);
}

void MarchingCubes::InitMarchingCubesEffect(Vk::Device* device, uint32_t width, uint32_t height)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/marching_cubes.comp";
	mMarchingCubesEffect = Vk::Effect::Create(device, nullptr, effectDesc);

	mMarchingInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mMarchingInputParameters.data.offsets[0] = glm::vec4(0, 0, 0, 0);
	mMarchingInputParameters.data.offsets[1] = glm::vec4(mVoxelSize, 0, 0, 0);
	mMarchingInputParameters.data.offsets[2] = glm::vec4(mVoxelSize, mVoxelSize, 0, 0);
	mMarchingInputParameters.data.offsets[3] = glm::vec4(0, mVoxelSize, 0, 0);
	mMarchingInputParameters.data.offsets[4] = glm::vec4(0, 0, mVoxelSize, 0);
	mMarchingInputParameters.data.offsets[5] = glm::vec4(mVoxelSize, 0, mVoxelSize, 0);
	mMarchingInputParameters.data.offsets[6] = glm::vec4(mVoxelSize, mVoxelSize, mVoxelSize, 0);
	mMarchingInputParameters.data.offsets[7] = glm::vec4(0, mVoxelSize, mVoxelSize, 0);
	mMarchingInputParameters.data.color = glm::vec4(0, 1, 0, 1);
	mMarchingInputParameters.data.voxelSize = mVoxelSize;
	mMarchingInputParameters.data.viewDistance = mViewDistance;
	mMarchingInputParameters.data.voxelsInBlock = mVoxelsInBlock;
	mMarchingInputParameters.data.flatNormals = false;
	mMarchingInputParameters.UpdateMemory();

	mCounterSSBO.Create(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	mEdgeTableTexture = Utopian::Vk::gTextureLoader().CreateTexture(edgeTable, VK_FORMAT_R32_SINT, 256, 1, 1, sizeof(int));
	mTriangleTableTexture = Utopian::Vk::gTextureLoader().CreateTexture(triTable, VK_FORMAT_R32_SINT, 16, 256, 1, sizeof(int));

	mMarchingCubesEffect->BindUniformBuffer("UBO_input", mMarchingInputParameters);
	mMarchingCubesEffect->BindStorageBuffer("CounterSSBO", mCounterSSBO);
	mMarchingCubesEffect->BindCombinedImage("edgeTableTex", *mEdgeTableTexture);
	mMarchingCubesEffect->BindCombinedImage("triangleTableTex", *mTriangleTableTexture);
	mMarchingCubesEffect->BindCombinedImage("sdfImage", *mSdfImage, *mNoiseSampler);
}

void MarchingCubes::InitTerrainEffect(Vk::Device* device, uint32_t width, uint32_t height)
{
	mTerrainCompletedSemaphore = std::make_shared<Vk::Semaphore>(device);
	mVulkanApp->SetWaitSubmitSemaphore(mTerrainCompletedSemaphore);

	mTerrainColorImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "Terrain color image");
	mTerrainDepthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT, "Terrain depth image");
	mTerrainPositionImage = std::make_shared<Vk::ImageStorage>(device, width, height, 1, "Terrain position image", VK_FORMAT_R32G32B32A32_SFLOAT);

	mTerrainRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
	mTerrainRenderTarget->AddWriteOnlyColorAttachment(mTerrainColorImage);
	mTerrainRenderTarget->AddWriteOnlyColorAttachment(mTerrainPositionImage, VK_IMAGE_LAYOUT_GENERAL);
	mTerrainRenderTarget->AddWriteOnlyDepthAttachment(mTerrainDepthImage);
	mTerrainRenderTarget->SetClearColor(47.0 / 255, 141.0 / 255, 255.0 / 255);
	mTerrainRenderTarget->Create();

	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.vertexShaderPath = "source/demos/marching_cubes/terrain.vert";
	effectDesc.shaderDesc.fragmentShaderPath = "source/demos/marching_cubes/terrain.frag";
	effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	mTerrainEffect = Vk::Effect::Create(device, mTerrainRenderTarget->GetRenderPass(), effectDesc);

	effectDesc.pipelineDesc.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	mTerrainEffectWireframe = Vk::Effect::Create(device, mTerrainRenderTarget->GetRenderPass(), effectDesc);

	mTerrainInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mTerrainSettings.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mTerrainSettings.data.mode = 0; // Phong

	mTerrainEffect->BindUniformBuffer("UBO", mTerrainInputParameters);
	mTerrainEffect->BindUniformBuffer("UBO_settings", mTerrainSettings);
	mTerrainEffectWireframe->BindUniformBuffer("UBO", mTerrainInputParameters);
	mTerrainEffectWireframe->BindUniformBuffer("UBO_settings", mTerrainSettings);

	gScreenQuadUi().AddQuad(0, 0, width, height, mTerrainColorImage.get(), mTerrainRenderTarget->GetSampler());
}

void MarchingCubes::InitIntersectionEffect(Vk::Device* device, uint32_t width, uint32_t height)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/terrain_intersection.comp";
	mIntersectionEffect = Vk::Effect::Create(device, nullptr, effectDesc);

	mIntersectionOutputSSBO.Create(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	mIntersectionInputUBO.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	mIntersectionEffect->BindImage("terrainPositionImage", *mTerrainPositionImage);
	mIntersectionEffect->BindUniformBuffer("UBO_input", mIntersectionInputUBO);
	mIntersectionEffect->BindStorageBuffer("SSBO_output", mIntersectionOutputSSBO);
}

glm::ivec3 MarchingCubes::GetBlockCoordinate(glm::vec3 position)
{
	int32_t blockX = position.x / (float)(mVoxelSize * mVoxelsInBlock);
	int32_t blockY = position.y / (float)(mVoxelSize * mVoxelsInBlock);
	int32_t blockZ = position.z / (float)(mVoxelSize * mVoxelsInBlock);

	return glm::ivec3(blockX, blockY, blockZ);
}

void MarchingCubes::GenerateNoiseTexture()
{
	Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	commandBuffer.CmdBindPipeline(mNoiseEffect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mNoiseEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(mNoiseTextureSize, mNoiseTextureSize, mNoiseTextureSize);
	commandBuffer.Flush();
}

void MarchingCubes::ApplyTerrainBrush()
{
	glm::vec3 target = mBrushPos;
	glm::vec3 localPosition = target - glm::vec3((800 - mViewDistance) * mVoxelsInBlock * mVoxelSize);
	glm::ivec3 startCoord = (localPosition / (float)mVoxelSize) - mBrushTextureRegion / 2.0f;

	mBrushInputParameters.data.startCoord = startCoord;
	mBrushInputParameters.data.textureRegionSize = mBrushTextureRegion;
	mBrushInputParameters.UpdateMemory();

	Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	commandBuffer.CmdBindPipeline(mBrushEffect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mBrushEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(mBrushTextureRegion, mBrushTextureRegion, mBrushTextureRegion); // Don't dispatch full texture to optimize performance
	commandBuffer.Flush();

	glm::ivec3 globalBlockCoord = GetBlockCoordinate(target);
	BlockKey blockKey(globalBlockCoord.x, globalBlockCoord.y, globalBlockCoord.z);

	// Update surrounding blocks
	// Note: This only works for positive block coordinates
	for (int32_t x = -1; x <= 1; x++) // Should be <= to cover full range
	{
		for (int32_t z = -1; z <= 1; z++)
		{
			for (int32_t y = -1; y <= 1; y++)
			{
				glm::ivec3 coord = glm::ivec3(x, y, z) + globalBlockCoord;
				glm::vec3 blockCenter = coord * mVoxelSize * mVoxelsInBlock + (mVoxelSize * mVoxelsInBlock / 2);
				if (glm::distance(blockCenter, target) < mBrushInputParameters.data.brushSize * 20.0f + (mVoxelSize * mVoxelsInBlock / 2)) // Why 20?
				{
					BlockKey blockKey(coord.x, coord.y, coord.z);
					if (mBlockList.find(blockKey) != mBlockList.end())
						mBlockList[blockKey]->modified = true;
				}
			}
		}
	}
}

void MarchingCubes::UpdateBlockList()
{
	// 1) Which blocks should be rendered? Based on the camera position
		// Transform the camera position in to block grid coordinate
	// 2) Are they already added? 
	// 3) Add them

	glm::vec3 cameraPos = mCamera->GetPosition();

	if (mStaticPosition)
		cameraPos = mOrigin;

	glm::ivec3 cameraCoord = GetBlockCoordinate(cameraPos);

	// Make all blocks invisible
	for (auto blockIter : mBlockList)
	{
		blockIter.second->visible = false;
	}

	// Note: This only works for positive block coordinates
	for (int32_t x = -mViewDistance; x < mViewDistance; x++) // Should be <= to cover full range
	{
		for (int32_t z = -mViewDistance; z < mViewDistance; z++)
		{
			for (int32_t y = -mViewDistance; y < mViewDistance; y++)
			{
				glm::ivec3 coord = glm::ivec3(x, y, z) + cameraCoord;

				BlockKey blockKey(coord.x, coord.y, coord.z);
				if (mBlockList.find(blockKey) == mBlockList.end())
				{
					glm::vec3 position = glm::vec3(coord.x * mVoxelsInBlock * mVoxelSize,
												   coord.y * mVoxelsInBlock * mVoxelSize,
												   coord.z * mVoxelsInBlock * mVoxelSize);

					glm::vec3 color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
					Block* block = new Block(mVulkanApp->GetDevice(), position, color, mVoxelsInBlock, mVoxelSize);

					mBlockList[blockKey] = block;

					UTO_LOG("loaded block (" + std::to_string(x) + ", "
					 						 + std::to_string(y) + ", "
					 						 + std::to_string(z) + ")");
				}
				else
				{
					mBlockList[blockKey]->visible = true;
				}
			}
		}
	}
}

void MarchingCubes::GenerateBlocks()
{
	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;

		// Generate the vertex buffer for the block
		if (!block->generated || block->modified)
		{
			mMarchingInputParameters.data.projection = mCamera->GetProjection();
			mMarchingInputParameters.data.view = mCamera->GetView();
			mMarchingInputParameters.data.voxelSize = mVoxelSize;
			mMarchingInputParameters.data.time = gTimer().GetTime();
			mMarchingInputParameters.UpdateMemory();

			// Reset block vertex count
			mCounterSSBO.data.numVertices = 0;
			mCounterSSBO.UpdateMemory();

			// Bind new vertex buffer SSBO descriptor
			mMarchingCubesEffect->BindStorageBuffer("VertexSSBO", &block->bufferInfo);

			Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			commandBuffer.CmdBindPipeline(mMarchingCubesEffect->GetPipeline());
			commandBuffer.CmdBindDescriptorSets(mMarchingCubesEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			PushConstantBlock pushConsts(glm::translate(glm::mat4(), block->position));
			commandBuffer.CmdPushConstants(mMarchingCubesEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer.CmdDispatch(32, 32, 32);
			commandBuffer.Flush();

			// Get # of vertices so we can tell how many to draw
			uint32_t* mapped;
			mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
			uint32_t numVertices = *(uint32_t*)mapped;
			mCounterSSBO.UnmapMemory();

			block->numVertices = numVertices;
			block->generated = true;
			block->modified = false;
		}
	}
}

void MarchingCubes::RenderBlocks()
{
	mTerrainInputParameters.data.projection = mCamera->GetProjection();
	mTerrainInputParameters.data.view = mCamera->GetView();
	mTerrainInputParameters.data.eyePos = mCamera->GetPosition();
	mTerrainInputParameters.UpdateMemory();

	mTerrainSettings.UpdateMemory(); // Updated from ImGui combobox

	mTerrainRenderTarget->Begin("Terrain pass", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	Vk::CommandBuffer* commandBuffer = mTerrainRenderTarget->GetCommandBuffer();

	SharedPtr<Utopian::Vk::Effect> effect = mWireframe ? mTerrainEffectWireframe : mTerrainEffect;

	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		if (block->visible && block->generated && block->numVertices != 0)
		{
			commandBuffer->CmdBindPipeline(effect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(effect);

			commandBuffer->CmdBindVertexBuffer(BINDING_0, 1, block->GetVertexBuffer());

			PushConstantBlock pushConstantBlock(glm::translate(glm::mat4(), block->position), glm::vec4(block->color, 1.0f));
			commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL,
													sizeof(pushConstantBlock), &pushConstantBlock);

			commandBuffer->CmdDraw(block->numVertices, 1, 0, 0);
		}
	}

	mTerrainRenderTarget->End(mVulkanApp->GetImageAvailableSemaphore(), mTerrainCompletedSemaphore);
}

void MarchingCubes::QueryBrushPosition()
{
	mIntersectionInputUBO.data.mousePosition = gInput().GetMousePosition();
	mIntersectionInputUBO.UpdateMemory();

	Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	commandBuffer.CmdBindPipeline(mIntersectionEffect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mIntersectionEffect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(1, 1, 1);
	commandBuffer.Flush();

	glm::vec3* mapped;
	mIntersectionOutputSSBO.MapMemory(0, sizeof(glm::vec3), 0, (void**)&mapped);
	mBrushPos = *(glm::vec3*)mapped;
	mIntersectionOutputSSBO.UnmapMemory();
}

void MarchingCubes::ActivateBlockRegeneration()
{
	for (auto blockIter : mBlockList)
		blockIter.second->modified = true;
}

void MarchingCubes::UpdateCallback()
{
	glm::vec3 cameraPos = mCamera->GetPosition();
	glm::ivec3 blockCoord = GetBlockCoordinate(cameraPos);

	ImGuiRenderer::BeginWindow("Raytracing Demo", glm::vec2(10, 150), 300.0f);
	ImGui::Text("Camera pos: (%.2f %.2f %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
	ImGui::Text("Camera origin pos: (%.2f %.2f %.2f)", cameraPos.x - mOrigin.x, cameraPos.y - mOrigin.y, cameraPos.z - mOrigin.z);
	ImGui::Text("Block (%d, %d, %d)", blockCoord.x, blockCoord.y, blockCoord.z);
	ImGui::Text("Brush pos: (%.2f %.2f %.2f)", mBrushPos.x, mBrushPos.y, mBrushPos.z);
	ImGui::Text("Num blocks: %d", (int)mBlockList.size());

	if (ImGui::CollapsingHeader("Options", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Static position:", &mStaticPosition);
		ImGui::Checkbox("Wireframe:", &mWireframe);
		ImGui::SliderFloat("Brush size:", &mBrushInputParameters.data.brushSize, 1.0f, 16.0f);
		ImGui::SliderFloat("Brush strength:", &mBrushInputParameters.data.brushStrength, 1.0f, 100.0f);
		ImGui::Combo("Terrain render option", &mTerrainSettings.data.mode, "Phong\0Normals\0Block cells\0");

		bool flatNormals = mMarchingInputParameters.data.flatNormals;
		if (ImGui::Checkbox("Flat normals:", &flatNormals))
		{
			mMarchingInputParameters.data.flatNormals = flatNormals;
			ActivateBlockRegeneration();
		}
	}


	mCamera->Update();

	ImGuiRenderer::EndWindow();

	if (gInput().KeyPressed(VK_SPACE))
		mBrushInputParameters.data.mode = !mBrushInputParameters.data.mode;

	if (gInput().KeyDown(VK_LBUTTON))
	{
		QueryBrushPosition();
		ApplyTerrainBrush();
	}

	// Recompile shaders
	if (gInput().KeyPressed('R'))
	{
		Vk::gEffectManager().RecompileModifiedShaders();
		GenerateNoiseTexture();

		// Regenerate all blocks since the algorithm might have changed
		ActivateBlockRegeneration();
	}

	if (gInput().KeyPressed('T'))
		ActivateBlockRegeneration();


	UpdateBlockList();
	GenerateBlocks();
}

void MarchingCubes::DrawCallback()
{
	// Update uniforms
	mMarchingInputParameters.data.time = 0.0f;
	mMarchingInputParameters.UpdateMemory();

	RenderBlocks();

	// Todo: Should be in Engine somewhere
	gScreenQuadUi().Render(mVulkanApp);
}

void MarchingCubes::Run()
{
	Utopian::gEngine().Run();
}

void MarchingCubes::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Utopian::gEngine().HandleMessages(hWnd, uMsg, wParam, lParam);
}