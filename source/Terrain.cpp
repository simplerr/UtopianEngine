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

	// Marching cubes uniform buffer
	mUniformBuffer.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	mUniformBuffer.data.offsets[0] = vec4(0, 0, 0, 0);
	mUniformBuffer.data.offsets[1] = vec4(mVoxelSize, 0, 0, 0);
	mUniformBuffer.data.offsets[2] = vec4(mVoxelSize, mVoxelSize, 0, 0);
	mUniformBuffer.data.offsets[3] = vec4(0, mVoxelSize, 0, 0);
	mUniformBuffer.data.offsets[4] = vec4(0, 0, mVoxelSize, 0);
	mUniformBuffer.data.offsets[5] = vec4(mVoxelSize, 0, mVoxelSize, 0);
	mUniformBuffer.data.offsets[6] = vec4(mVoxelSize, mVoxelSize, mVoxelSize, 0);
	mUniformBuffer.data.offsets[7] = vec4(0, mVoxelSize, mVoxelSize, 0);
	mUniformBuffer.data.color = vec4(0, 1, 0, 1);
	mUniformBuffer.data.voxelSize = mVoxelSize;
	mUniformBuffer.UpdateMemory(mRenderer->GetVkDevice());

	/* 
		Initialize Vulkan handles
	*/
	mCommandBuffer = renderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	mDescriptorSetLayout = new Vulkan::DescriptorSetLayout(mRenderer->GetDevice());
	mDescriptorSetLayout->AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	mDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	mDescriptorSetLayout->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	mDescriptorSetLayout->Create();

	mComputeDescriptorSetLayout = new Vulkan::DescriptorSetLayout(mRenderer->GetDevice());
	mComputeDescriptorSetLayout->AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
	mComputeDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
	mComputeDescriptorSetLayout->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
	mComputeDescriptorSetLayout->Create();

	mBlockDescriptorSetLayout = new Vulkan::DescriptorSetLayout(mRenderer->GetDevice());
	mBlockDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	mBlockDescriptorSetLayout->AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	mBlockDescriptorSetLayout->Create();

	mComputeBlockDescriptorSetLayout = new Vulkan::DescriptorSetLayout(mRenderer->GetDevice());
	mComputeBlockDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
	mComputeBlockDescriptorSetLayout->AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
	mComputeBlockDescriptorSetLayout->Create();

	mPipelineLayout = new Vulkan::PipelineLayout(mRenderer->GetDevice());
	mPipelineLayout->AddDescriptorSetLayout(mDescriptorSetLayout);
	mPipelineLayout->AddDescriptorSetLayout(mBlockDescriptorSetLayout);
	mPipelineLayout->AddPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstantBlock));
	mPipelineLayout->Create();

	mComputePipelineLayout = new Vulkan::PipelineLayout(mRenderer->GetDevice());
	mComputePipelineLayout->AddDescriptorSetLayout(mComputeDescriptorSetLayout);
	mComputePipelineLayout->AddDescriptorSetLayout(mComputeBlockDescriptorSetLayout);
	mComputePipelineLayout->AddPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstantBlock));
	mComputePipelineLayout->Create();

	mDescriptorPool = new Vulkan::DescriptorPool(mRenderer->GetDevice());
	mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
	mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NUM_MAX_STORAGE_BUFFERS); // NOTE:
	mDescriptorPool->Create();

	mEdgeTableTexture = mRenderer->mTextureLoader->CreateTexture(edgeTable, VK_FORMAT_R32_UINT, 256, 1, sizeof(int));
	mTriangleTableTexture = mRenderer->mTextureLoader->CreateTexture(triTable, VK_FORMAT_R32_UINT, 16, 256, sizeof(int));


	mVertexDescription = new Vulkan::VertexDescription();
	mVertexDescription->AddBinding(0, sizeof(CubeVertex), VK_VERTEX_INPUT_RATE_VERTEX);					
	mVertexDescription->AddAttribute(0, Vulkan::Vec3Attribute());	

	Vulkan::Shader* shader = mRenderer->mShaderManager->CreateShader("data/shaders/terrain/base.vert.spv", "data/shaders/terrain/base.frag.spv", "data/shaders/terrain/marching_cubes.geom.spv");
	mGeometryPipeline = new Vulkan::Pipeline(mRenderer->GetDevice(), mPipelineLayout, mRenderer->GetRenderPass(), mVertexDescription, shader);
	mGeometryPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	mGeometryPipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	mGeometryPipeline->Create();

	mTerrainEffect.Init(renderer);

	BuildPointList();
	
	Vulkan::Shader* computeShader = mRenderer->mShaderManager->CreateComputeShader("data/shaders/marching_cubes/marching_cubes.comp.spv");
	mComputePipeline = new Vulkan::ComputePipeline(mRenderer->GetDevice(), mPipelineLayout, computeShader);
	mComputePipeline->Create();

	mDescriptorSet = new Vulkan::DescriptorSet(mRenderer->GetDevice(), mDescriptorSetLayout, mDescriptorPool);
	mDescriptorSet->AllocateDescriptorSets();
	mDescriptorSet->BindUniformBuffer(1, &mUniformBuffer.GetDescriptor());
	mDescriptorSet->BindCombinedImage(0, &mEdgeTableTexture->GetTextureDescriptorInfo());
	mDescriptorSet->BindCombinedImage(2, &mTriangleTableTexture->GetTextureDescriptorInfo());
	mDescriptorSet->UpdateDescriptorSets();

	//mCommandBuffer->ToggleActive();
}

