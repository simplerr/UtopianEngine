#include <glm/matrix.hpp>
#include <string>
#include "core/Engine.h"
#include "core/Input.h"
#include "core/Log.h"
#include "core/MiniCamera.h"
#include "core/Profiler.h"
#include "core/Window.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/Debug.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
#include "MarchingCubes.h"
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

	// Wait on the terrain job before submitting the frame
	mTerrainJob.completedSemaphore = std::make_shared<Vk::Semaphore>(mVulkanApp->GetDevice());
	mVulkanApp->SetWaitSubmitSemaphore(mTerrainJob.completedSemaphore);
}

MarchingCubes::~MarchingCubes()
{
	Utopian::gEngine().Destroy();
}

void MarchingCubes::DestroyCallback()
{
	// Todo: Remove this when the engine shutdown sequence is improved
	mMarchingCubesJob.effect = nullptr;
	mTerrainJob.effect = nullptr;
	mTerrainJob.effectWireframe = nullptr;
	mTerrainJob.renderTarget = nullptr;
	mTerrainJob.colorImage = nullptr;
	mTerrainJob.positionImage = nullptr;
	mTerrainJob.depthImage = nullptr;
	mTerrainJob.completedSemaphore = nullptr;
	mMarchingCubesJob.edgeTableTexture = nullptr;
	mMarchingCubesJob.triangleTableTexture = nullptr;
	mNoiseJob.sampler = nullptr;
	mNoiseJob.effect = nullptr;
	mNoiseJob.sdfImage = nullptr;
	mBrushJob.effect = nullptr;
	mIntersectionJob.effect = nullptr;

	mMarchingCubesJob.inputUBO.GetBuffer()->Destroy();
	mTerrainJob.inputUBO.GetBuffer()->Destroy();
	mTerrainJob.fragmentInputUBO.GetBuffer()->Destroy();
	mMarchingCubesJob.counterSSBO.GetBuffer()->Destroy();
	mBrushJob.inputUBO.GetBuffer()->Destroy();
	mIntersectionJob.outputSSBO.GetBuffer()->Destroy();
	mIntersectionJob.inputUBO.GetBuffer()->Destroy();

	for (auto& block : mBlockList)
		delete block.second;
}

void MarchingCubes::InitResources()
{
	Vk::Device* device = mVulkanApp->GetDevice();
	uint32_t width = mWindow->GetWidth();
	uint32_t height = mWindow->GetHeight();

	mCamera = std::make_shared<MiniCamera>(mOrigin + glm::vec3(1400, 1400, 1400), glm::vec3(25, 0, 25), 1, 50000, 10.0f, width, height);

	InitNoiseJob(device);
	InitBrushJob(device);
	InitMarchingCubesJob(device, width, height);
	InitTerrainJob(device, width, height);
	InitIntersectionJob(device, width, height);

	RunNoiseJob();

	//gScreenQuadUi().AddQuad(50, 50, 256, 256, mNoiseJob.sdfImage.get(), mNoiseJob.sampler.get());
}

void MarchingCubes::InitNoiseJob(Vk::Device* device)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/shaders/generate_noise.comp";
	mNoiseJob.effect = Vk::Effect::Create(device, nullptr, effectDesc);

	mNoiseJob.sdfImage = std::make_shared<Utopian::Vk::ImageStorage>(device, mNoiseJob.textureSize, mNoiseJob.textureSize, mNoiseJob.textureSize,
																	 "3D Noise Texture", VK_FORMAT_R32_SFLOAT);

	mNoiseJob.effect->BindImage("sdfImage", *mNoiseJob.sdfImage);

	mNoiseJob.sampler = std::make_shared<Utopian::Vk::Sampler>(device);
}

void MarchingCubes::InitBrushJob(Vk::Device* device)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/shaders/terrain_brush.comp";
	mBrushJob.effect = Vk::Effect::Create(device, nullptr, effectDesc);

	mBrushJob.inputUBO.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mBrushJob.inputUBO.data.brushSize = 8.0f;
	mBrushJob.inputUBO.data.brushStrength = 50.0f;
	mBrushJob.inputUBO.data.mode = 0; // Add
	mBrushJob.inputUBO.UpdateMemory();

	mBrushJob.effect->BindImage("sdfImage", *mNoiseJob.sdfImage);
	mBrushJob.effect->BindUniformBuffer("UBO_input", mBrushJob.inputUBO);
}

