#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/Buffer.h"
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
	mDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT);
	mDescriptorSetLayout->Create();

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.push_back(mDescriptorSetLayout->GetVkHandle());
	Vulkan::PushConstantRange pushConstantRange = Vulkan::PushConstantRange(VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(PushConstantBlock));
	mPipelineLayout = new Vulkan::PipelineLayout(mRenderer->GetDevice(), descriptorSetLayouts, &pushConstantRange);

	mDescriptorPool = new Vulkan::DescriptorPool(mRenderer->GetDevice());
	mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	mDescriptorPool->Create();

	mUniformBuffer.CreateBuffer(mRenderer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	mDescriptorSet = new Vulkan::DescriptorSet(mRenderer->GetDevice(), mDescriptorSetLayout, mDescriptorPool);
	mDescriptorSet->AllocateDescriptorSets();
	mDescriptorSet->BindUniformBuffer(0, &mUniformBuffer.GetDescriptor());
	mDescriptorSet->UpdateDescriptorSets();

	mVertexDescription = new Vulkan::VertexDescription();
	mVertexDescription->AddBinding(0, sizeof(CubeVertex), VK_VERTEX_INPUT_RATE_VERTEX);					
	mVertexDescription->AddAttribute(0, Vulkan::Vec3Attribute());	

	Vulkan::Shader* shader = mRenderer->mShaderManager->CreateShader("data/shaders/terrain/base.vert.spv", "data/shaders/terrain/base.frag.spv", "data/shaders/terrain/marching_cubes.geom.spv");
	mPipeline = new Vulkan::Pipeline(mRenderer->GetDevice(), mPipelineLayout, mRenderer->GetRenderPass(), mVertexDescription, shader);
	mPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	mPipeline->Create();
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
	// TEMP:
	mUniformBuffer.data.projection = mCamera->GetProjection();
	mUniformBuffer.data.view = mCamera->GetView();
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