Terrain::~Terrain()
{
	delete mDescriptorSetLayout;
	delete mPipelineLayout;
	delete mDescriptorPool;
	delete mDescriptorSet;
	delete mGeometryPipeline;
	delete mComputePipeline;
	delete mVertexDescription;
	delete mEdgeTableTexture;
	delete mTriangleTableTexture;
	delete mMarchingCubesVB;
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
				Block* block = new Block(mRenderer, position, color, mVoxelsInBlock, mVoxelSize, mBlockDescriptorSetLayout, mDescriptorPool); // NOTE: The descriptor set layout

				mBlockList[blockKey] = block;

				Vulkan::VulkanDebug::ConsolePrint(x, "loaded blockX: ");
				Vulkan::VulkanDebug::ConsolePrint(z, "loaded blockZ: ");
			}
			else
			{
				mBlockList[blockKey]->SetVisible(true);
			}
		}
	}
}

void Terrain::GenerateBlocks()
{
	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		if (mUseComputeShader)
		{			
			// Generate the vertex buffer for the block 
			if (!block->IsGenerated() || block->IsModified())
			{
				Vulkan::CommandBuffer commandBuffer = Vulkan::CommandBuffer(mRenderer->GetDevice(), mRenderer->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

				commandBuffer.CmdBindPipeline(mComputePipeline);
				VkDescriptorSet descriptorSets[2] = { mDescriptorSet->descriptorSet, block->GetDescriptorSet()->descriptorSet };
				vkCmdBindDescriptorSets(commandBuffer.GetVkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout->GetVkHandle(), 0, 2, descriptorSets, 0, NULL);

				// Push the world matrix constant
				Vulkan::PushConstantBlock pushConstantBlock;
				pushConstantBlock.world = glm::mat4();
				//pushConstantBlock.worldInvTranspose = glm::mat4();

				glm::vec3 position = block->GetPosition();
				pushConstantBlock.world = glm::translate(glm::mat4(), block->GetPosition());
				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				commandBuffer.CmdPushConstants(mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

				// TODO: Transform block position (PUSH CONSTS!)
				vkCmdDispatch(commandBuffer.GetVkHandle(), 32, 32, 32);

				commandBuffer.Flush(mRenderer->GetQueue()->GetVkHandle(), mRenderer->GetCommandPool());

				uint32_t* mapped;
				block->mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
				uint32_t numVertices = *(uint32_t*)mapped;
				block->mCounterSSBO.UnmapMemory();

				block->SetNumVertices(numVertices);
				block->SetGenerated(true);
				block->SetModified(false);
			}
		}
		else
		{
			// Generate the vertex buffer for the block 
			if (!block->IsGenerated() || block->IsModified())
			{
				Vulkan::CommandBuffer commandBuffer = Vulkan::CommandBuffer(mRenderer->GetDevice(), mRenderer->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

				commandBuffer.CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
				commandBuffer.CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
				commandBuffer.CmdBindPipeline(mGeometryPipeline);

				VkDescriptorSet descriptorSets[2] = { mDescriptorSet->descriptorSet, block->GetDescriptorSet()->descriptorSet };
				vkCmdBindDescriptorSets(commandBuffer.GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 2, descriptorSets, 0, NULL);

				//VkDescriptorSet descriptorSets[3] = { mRenderer->mCameraDescriptorSet->descriptorSet, mRenderer->mLightDescriptorSet->descriptorSet, textureDescriptorSet };
				//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderer->GetPipelineLayout()->GetVkHandle(), 0, 3, descriptorSets, 0, NULL);

				commandBuffer.CmdBindVertexBuffer(0, 1, mMarchingCubesVB);

				// Push the world matrix constant
				Vulkan::PushConstantBlock pushConstantBlock;
				pushConstantBlock.world = glm::mat4();
				pushConstantBlock.worldInvTranspose = glm::mat4();

				pushConstantBlock.world = glm::translate(glm::mat4(), block->GetPosition());
				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				commandBuffer.CmdPushConstants(mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

				vkCmdDraw(commandBuffer.GetVkHandle(), mVoxelsInBlock*mVoxelsInBlock*mVoxelsInBlock, 1, 0, 0); // TODO: Vulkan::Buffer should have a vertexCount member?

				commandBuffer.Flush(mRenderer->GetQueue()->GetVkHandle(), mRenderer->GetCommandPool());

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
}

void Terrain::Update()
{
	UpdateBlockList();
	GenerateBlocks();

	static float time = 0.0f;

	if(mUpdateTimer)
		time += 0.002f;

	static bool test = true;

	// TEMP:
	mUniformBuffer.data.projection = mCamera->GetProjection();
	mUniformBuffer.data.view = mCamera->GetView();
	mUniformBuffer.data.voxelSize = mVoxelSize;
	mUniformBuffer.data.time = time;
	mUniformBuffer.UpdateMemory(mRenderer->GetVkDevice());

	mTerrainEffect.uniformBufferVS.data.projection = mCamera->GetProjection();
	mTerrainEffect.uniformBufferVS.data.view = mCamera->GetView();
	mTerrainEffect.uniformBufferVS.data.eyePos = mCamera->GetPosition();
	mTerrainEffect.uniformBufferVS.UpdateMemory(mRenderer->GetVkDevice());
	mTerrainEffect.uniformBufferPS.data.eyePos = mCamera->GetPosition(); // Test
	mTerrainEffect.uniformBufferPS.data.fogStart = 10000.0f; // Test
	mTerrainEffect.uniformBufferPS.data.fogDistance = 5400.0f;
	mTerrainEffect.uniformBufferPS.UpdateMemory(mRenderer->GetVkDevice());

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
			mCommandBuffer->CmdBindPipeline(mTerrainEffect.GetPipeline());
			VkDescriptorSet descriptorSets[1] = {mTerrainEffect.mDescriptorSet->descriptorSet};
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mTerrainEffect.GetPipelineLayout(), 0, 1, &mTerrainEffect.mDescriptorSet->descriptorSet, 0, NULL);

			mCommandBuffer->CmdBindVertexBuffer(BINDING_0, 1, block->GetVertexBuffer());

			// Push the world matrix constant
			Vulkan::PushConstantBasicBlock pushConstantBlock;
			pushConstantBlock.world = glm::mat4();
			pushConstantBlock.color = block->GetColor();

			pushConstantBlock.world = glm::translate(glm::mat4(), block->GetPosition());
			pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
			pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
			pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

			//mCommandBuffer->CmdPushConstants(mTerrainEffect->mBasicPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
			vkCmdPushConstants(commandBuffer, mTerrainEffect.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);

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

void Terrain::BuildPointList()
{
	float size = mVoxelsInBlock * mVoxelSize;
	for (uint32_t x = 0; x < mVoxelsInBlock; x++)
	{
		for (uint32_t y = 0; y < mVoxelsInBlock; y++)
		{
			for (uint32_t z = 0; z < mVoxelsInBlock; z++)
			{
				mPointList.push_back(CubeVertex(-size / 2 + x * mVoxelSize, -size / 2 + y * mVoxelSize, -size / 2 + z * mVoxelSize));
			}
		}
	}

	VkDeviceSize bufferSize = mVoxelsInBlock * mVoxelsInBlock * mVoxelsInBlock * sizeof(CubeVertex);
	mMarchingCubesVB = new Vulkan::Buffer(mRenderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, mPointList.data());
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
