#include "WaterRenderer.h"
#include "Camera.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/StaticModel.h"
#include "vulkan/Handles/Texture.h"
#include "vulkan/Handles/CommandBuffer.h"
#include "vulkan/Handles/DescriptorSet.h"
#include "vulkan/Renderer.h"
#include "vulkan/RenderTarget.h"

WaterRenderer::WaterRenderer(Vulkan::Renderer* renderer, Vulkan::ModelLoader* modelLoader, Vulkan::TextureLoader* textureLoader)
{
	mGridModel = modelLoader->LoadGrid(renderer->GetDevice(), 2000.0f, 80);

	mWaterEffect.Init(renderer);

	mReflectionRenderTarget = new Vulkan::RenderTarget(renderer->GetDevice(), renderer->GetCommandPool(), renderer->GetWindowWidth(), renderer->GetWindowHeight());
	mRefractionRenderTarget = new Vulkan::RenderTarget(renderer->GetDevice(), renderer->GetCommandPool(), renderer->GetWindowWidth(), renderer->GetWindowHeight());
	dudvTexture = textureLoader->LoadTexture("data/textures/water_dudv.png");

	mWaterEffect.mDescriptorSet0->BindUniformBuffer(0, &mWaterEffect.per_frame_vs.GetDescriptor());
	mWaterEffect.mDescriptorSet0->BindCombinedImage(1, mReflectionRenderTarget->GetImage(), mReflectionRenderTarget->GetSampler());
	mWaterEffect.mDescriptorSet0->BindCombinedImage(2, mRefractionRenderTarget->GetImage(), mRefractionRenderTarget->GetSampler());
	mWaterEffect.mDescriptorSet0->BindCombinedImage(3, &dudvTexture->GetTextureDescriptorInfo());
	mWaterEffect.mDescriptorSet0->UpdateDescriptorSets();
}

WaterRenderer::~WaterRenderer()
{

}

void WaterRenderer::Render(Vulkan::Renderer* renderer, Vulkan::CommandBuffer* commandBuffer)
{
	commandBuffer->CmdBindPipeline(mWaterEffect.GetPipeline());

	VkDescriptorSet descriptorSets[1] = { mWaterEffect.mDescriptorSet0->descriptorSet };
	commandBuffer->CmdBindDescriptorSet(&mWaterEffect, 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

	commandBuffer->CmdBindVertexBuffer(0, 1, &mGridModel->mMeshes[0]->vertices.buffer);
	commandBuffer->CmdBindIndexBuffer(mGridModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

	// Push the world matrix constant
	Vulkan::WaterEffect::PushConstantBlock pushConstantBlock;
	pushConstantBlock.world = glm::mat4();

	pushConstantBlock.world = glm::translate(glm::mat4(), vec3(123000.0f, 0.0f, 106000.0f));
	pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
	pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
	pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

	commandBuffer->CmdPushConstants(&mWaterEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
	commandBuffer->CmdDrawIndexed(mGridModel->GetNumIndices(), 1, 0, 0, 0);
}

void WaterRenderer::Update(Vulkan::Renderer* renderer, Vulkan::Camera* camera)
{
	mWaterEffect.per_frame_vs.data.projection= camera->GetProjection();
	mWaterEffect.per_frame_vs.data.view = camera->GetView();
	mWaterEffect.per_frame_vs.data.eyePos = camera->GetPosition();
	mWaterEffect.per_frame_vs.data.moveFactor += 0.0015;

	mWaterEffect.UpdateMemory(renderer->GetDevice());
}

Vulkan::RenderTarget* WaterRenderer::GetReflectionRenderTarget()
{
	return mReflectionRenderTarget;
}

Vulkan::RenderTarget* WaterRenderer::GetRefractionRenderTarget()
{
	return mRefractionRenderTarget;
}