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

namespace Utopian
{
	WaterRenderer::WaterRenderer(Vk::Renderer* renderer, Vk::ModelLoader* modelLoader, Vk::TextureLoader* textureLoader)
	{
		mRenderer = renderer;
		mModelLoader = modelLoader;

		//mGridModel = modelLoader->LoadGrid(renderer->GetDevice(), 2000.0f, 80);

		mWaterEffect.Init(renderer);

		mReflectionRenderTarget = new Vk::RenderTarget(renderer->GetDevice(), renderer->GetCommandPool(), renderer->GetWindowWidth(), renderer->GetWindowHeight());
		mRefractionRenderTarget = new Vk::RenderTarget(renderer->GetDevice(), renderer->GetCommandPool(), renderer->GetWindowWidth(), renderer->GetWindowHeight());
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

	void WaterRenderer::Render(Vk::Renderer* renderer, Vk::CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdBindPipeline(mWaterEffect.GetPipeline());

		VkDescriptorSet descriptorSets[1] = { mWaterEffect.mDescriptorSet0->descriptorSet };
		commandBuffer->CmdBindDescriptorSet(&mWaterEffect, 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

		// Push the world matrix constant
		Vk::WaterEffect::PushConstantBlock pushConstantBlock;
		pushConstantBlock.world = glm::mat4();

		for (uint32_t i = 0; i < mWaterList.size(); i++)
		{
			Utopian::Vk::StaticModel* model = mWaterList[i].gridModel;
			commandBuffer->CmdBindVertexBuffer(0, 1, &model->mMeshes[0]->vertices.buffer);
			commandBuffer->CmdBindIndexBuffer(model->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

			pushConstantBlock.world = glm::translate(glm::mat4(), mWaterList[i].position);
			pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
			pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
			pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

			commandBuffer->CmdPushConstants(&mWaterEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
			commandBuffer->CmdDrawIndexed(model->GetNumIndices(), 1, 0, 0, 0);
		}
	}

	void WaterRenderer::Update(Vk::Renderer* renderer, Camera* camera)
	{
		mWaterEffect.per_frame_vs.data.projection= camera->GetProjection();
		mWaterEffect.per_frame_vs.data.view = camera->GetView();
		mWaterEffect.per_frame_vs.data.eyePos = camera->GetPosition();
		mWaterEffect.per_frame_vs.data.moveFactor += 0.0015;

		mWaterEffect.UpdateMemory(renderer->GetDevice());
	}

	void WaterRenderer::AddWater(glm::vec3 position, uint32_t numCells)
	{
		Water water;
		water.gridModel = mModelLoader->LoadGrid(mRenderer->GetDevice(), CELL_SIZE, numCells);
		water.position = glm::vec3(position.x, 0.0f, position.z); // NOTE: Water can only be placed at y=0
		mWaterList.push_back(water);
	}

	Vk::RenderTarget* WaterRenderer::GetReflectionRenderTarget()
	{
		return mReflectionRenderTarget;
	}

	Vk::RenderTarget* WaterRenderer::GetRefractionRenderTarget()
	{
		return mRefractionRenderTarget;
	}
}