void MarchingCubes::InitMarchingCubesJob(Vk::Device* device, uint32_t width, uint32_t height)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/shaders/marching_cubes.comp";
	mMarchingCubesJob.effect = Vk::Effect::Create(device, nullptr, effectDesc);

	mMarchingCubesJob.inputUBO.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mMarchingCubesJob.inputUBO.data.offsets[0] = glm::vec4(0, 0, 0, 0);
	mMarchingCubesJob.inputUBO.data.offsets[1] = glm::vec4(mVoxelSize, 0, 0, 0);
	mMarchingCubesJob.inputUBO.data.offsets[2] = glm::vec4(mVoxelSize, mVoxelSize, 0, 0);
	mMarchingCubesJob.inputUBO.data.offsets[3] = glm::vec4(0, mVoxelSize, 0, 0);
	mMarchingCubesJob.inputUBO.data.offsets[4] = glm::vec4(0, 0, mVoxelSize, 0);
	mMarchingCubesJob.inputUBO.data.offsets[5] = glm::vec4(mVoxelSize, 0, mVoxelSize, 0);
	mMarchingCubesJob.inputUBO.data.offsets[6] = glm::vec4(mVoxelSize, mVoxelSize, mVoxelSize, 0);
	mMarchingCubesJob.inputUBO.data.offsets[7] = glm::vec4(0, mVoxelSize, mVoxelSize, 0);
	mMarchingCubesJob.inputUBO.data.color = glm::vec4(0, 1, 0, 1);
	mMarchingCubesJob.inputUBO.data.voxelSize = (float)mVoxelSize;
	mMarchingCubesJob.inputUBO.data.viewDistance = mViewDistance;
	mMarchingCubesJob.inputUBO.data.voxelsInBlock = mVoxelsInBlock;
	mMarchingCubesJob.inputUBO.data.flatNormals = false;
	mMarchingCubesJob.inputUBO.UpdateMemory();

	mMarchingCubesJob.counterSSBO.Create(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
										 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	mMarchingCubesJob.edgeTableTexture = Utopian::Vk::gTextureLoader().CreateTexture(edgeTable, VK_FORMAT_R32_SINT, 256, 1, 1, sizeof(int));
	mMarchingCubesJob.triangleTableTexture = Utopian::Vk::gTextureLoader().CreateTexture(triTable, VK_FORMAT_R32_SINT, 16, 256, 1, sizeof(int));

	mMarchingCubesJob.effect->BindUniformBuffer("UBO_input", mMarchingCubesJob.inputUBO);
	mMarchingCubesJob.effect->BindStorageBuffer("CounterSSBO", mMarchingCubesJob.counterSSBO);
	mMarchingCubesJob.effect->BindCombinedImage("edgeTableTex", *mMarchingCubesJob.edgeTableTexture);
	mMarchingCubesJob.effect->BindCombinedImage("triangleTableTex", *mMarchingCubesJob.triangleTableTexture);
	mMarchingCubesJob.effect->BindCombinedImage("sdfImage", *mNoiseJob.sdfImage, *mNoiseJob.sampler);
}

void MarchingCubes::InitTerrainJob(Vk::Device* device, uint32_t width, uint32_t height)
{
	mTerrainJob.colorImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "Terrain color image");
	mTerrainJob.depthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT, "Terrain depth image");
	mTerrainJob.positionImage = std::make_shared<Vk::ImageStorage>(device, width, height, 1, "Terrain position image", VK_FORMAT_R32G32B32A32_SFLOAT);

	mTerrainJob.renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
	mTerrainJob.renderTarget->AddWriteOnlyColorAttachment(mTerrainJob.colorImage);
	mTerrainJob.renderTarget->AddWriteOnlyColorAttachment(mTerrainJob.positionImage, VK_IMAGE_LAYOUT_GENERAL);
	mTerrainJob.renderTarget->AddWriteOnlyDepthAttachment(mTerrainJob.depthImage);
	mTerrainJob.renderTarget->SetClearColor(47.0f / 255.0f, 141.0f / 255.0f, 1.0f);
	mTerrainJob.renderTarget->Create();

	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.vertexShaderPath = "source/demos/marching_cubes/shaders/terrain.vert";
	effectDesc.shaderDesc.fragmentShaderPath = "source/demos/marching_cubes/shaders/terrain.frag";
	effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	mTerrainJob.effect = Vk::Effect::Create(device, mTerrainJob.renderTarget->GetRenderPass(), effectDesc);

	effectDesc.pipelineDesc.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	mTerrainJob.effectWireframe = Vk::Effect::Create(device, mTerrainJob.renderTarget->GetRenderPass(), effectDesc);

	mTerrainJob.inputUBO.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mTerrainJob.fragmentInputUBO.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	mTerrainJob.fragmentInputUBO.data.mode = 0; // Phong

	mTerrainJob.effect->BindUniformBuffer("UBO", mTerrainJob.inputUBO);
	mTerrainJob.effect->BindUniformBuffer("UBO_fragInput", mTerrainJob.fragmentInputUBO);
	mTerrainJob.effectWireframe->BindUniformBuffer("UBO", mTerrainJob.inputUBO);
	mTerrainJob.effectWireframe->BindUniformBuffer("UBO_fragInput", mTerrainJob.fragmentInputUBO);

	gScreenQuadUi().AddQuad(0, 0, width, height, mTerrainJob.colorImage.get(), mTerrainJob.renderTarget->GetSampler());
}

