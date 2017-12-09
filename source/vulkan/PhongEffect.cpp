#include "vulkan/PhongEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"

namespace Vulkan
{
	PhongEffect::PhongEffect()
	{
		SetPipeline(PipelineType2::BASIC);
	}

	void PhongEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_NUM_TEXTURES);
		mDescriptorPool->Create();
	}

	void PhongEffect::CreateVertexDescription(Device* device)
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription = new Vulkan::VertexDescription();
		mVertexDescription->AddBinding(BINDING_0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

		// We need to tell Vulkan about the memory layout for each attribute
		// 5 attributes: position, normal, texture coordinates, tangent and color
		// See Vertex struct
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 0 : Position
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 1 : Color
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 2 : Normal
		mVertexDescription->AddAttribute(BINDING_0, Vec2Attribute());	// Location 3 : Texture
		mVertexDescription->AddAttribute(BINDING_0, Vec4Attribute());	// Location 4 : Tangent
	}

	void PhongEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.AddUniformBuffer(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_2, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void PhongEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_vs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		per_frame_ps.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		
		// per_frame_vs
		Vulkan::DescriptorSetLayout* setLayout0 = mPipelineInterface.GetDescriptorSetLayout(SET_0);
		mCameraDescriptorSet = new Vulkan::DescriptorSet(device, setLayout0, mDescriptorPool);
		mCameraDescriptorSet->BindUniformBuffer(0, &per_frame_vs.GetDescriptor());
		mCameraDescriptorSet->UpdateDescriptorSets();

		// per_frame_ps
		Vulkan::DescriptorSetLayout* setLayout1 = mPipelineInterface.GetDescriptorSetLayout(SET_1);
		mLightDescriptorSet = new Vulkan::DescriptorSet(device, setLayout1, mDescriptorPool);
		mLightDescriptorSet->BindUniformBuffer(0, &per_frame_ps.GetDescriptor());
		mLightDescriptorSet->UpdateDescriptorSets();
	}

	void PhongEffect::CreatePipeline(Renderer* renderer)
	{
		// Load shader
		// [TODO] Move this into Pipeline?
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/phong/phong.vert.spv", "data/shaders/phong/phong.frag.spv");

		// Solid pipeline
		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[PipelineType2::BASIC] = pipeline;

		// Wireframe pipeline
		pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType2::WIREFRAME] = pipeline;

		// Test pipeline
		Shader* testShader = renderer->mShaderManager->CreateShader("data/shaders/test/test.vert.spv", "data/shaders/test/test.frag.spv");
		pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;
		pipeline->Create();
		mPipelines[PipelineType2::TEST] = pipeline;

		pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;
		// TODO: Disable depth test
		pipeline->Create();
		mPipelines[PipelineType2::DEBUG] = pipeline;
	}

	void PhongEffect::UpdateMemory(Device* device)
	{
		per_frame_vs.UpdateMemory(device->GetVkDevice());
		per_frame_ps.UpdateMemory(device->GetVkDevice());
	}

	void PhongEffect::VertexUniformBuffer::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(camera), 0, (void**)&mapped);
		memcpy(mapped, &camera, sizeof(camera));
		mBuffer->UnmapMemory();
	}

	int PhongEffect::VertexUniformBuffer::GetSize()
	{
		return sizeof(camera) + sizeof(constants);
	}

	void PhongEffect::FragmentUniformBuffer::UpdateMemory(VkDevice device)
	{
		// Map and update the light data
		uint8_t* mapped;
		uint32_t dataOffset = 0;
		uint32_t dataSize = lights.size() * sizeof(Vulkan::Light);
		mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
		memcpy(mapped, lights.data(), dataSize);
		mBuffer->UnmapMemory();

		// Map and update number of lights
		dataOffset += dataSize;
		dataSize = sizeof(constants);
		mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
		memcpy(mapped, &constants.numLights, dataSize);
		mBuffer->UnmapMemory();
	}

	int PhongEffect::FragmentUniformBuffer::GetSize()
	{
		return lights.size() * sizeof(Vulkan::Light) + sizeof(constants);
	}
}