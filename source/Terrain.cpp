#include "vulkan/Renderer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/CommandBuffer.h"
#include "Camera.h"
#include "Terrain.h"

Block::Block(Vulkan::Renderer* renderer, uint32_t blockSize)
{
	float spacing = 100.0f;
	float size = blockSize * spacing;
	for (uint32_t x = 0; x < blockSize; x++)
	{
		for (uint32_t y = 0; y < blockSize; y++)
		{
			for (uint32_t z = 0; z < blockSize; z++)
			{
				mPointList.push_back(CubeVertex(-size/2 + x * spacing, -size/2 + y * spacing, -size/2 + z * spacing));
			}
		}
	}

	VkDeviceSize bufferSize = blockSize * blockSize * blockSize * sizeof(CubeVertex);
	mVertexBuffer = new Vulkan::Buffer(renderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, mPointList.data());
}

Block::~Block()
{
	delete mVertexBuffer;
}

Vulkan::Buffer* Block::GetVertexBuffer()
{
	return mVertexBuffer;
}

Terrain::Terrain(Vulkan::Renderer* renderer, Vulkan::Camera* camera)
{
	mRenderer = renderer;
	mCamera = camera;
	mTestBlock = new Block(renderer, mBlockSize);

	/* 
		Initialize Vulkan handles
	*/
	mCommandBuffer = renderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	mDescriptorSetLayout = new Vulkan::DescriptorSetLayout(mRenderer->GetDevice());
	mDescriptorSetLayout->AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT);
	mDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_GEOMETRY_BIT);
	mDescriptorSetLayout->Create();

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.push_back(mDescriptorSetLayout->GetVkHandle());
	Vulkan::PushConstantRange pushConstantRange = Vulkan::PushConstantRange(VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(PushConstantBlock));
	mPipelineLayout = new Vulkan::PipelineLayout(mRenderer->GetDevice(), descriptorSetLayouts, &pushConstantRange);

	mDescriptorPool = new Vulkan::DescriptorPool(mRenderer->GetDevice());
	mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
	mDescriptorPool->Create();

	mUniformBuffer.CreateBuffer(mRenderer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	int edgeTable[256] = {
		0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
		0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
		0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
		0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
		0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
		0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
		0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
		0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
		0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
		0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
		0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
		0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
		0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
		0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
		0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
		0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
		0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
		0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
		0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
		0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
		0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
		0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
		0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
		0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
		0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
		0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };

	mEdgeTableTexture = mRenderer->mTextureLoader->LoadTexture(mDescriptorSetLayout, mDescriptorPool, edgeTable, VK_FORMAT_R32_UINT, 256, 1, sizeof(int));
	//mEdgeTableTexture->CreateDescriptorSet(mRenderer->GetDevice(), mDescriptorSetLayout, mDescriptorPool);

	mDescriptorSet = new Vulkan::DescriptorSet(mRenderer->GetDevice(), mDescriptorSetLayout, mDescriptorPool);
	mDescriptorSet->AllocateDescriptorSets();
	mDescriptorSet->BindUniformBuffer(1, &mUniformBuffer.GetDescriptor());
	mDescriptorSet->BindCombinedImage(0, &mEdgeTableTexture->GetTextureDescriptorInfo());
	mDescriptorSet->UpdateDescriptorSets();

	mVertexDescription = new Vulkan::VertexDescription();
	mVertexDescription->AddBinding(0, sizeof(CubeVertex), VK_VERTEX_INPUT_RATE_VERTEX);					
	mVertexDescription->AddAttribute(0, Vulkan::Vec3Attribute());	

	Vulkan::Shader* shader = mRenderer->mShaderManager->CreateShader("data/shaders/terrain/base.vert.spv", "data/shaders/terrain/base.frag.spv", "data/shaders/terrain/marching_cubes.geom.spv");
	mPipeline = new Vulkan::Pipeline(mRenderer->GetDevice(), mPipelineLayout, mRenderer->GetRenderPass(), mVertexDescription, shader);
	mPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	mPipeline->Create();

	// Cube corner offsets
	mUniformBuffer.data.offsets[0] = vec4(0, 0, 0, 0);
	mUniformBuffer.data.offsets[1] = vec4(mVoxelSize, 0, 0, 0);
	mUniformBuffer.data.offsets[2] = vec4(mVoxelSize, mVoxelSize, 0, 0);
	mUniformBuffer.data.offsets[3] = vec4(0, mVoxelSize, 0, 0);
	mUniformBuffer.data.offsets[4] = vec4(0, 0, mVoxelSize, 0);
	mUniformBuffer.data.offsets[5] = vec4(mVoxelSize, 0, mVoxelSize, 0);
	mUniformBuffer.data.offsets[6] = vec4(mVoxelSize, mVoxelSize, mVoxelSize, 0);
	mUniformBuffer.data.offsets[7] = vec4(0, mVoxelSize, mVoxelSize, 0);
	mUniformBuffer.data.color = vec4(0, 1, 0, 1);

	mUniformBuffer.UpdateMemory(mRenderer->GetVkDevice());
}

Terrain::~Terrain()
{
	delete mTestBlock;
	delete mDescriptorSetLayout;
	delete mPipelineLayout;
	delete mDescriptorPool;
	delete mDescriptorSet;
	delete mPipeline;
	delete mVertexDescription;
}

void Terrain::Update()
{
	static float time = 0.0f;

	if(mUpdateTimer)
		time += 0.002f;

	// TEMP:
	mUniformBuffer.data.projection = mCamera->GetProjection();
	mUniformBuffer.data.view = mCamera->GetView();
	mUniformBuffer.data.voxelSize = mVoxelSize;
	mUniformBuffer.data.time = time;
	mUniformBuffer.UpdateMemory(mRenderer->GetVkDevice());

	// Temp
	VkCommandBuffer commandBuffer = mCommandBuffer->GetVkHandle();

	// Build mesh rendering command buffer
	mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());

	mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
	mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

	mCommandBuffer->CmdBindPipeline(mPipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 1, &mDescriptorSet->descriptorSet, 0, NULL);

	// Push the world matrix constant
	Vulkan::PushConstantBlock pushConstantBlock;
	pushConstantBlock.world = glm::mat4();
	pushConstantBlock.worldInvTranspose = glm::mat4(); 

	// NOTE: For some reason the translation needs to be negated when rendering
	// Otherwise the physical representation does not match the rendered scene
	pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
	pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
	pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

	mCommandBuffer->CmdPushConstants(mPipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

	mCommandBuffer->CmdBindVertexBuffer(0, 1, mTestBlock->GetVertexBuffer());
	vkCmdDraw(commandBuffer, mBlockSize*mBlockSize*mBlockSize, 1, 0, 0); // TODO: Vulkan::Buffer should have a vertexCount member?

	mCommandBuffer->End();
}

void Terrain::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN:
			if (wParam == 'P')
			{
				mUpdateTimer = !mUpdateTimer;
			}
			break;
	}
}