void MarchingCubes::InitIntersectionJob(Vk::Device* device, uint32_t width, uint32_t height)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/shaders/terrain_intersection.comp";
	mIntersectionJob.effect = Vk::Effect::Create(device, nullptr, effectDesc);

	mIntersectionJob.outputSSBO.Create(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
										VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	mIntersectionJob.inputUBO.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	mIntersectionJob.effect->BindImage("terrainPositionImage", *mTerrainJob.positionImage);
	mIntersectionJob.effect->BindUniformBuffer("UBO_input", mIntersectionJob.inputUBO);
	mIntersectionJob.effect->BindStorageBuffer("SSBO_output", mIntersectionJob.outputSSBO);
}

glm::ivec3 MarchingCubes::GetBlockCoordinate(glm::vec3 position)
{
	int32_t blockX = (int32_t)(position.x / (float)(mVoxelSize * mVoxelsInBlock));
	int32_t blockY = (int32_t)(position.y / (float)(mVoxelSize * mVoxelsInBlock));
	int32_t blockZ = (int32_t)(position.z / (float)(mVoxelSize * mVoxelsInBlock));

	return glm::ivec3(blockX, blockY, blockZ);
}

void MarchingCubes::RunNoiseJob()
{
	Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	commandBuffer.CmdBindPipeline(mNoiseJob.effect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mNoiseJob.effect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(mNoiseJob.textureSize, mNoiseJob.textureSize, mNoiseJob.textureSize);
	commandBuffer.Flush();
}

void MarchingCubes::RunBrushJob()
{
	glm::vec3 target = mIntersectionJob.brushPos;
	glm::vec3 localPosition = target - glm::vec3((800.0f - mViewDistance) * mVoxelsInBlock * mVoxelSize);
	glm::ivec3 startCoord = (localPosition / (float)mVoxelSize) - mBrushJob.textureRegion / 2.0f;

	mBrushJob.inputUBO.data.startCoord = startCoord;
	mBrushJob.inputUBO.data.textureRegionSize = mBrushJob.textureRegion;
	mBrushJob.inputUBO.UpdateMemory();

	Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	// Don't dispatch full texture to optimize performance
	commandBuffer.CmdBindPipeline(mBrushJob.effect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mBrushJob.effect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(mBrushJob.textureRegion, mBrushJob.textureRegion, mBrushJob.textureRegion);
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
				if (glm::distance(blockCenter, target) < mBrushJob.inputUBO.data.brushSize * 20.0f + (mVoxelSize * mVoxelsInBlock / 2)) // Why 20?
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
					Block* block = new Block(mVulkanApp->GetDevice(), position, color, mVoxelsInBlock, (float)mVoxelSize);

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

void MarchingCubes::RunMarchingCubesJob()
{
	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;

		// Generate the vertex buffer for the block
		if (!block->generated || block->modified)
		{
			mMarchingCubesJob.inputUBO.data.projection = mCamera->GetProjection();
			mMarchingCubesJob.inputUBO.data.view = mCamera->GetView();
			mMarchingCubesJob.inputUBO.data.voxelSize = (float)mVoxelSize;
			mMarchingCubesJob.inputUBO.data.time = gTimer().GetTime();
			mMarchingCubesJob.inputUBO.UpdateMemory();

			// Reset block vertex count
			mMarchingCubesJob.counterSSBO.data.numVertices = 0;
			mMarchingCubesJob.counterSSBO.UpdateMemory();

			// Bind new vertex buffer SSBO descriptor
			mMarchingCubesJob.effect->BindStorageBuffer("VertexSSBO", &block->bufferInfo);

			Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			commandBuffer.CmdBindPipeline(mMarchingCubesJob.effect->GetPipeline());
			commandBuffer.CmdBindDescriptorSets(mMarchingCubesJob.effect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			PushConstantBlock pushConsts(glm::translate(glm::mat4(), block->position));
			commandBuffer.CmdPushConstants(mMarchingCubesJob.effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer.CmdDispatch(mVoxelsInBlock, mVoxelsInBlock, mVoxelsInBlock);
			commandBuffer.Flush();

			// Get # of vertices so we can tell how many to draw
			uint32_t* mapped;
			mMarchingCubesJob.counterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
			uint32_t numVertices = *(uint32_t*)mapped;
			mMarchingCubesJob.counterSSBO.UnmapMemory();

			block->numVertices = numVertices;
			block->generated = true;
			block->modified = false;
		}
	}
}

void MarchingCubes::RunTerrainJob()
{
	mTerrainJob.inputUBO.data.projection = mCamera->GetProjection();
	mTerrainJob.inputUBO.data.view = mCamera->GetView();
	mTerrainJob.inputUBO.data.eyePos = mCamera->GetPosition();
	mTerrainJob.inputUBO.UpdateMemory();

	mTerrainJob.fragmentInputUBO.data.brushPos = mIntersectionJob.brushPos;
	mTerrainJob.fragmentInputUBO.UpdateMemory();

	mTerrainJob.renderTarget->Begin("Terrain pass", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	Vk::CommandBuffer* commandBuffer = mTerrainJob.renderTarget->GetCommandBuffer();

	SharedPtr<Utopian::Vk::Effect> effect = mWireframe ? mTerrainJob.effectWireframe : mTerrainJob.effect;

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

	mTerrainJob.renderTarget->End(mVulkanApp->GetImageAvailableSemaphore(), mTerrainJob.completedSemaphore);
}

void MarchingCubes::RunIntersectionJob()
{
	mIntersectionJob.inputUBO.data.mousePosition = gInput().GetMousePosition();
	mIntersectionJob.inputUBO.UpdateMemory();

	Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	commandBuffer.CmdBindPipeline(mIntersectionJob.effect->GetPipeline());
	commandBuffer.CmdBindDescriptorSets(mIntersectionJob.effect, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	commandBuffer.CmdDispatch(1, 1, 1);
	commandBuffer.Flush();

	glm::vec3* mapped;
	mIntersectionJob.outputSSBO.MapMemory(0, sizeof(glm::vec3), 0, (void**)&mapped);
	mIntersectionJob.brushPos = *(glm::vec3*)mapped;
	mIntersectionJob.outputSSBO.UnmapMemory();
}

void MarchingCubes::ActivateBlockRegeneration()
{
	for (auto blockIter : mBlockList)
		blockIter.second->modified = true;
}

void MarchingCubes::UpdateCallback()
{
	mCamera->Update();
	UpdateUi();

	if (gInput().KeyPressed(VK_SPACE))
		mBrushJob.inputUBO.data.mode = !mBrushJob.inputUBO.data.mode;

	RunIntersectionJob();

	if (gInput().KeyDown(VK_LBUTTON))
		RunBrushJob();

	if (gInput().KeyPressed('T'))
		ActivateBlockRegeneration();

	UpdateBlockList();
	RunMarchingCubesJob();

	// Recompile shaders
	if (gInput().KeyPressed('R'))
	{
		Vk::gEffectManager().RecompileModifiedShaders();
		RunNoiseJob();

		// Regenerate all blocks since the algorithm might have changed
		ActivateBlockRegeneration();
	}
}

void MarchingCubes::UpdateUi()
{
	glm::vec3 cameraPos = mCamera->GetPosition();
	glm::ivec3 blockCoord = GetBlockCoordinate(cameraPos);

	ImGuiRenderer::BeginWindow("Raytracing Demo", glm::vec2(10, 150), 300.0f);
	ImGui::Text("Camera pos: (%.2f %.2f %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
	ImGui::Text("Camera origin pos: (%.2f %.2f %.2f)", cameraPos.x - mOrigin.x, cameraPos.y - mOrigin.y, cameraPos.z - mOrigin.z);
	ImGui::Text("Block (%d, %d, %d)", blockCoord.x, blockCoord.y, blockCoord.z);
	ImGui::Text("Brush pos: (%.2f %.2f %.2f)", mIntersectionJob.brushPos.x, mIntersectionJob.brushPos.y, mIntersectionJob.brushPos.z);
	ImGui::Text("Num blocks: %d", (int)mBlockList.size());

	if (ImGui::CollapsingHeader("Options", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Static position:", &mStaticPosition);
		ImGui::Checkbox("Wireframe:", &mWireframe);
		ImGui::SliderFloat("Brush size:", &mBrushJob.inputUBO.data.brushSize, 1.0f, 16.0f);
		ImGui::SliderFloat("Brush strength:", &mBrushJob.inputUBO.data.brushStrength, 1.0f, 100.0f);
		ImGui::Combo("Terrain render option", &mTerrainJob.fragmentInputUBO.data.mode, "Phong\0Normals\0Block cells\0");

		bool flatNormals = mMarchingCubesJob.inputUBO.data.flatNormals;
		if (ImGui::Checkbox("Flat normals:", &flatNormals))
		{
			mMarchingCubesJob.inputUBO.data.flatNormals = flatNormals;
			ActivateBlockRegeneration();
		}
	}

	ImGuiRenderer::EndWindow();
}

void MarchingCubes::DrawCallback()
{
	RunTerrainJob();

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