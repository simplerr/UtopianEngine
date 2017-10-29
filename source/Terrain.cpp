#include <fstream>
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/CommandBuffer.h"
#include "Camera.h"
#include "Terrain.h"
#include "Block.h"
#include "MarchingCubes.h"

bool operator<(BlockKey const& a, BlockKey const& b)
{
	if (a.x != b.x)
       return (a.x < b.x);

   if (a.y != b.y)
       return (a.y < b.y);

   return (a.z < b.z);
}

Terrain::Terrain(Vulkan::Renderer* renderer, Vulkan::Camera* camera)
{
	mRenderer = renderer;
	mCamera = camera;

	/* 
		Initialize Vulkan handles
	*/
	mCommandBuffer = renderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	// NOTE: This must be done before Init()
	// Move to effect?
	mMarchingCubesEffect.edgeTableTex = mRenderer->mTextureLoader->CreateTexture(edgeTable, VK_FORMAT_R32_UINT, 256, 1, sizeof(int));
	mMarchingCubesEffect.triangleTableTex = mRenderer->mTextureLoader->CreateTexture(triTable, VK_FORMAT_R32_UINT, 16, 256, sizeof(int));

	/* Experimentation */
	const uint32_t w = 4;
	const uint32_t h = 4;
	const uint32_t d = 4;

	//char texture3d[w][h][d];
	char texture3d[w * h * d * 4];
	memset(texture3d, 0x00, sizeof(uint32_t) * w * h * d);

	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			for (int z = 0; z < d; z++)
			{
				//texture3d[x + y * w + z * w * h] = 0xFF;
				//texture3d[x + y * w + z * w * h + 1] = 0xFF;
				//texture3d[x + y * w + z * w * h + 2] = 0xFF;
				//texture3d[x + y * w + z * w * h + 3] = 0xFF;
			}
		}
	}

	texture3d[0] = 0xFF;
	texture3d[1] = 0xFF;
	//texture3d[2] = 0xFF;
	//texture3d[3] = 0xFF;

	//texture3d[0] = 0x00ff0000; // here the format is AABBGGRR
	//texture3d[1] = 0x0000ff00; // here the format is AABBGGRR
	//texture3d[2] = 0x000000ff; // here the format is AABBGGRR

	mTerrainEffect.texture3d = mRenderer->mTextureLoader->CreateTexture(texture3d, VK_FORMAT_R8G8B8A8_SRGB, w, h, sizeof(uint32_t), d);

	mTerrainEffect.Init(renderer);
	mMarchingCubesEffect.Init(renderer);

	mMarchingCubesEffect.ubo.data.offsets[0] = vec4(0, 0, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[1] = vec4(mVoxelSize, 0, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[2] = vec4(mVoxelSize, mVoxelSize, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[3] = vec4(0, mVoxelSize, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[4] = vec4(0, 0, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.offsets[5] = vec4(mVoxelSize, 0, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.offsets[6] = vec4(mVoxelSize, mVoxelSize, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.offsets[7] = vec4(0, mVoxelSize, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.color = vec4(0, 1, 0, 1);
	mMarchingCubesEffect.ubo.data.voxelSize = mVoxelSize;
	mMarchingCubesEffect.UpdateMemory(renderer->GetDevice());
}

Terrain::~Terrain()
{

}

void Terrain::UpdateBlockList()
{
	// 1) Which blocks should be rendered? Based on the camera position
		// Transform the camera position in to block grid coordinate
	// 2) Are they already added? 
	// 3) Add them

	glm::vec3 cameraPos = mCamera->GetPosition();
	int32_t blockX = cameraPos.x / (float)(mVoxelSize * mVoxelsInBlock) + 1;
	int32_t blockY = 0;
	int32_t blockZ = cameraPos.z / (float)(mVoxelSize * mVoxelsInBlock) + 1;

	// Make all blocks invisible
	/*for (auto blockIter : mBlockList)
	{
		blockIter.second->SetVisible(false);
	}*/

	for (int32_t x = blockX - mViewDistance; x <= (blockX + mViewDistance); x++)
	{
		for (int32_t z = blockZ - mViewDistance; z <= (blockZ + mViewDistance); z++)
		{
			BlockKey blockKey(x, 0, z);
			if (mBlockList.find(blockKey) == mBlockList.end())
			{
				glm::vec3 position = glm::vec3(x*mVoxelsInBlock*mVoxelSize, 0, z*mVoxelsInBlock*mVoxelSize);
				glm::vec3 color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
				Vulkan::DescriptorSetLayout setLayout1 = mMarchingCubesEffect.GetDescriptorSetLayout(SET_1);
				Block* block = new Block(mRenderer, position, color, mVoxelsInBlock, mVoxelSize, &setLayout1, mMarchingCubesEffect.GetDescriptorPool()); // NOTE: The descriptor set layout

				mBlockList[blockKey] = block;

				//Vulkan::VulkanDebug::ConsolePrint(x, "loaded blockX: ");
				//Vulkan::VulkanDebug::ConsolePrint(z, "loaded blockZ: ");
			}
			else
			{
				mBlockList[blockKey]->SetVisible(true);
			}
		}
	}
}

void Terrain::GenerateBlocks(float time)
{
	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		// Generate the vertex buffer for the block 
		if (!block->IsGenerated() || block->IsModified())
		{
			mMarchingCubesEffect.ubo.data.projection = mCamera->GetProjection();
			mMarchingCubesEffect.ubo.data.view = mCamera->GetView();
			mMarchingCubesEffect.ubo.data.voxelSize = mVoxelSize;
			mMarchingCubesEffect.ubo.data.time = time;
			mMarchingCubesEffect.UpdateMemory(mRenderer->GetDevice());

			Vulkan::CommandBuffer commandBuffer = Vulkan::CommandBuffer(mRenderer->GetDevice(), mRenderer->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			commandBuffer.CmdBindPipeline(mMarchingCubesEffect.GetComputePipeline());
			// TODO: Use the firstSet parameter to only update the Block descriptor
			VkDescriptorSet descriptorSets[2] = { mMarchingCubesEffect.GetDescriptorSet0(),
												  block->GetDescriptorSet()->descriptorSet }; // TODO: The block descriptor set is set in Terrain.cpp
			commandBuffer.CmdBindDescriptorSet(&mMarchingCubesEffect, 2, descriptorSets, VK_PIPELINE_BIND_POINT_COMPUTE);

			// Push the world matrix constant
			Vulkan::PushConstantBlock pushConstantBlock;
			pushConstantBlock.world = glm::mat4();

			glm::vec3 position = block->GetPosition();
			pushConstantBlock.world = glm::translate(glm::mat4(), block->GetPosition());
			pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
			pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
			pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

			commandBuffer.CmdPushConstants(&mMarchingCubesEffect, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
			commandBuffer.CmdDispatch(32, 32, 32);
			commandBuffer.Flush(mRenderer->GetQueue()->GetVkHandle(), mRenderer->GetCommandPool());

			// Get # of vertices so we can tell how many to draw
			uint32_t* mapped;
			block->mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
			uint32_t numVertices = *(uint32_t*)mapped;
			block->mCounterSSBO.UnmapMemory();

			block->SetNumVertices(numVertices);
			block->SetGenerated(true);
			block->SetModified(false);
		}
	}
}

void Terrain::Update()
{
	static float time = 0.0f;

	if(mUpdateTimer)
		time += 0.002f;

	UpdateBlockList();
	GenerateBlocks(time);

	mTerrainEffect.per_frame_vs.data.projection = mCamera->GetProjection();
	mTerrainEffect.per_frame_vs.data.view = mCamera->GetView();
	mTerrainEffect.per_frame_vs.data.eyePos = mCamera->GetPosition();
	mTerrainEffect.per_frame_ps.data.eyePos = mCamera->GetPosition(); // Test
	mTerrainEffect.per_frame_ps.data.fogStart = 10000.0f; // Test
	mTerrainEffect.per_frame_ps.data.fogDistance = 5400.0f;
	mTerrainEffect.UpdateMemory(mRenderer->GetDevice());

	VkCommandBuffer commandBuffer = mCommandBuffer->GetVkHandle();

	// Build mesh rendering command buffer
	mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());

	mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
	mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		if (block->IsVisible() && block->IsGenerated())
		{
			mTerrainEffect.SetPipeline(block->pipelineType);
				
			mCommandBuffer->CmdBindPipeline(mTerrainEffect.GetPipeline());
			VkDescriptorSet descriptorSets[1] = {mTerrainEffect.mDescriptorSet0->descriptorSet};
			mCommandBuffer->CmdBindDescriptorSet(&mTerrainEffect, 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

			mCommandBuffer->CmdBindVertexBuffer(BINDING_0, 1, block->GetVertexBuffer());

			// Push the world matrix constant
			Vulkan::PushConstantBasicBlock pushConstantBlock;
			pushConstantBlock.world = glm::mat4();
			pushConstantBlock.color = block->GetColor();

			pushConstantBlock.world = glm::translate(glm::mat4(), block->GetPosition());
			pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
			pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
			pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

			mCommandBuffer->CmdPushConstants(&mTerrainEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
			mCommandBuffer->CmdDraw(block->GetNumVertices(), 1, 0, 0);
		}
	}

	mCommandBuffer->End();
}

void Terrain::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN:
			if (wParam == 'G')
			{
				uint32_t numBlocks = 0;
				uint32_t numVertices = 0;
				for (auto blockIter : mBlockList)
				{
					Block* block = blockIter.second;
					numBlocks++;
					numVertices += block->GetNumVertices();
				}

				Vulkan::VulkanDebug::ConsolePrint(numBlocks, "numBlocks: ");
				Vulkan::VulkanDebug::ConsolePrint(numVertices, "numVertices: ");

				glm::vec3 cameraPos = mCamera->GetPosition();
				int32_t blockX = cameraPos.x / (float)(mVoxelSize * mVoxelsInBlock);
				int32_t blockY = 0;
				int32_t blockZ = cameraPos.z / (float)(mVoxelSize * mVoxelsInBlock);

				Vulkan::VulkanDebug::ConsolePrint(blockX, "blockX: ");
				Vulkan::VulkanDebug::ConsolePrint(blockZ, "blockZ: ");

				UpdateBlockList();
			}
			/*if (wParam == 'P')
			{
				mUpdateTimer = !mUpdateTimer;
			}
			if (wParam == 'G')
			{
				uint32_t* mapped;
				mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
				uint32_t count = *(uint32_t*)mapped;
				mCounterSSBO.UnmapMemory();

				Vulkan::VulkanDebug::ConsolePrint(*mapped, "numVertices: ");
				DumpDebug();
			}
			if (wParam == 'H')
			{
				mDrawGeneratedBuffer = !mDrawGeneratedBuffer;
				uint32_t* mapped;
				mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
				mNumVertices = *(uint32_t*)mapped;
				mCounterSSBO.UnmapMemory();
			}*/
			break;
	}
}

void Terrain::DumpDebug()
{
	//// Storage buffer test
	//std::ofstream fout("vertices.txt");

	//Vulkan::VulkanDebug::ConsolePrint("TEST OUTPUT STORAGE");

	//void* mappedData;
	//mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, &mappedData);
	//uint32_t numVertices = *(uint32_t*)mappedData;
	//mCounterSSBO.UnmapMemory();

	//fout << "numVertices: " << numVertices << std::endl;

	//mVertexSSBO.MapMemory(0, numVertices * sizeof(GeometryVertex), 0, &mappedData);

	//GeometryVertex* data = (GeometryVertex*)mappedData;
	//for (uint32_t i = 0; i < numVertices; i++)
	//{
	//	GeometryVertex d = *data;
	//	fout << "[x: " << d.pos.x << " y: " << d.pos.y << " z: " << d.pos.z << " w: " << d.pos.w << "] [nx: " << d.normal.x << " ny: " << d.normal.y << " nz: " << d.normal.z << " nw: " << d.normal.w << "]" << std::endl;

	//	data++;
	//}

	//mVertexSSBO.UnmapMemory();

	//fout.close();
}
