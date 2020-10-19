#include "MarchingCubes.h"
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
	mEdgeTableTexture = nullptr;
	mTriangleTableTexture = nullptr;
	mNoiseSampler = nullptr;
	mNoiseEffect = nullptr;
	mSdfImage = nullptr;
	mBrushEffect = nullptr;

	mMarchingInputParameters.GetBuffer()->Destroy();
	mTerrainInputParameters.GetBuffer()->Destroy();
	mCounterSSBO.GetBuffer()->Destroy();
	mBrushInputParameters.GetBuffer()->Destroy();

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

	GenerateNoiseTexture();

	gScreenQuadUi().AddQuad(50, 50, 512, 512, mSdfImage.get(), mNoiseSampler.get());
}

void MarchingCubes::InitNoiseTextureEffect(Vk::Device* device)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/demos/marching_cubes/generate_noise.comp";
	mNoiseEffect = Vk::Effect::Create(device, nullptr, effectDesc);

	mSdfImage = std::make_shared<Utopian::Vk::ImageStorage>(device, mNoiseTextureSize, mNoiseTextureSize, mNoiseTextureSize,
															"3D Noise Texture", VK_FORMAT_R8_SNORM);

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
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.vertexShaderPath = "source/demos/marching_cubes/terrain.vert";
	effectDesc.shaderDesc.fragmentShaderPath = "source/demos/marching_cubes/terrain.frag";
	effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	mTerrainEffect = Vk::Effect::Create(device, mVulkanApp->GetRenderPass(), effectDesc);

	effectDesc.pipelineDesc.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	mTerrainEffectWireframe = Vk::Effect::Create(device, mVulkanApp->GetRenderPass(), effectDesc);

	mTerrainInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	mTerrainEffect->BindUniformBuffer("UBO", mTerrainInputParameters);
	mTerrainEffectWireframe->BindUniformBuffer("UBO", mTerrainInputParameters);

	mTerrainCommandBuffer = std::make_shared<Vk::CommandBuffer>(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	mVulkanApp->AddSecondaryCommandBuffer(mTerrainCommandBuffer.get());
}

glm::ivec3 MarchingCubes::GetBlockCoordinate(glm::vec3 position)
{
	int32_t blockX = position.x / (float)(mVoxelSize * mVoxelsInBlock);
	int32_t blockY = position.y / (float)(mVoxelSize * mVoxelsInBlock);
	int32_t blockZ = position.z / (float)(mVoxelSize * mVoxelsInBlock);

	// blockX += position.x < 0 ? - 1 : 1;
	// blockY += position.y < 0 ? - 1 : 1;
	// blockZ += position.z < 0 ? - 1 : 1;

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
	glm::vec3 target = mCamera->GetPosition() + glm::normalize(mCamera->GetTarget() - mCamera->GetPosition()) * glm::vec3(500.0f);
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
	mTerrainInputParameters.UpdateMemory();

	mTerrainCommandBuffer->Begin(mVulkanApp->GetRenderPass(), mVulkanApp->GetCurrentFrameBuffer());
	mTerrainCommandBuffer->CmdSetViewPort(mVulkanApp->GetWindow()->GetWidth(), mVulkanApp->GetWindow()->GetHeight());
	mTerrainCommandBuffer->CmdSetScissor(mVulkanApp->GetWindow()->GetWidth(), mVulkanApp->GetWindow()->GetHeight());

	SharedPtr<Utopian::Vk::Effect> effect = mWireframe ? mTerrainEffectWireframe : mTerrainEffect;

	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		if (block->visible && block->generated && block->numVertices != 0)
		{
			mTerrainCommandBuffer->CmdBindPipeline(effect->GetPipeline());
			mTerrainCommandBuffer->CmdBindDescriptorSets(effect);

			mTerrainCommandBuffer->CmdBindVertexBuffer(BINDING_0, 1, block->GetVertexBuffer());

			PushConstantBlock pushConstantBlock(glm::translate(glm::mat4(), block->position), glm::vec4(block->color, 1.0f));
			mTerrainCommandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL,
													sizeof(pushConstantBlock), &pushConstantBlock);

			mTerrainCommandBuffer->CmdDraw(block->numVertices, 1, 0, 0);
		}
	}

	mTerrainCommandBuffer->End();
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
	ImGui::Text("Num blocks: %d", (int)mBlockList.size());
	ImGui::Checkbox("Static position:", &mStaticPosition);
	ImGui::Checkbox("Wireframe:", &mWireframe);
	ImGui::SliderFloat("Brush size:", &mBrushInputParameters.data.brushSize, 1.0f, 16.0f);

	bool flatNormals = mMarchingInputParameters.data.flatNormals;
	if (ImGui::Checkbox("Flat normals:", &flatNormals))
	{
		mMarchingInputParameters.data.flatNormals = flatNormals;
		ActivateBlockRegeneration();
	}

	mCamera->Update();

	ImGuiRenderer::EndWindow();

	if (gInput().KeyPressed(VK_SPACE))
		mBrushInputParameters.data.mode = !mBrushInputParameters.data.mode;

	if (gInput().KeyDown(VK_LBUTTON))
		ApplyTerrainBrush();

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