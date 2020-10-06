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
	mTerrainCommandBuffer = nullptr;
	mEdgeTableTex = nullptr;
	mTriangleTableTex = nullptr;

	mMarchingInputParameters.GetBuffer()->Destroy();
	mTerrainInputParameters.GetBuffer()->Destroy();
	mCounterSSBO.GetBuffer()->Destroy();

	for (auto& block : mBlockList)
		delete block.second;
}

void MarchingCubes::InitResources()
{
	uint32_t width = mWindow->GetWidth();
	uint32_t height = mWindow->GetHeight();
	Vk::Device* device = mVulkanApp->GetDevice();

	mCamera = std::make_shared<MiniCamera>(glm::vec3(5, 25, 5), glm::vec3(25, 0, 25), 1, 50000, 1.0f, width, height);

	InitMarchingCubesEffect(device, width, height);
	InitTerrainEffect(device, width, height);

	//gScreenQuadUi().AddQuad(0, 0, width, height, mOutputImage.get(), mSampler.get());
}

void MarchingCubes::InitMarchingCubesEffect(Vk::Device* device, uint32_t width, uint32_t height)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.computeShaderPath = "source/marching_cubes_demo/marching_cubes.comp";
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
	mMarchingInputParameters.UpdateMemory();

	mCounterSSBO.Create(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	mEdgeTableTex = Utopian::Vk::gTextureLoader().CreateTexture(edgeTable, VK_FORMAT_R32_SINT, 256, 1, 1, sizeof(int));
	mTriangleTableTex = Utopian::Vk::gTextureLoader().CreateTexture(triTable, VK_FORMAT_R32_SINT, 16, 256, 1, sizeof(int));

	mMarchingCubesEffect->BindUniformBuffer("UBO_input", mMarchingInputParameters);
	mMarchingCubesEffect->BindStorageBuffer("CounterSSBO", mCounterSSBO);
	mMarchingCubesEffect->BindCombinedImage("edgeTableTex", *mEdgeTableTex);
	mMarchingCubesEffect->BindCombinedImage("triangleTableTex", *mTriangleTableTex);
}

void MarchingCubes::InitTerrainEffect(Vk::Device* device, uint32_t width, uint32_t height)
{
	Vk::EffectCreateInfo effectDesc;
	effectDesc.shaderDesc.vertexShaderPath = "source/marching_cubes_demo/terrain.vert";
	effectDesc.shaderDesc.fragmentShaderPath = "source/marching_cubes_demo/terrain.frag";
	effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	//effectDesc.pipelineDesc.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	mTerrainEffect = Vk::Effect::Create(device, mVulkanApp->GetRenderPass(), effectDesc);

	mTerrainInputParameters.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	mTerrainEffect->BindUniformBuffer("UBO", mTerrainInputParameters);

	mTerrainCommandBuffer = std::make_shared<Vk::CommandBuffer>(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	mVulkanApp->AddSecondaryCommandBuffer(mTerrainCommandBuffer.get());
}

void MarchingCubes::UpdateBlockList()
{
	// 1) Which blocks should be rendered? Based on the camera position
		// Transform the camera position in to block grid coordinate
	// 2) Are they already added? 
	// 3) Add them

	glm::vec3 cameraPos = mCamera->GetPosition();
	int32_t blockX = cameraPos.x / (float)(mVoxelSize * mVoxelsInBlock) + 1;
	int32_t blockY = cameraPos.y / (float)(mVoxelSize * mVoxelsInBlock) + 1;
	int32_t blockZ = cameraPos.z / (float)(mVoxelSize * mVoxelsInBlock) + 1;

	// Make all blocks invisible
	for (auto blockIter : mBlockList)
	{
		blockIter.second->visible = false;
	}

	for (int32_t x = blockX - mViewDistance; x <= (blockX + mViewDistance); x++)
	{
		for (int32_t z = blockZ - mViewDistance; z <= (blockZ + mViewDistance); z++)
		{
			for (int32_t y = blockY - mViewDistance; y <= (blockY + mViewDistance); y++)
			{
				BlockKey blockKey(x, y, z);
				if (mBlockList.find(blockKey) == mBlockList.end())
				{
					glm::vec3 position = glm::vec3(x * mVoxelsInBlock * mVoxelSize, y * mVoxelsInBlock * mVoxelSize, z * mVoxelsInBlock * mVoxelSize);
					glm::vec3 color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
					//color = glm::vec3(0.0f, 0.7f, 0.0f);
					Block* block = new Block(mVulkanApp->GetDevice(), position, color, mVoxelsInBlock, mVoxelSize);

					mBlockList[blockKey] = block;

					//Vulkan::UTO_LOG(x, "loaded blockX: ");
					//Vulkan::UTO_LOG(z, "loaded blockZ: ");
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

			Utopian::Vk::PushConstantBlock pushConsts(glm::translate(glm::mat4(), block->position));
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

	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		if (block->visible && block->generated && block->numVertices != 0)
		{
			mTerrainCommandBuffer->CmdBindPipeline(mTerrainEffect->GetPipeline());
			mTerrainCommandBuffer->CmdBindDescriptorSets(mTerrainEffect);

			mTerrainCommandBuffer->CmdBindVertexBuffer(BINDING_0, 1, block->GetVertexBuffer());

			Utopian::Vk::PushConstantBlock pushConstantBlock(glm::translate(glm::mat4(), block->position), glm::vec4(block->color, 1.0f));
			mTerrainCommandBuffer->CmdPushConstants(mTerrainEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL,
													sizeof(pushConstantBlock), &pushConstantBlock);

			mTerrainCommandBuffer->CmdDraw(block->numVertices, 1, 0, 0);
		}
	}

	mTerrainCommandBuffer->End();
}

void MarchingCubes::UpdateCallback()
{
	ImGuiRenderer::BeginWindow("Raytracing Demo", glm::vec2(10, 150), 300.0f);
	ImGuiRenderer::EndWindow();

	// Recompile shaders
	if (gInput().KeyPressed('R'))
	{
		Vk::gEffectManager().RecompileModifiedShaders();

		// Regenerate all blocks since the algorithm might have changed
		for (auto blockIter : mBlockList)
			blockIter.second->modified = true;
	}

	mCamera->Update();

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