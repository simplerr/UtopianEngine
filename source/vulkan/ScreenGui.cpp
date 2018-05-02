#include <vector>
#include "vulkan/ScreenGui.h"
#include "vulkan/Renderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Utopian::Vk
{
	ScreenGui::ScreenGui(Renderer* renderer)
	{
		mEffect.Init(renderer);

		mRenderer = renderer;
	}

	void ScreenGui::Render(Renderer* renderer, CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdBindPipeline(mEffect.GetPipeline(0));

		for (uint32_t layer = NUM_MAX_LAYERS; layer > 0u; layer--)
		{
			for (int i = 0; i < mQuadList.size(); i++)
			{
				if (mQuadList[i]->visible == false || mQuadList[i]->layer != (layer - 1))
					continue;

				ScreenQuadEffect::PushConstantBlock pushConstantBlock;

				float horizontalRatio = (float)mQuadList[i]->width / renderer->GetWindowWidth();
				float verticalRatio = (float)mQuadList[i]->height / renderer->GetWindowHeight();

				float offsetX = (float)(mQuadList[i]->left + mQuadList[i]->width / 2.0f) / renderer->GetWindowWidth();
				float offsetY = (float)(mQuadList[i]->top + mQuadList[i]->height / 2.0f) / renderer->GetWindowHeight();
				pushConstantBlock.world = glm::mat4();
				pushConstantBlock.world = glm::translate(pushConstantBlock.world, glm::vec3(offsetX * 2.0f - 1.0f, offsetY * 2.0f - 1.0, 0));
				pushConstantBlock.world = glm::scale(pushConstantBlock.world, glm::vec3(horizontalRatio, verticalRatio, 0));

				commandBuffer->CmdPushConstants(&mEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(ScreenQuadEffect::PushConstantBlock), &pushConstantBlock);
				VkDescriptorSet descriptorSets[1] = { mQuadList[i]->descriptorSet->descriptorSet };
				commandBuffer->CmdBindDescriptorSet(&mEffect, 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				renderer->DrawScreenQuad(commandBuffer);
			}
		}
	}

	SharedPtr<ScreenQuad> ScreenGui::AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler, uint32_t layer)
	{
		SharedPtr<ScreenQuad> textureQuad = std::make_shared<ScreenQuad>(left, top, width, height, layer);

		textureQuad->descriptorSet = new Utopian::Vk::DescriptorSet(mRenderer->GetDevice(), mEffect.GetDescriptorSetLayout(0), mEffect.GetDescriptorPool());
		textureQuad->descriptorSet->BindCombinedImage(0, image, sampler);
		textureQuad->descriptorSet->UpdateDescriptorSets();
		mQuadList.push_back(textureQuad);

		return textureQuad;
	}

	SharedPtr<ScreenQuad> ScreenGui::AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture, uint32_t layer)
	{
		SharedPtr<ScreenQuad> textureQuad = std::make_shared<ScreenQuad>(left, top, width, height, layer);

		textureQuad->descriptorSet = new Utopian::Vk::DescriptorSet(mRenderer->GetDevice(), mEffect.GetDescriptorSetLayout(0), mEffect.GetDescriptorPool());
		textureQuad->descriptorSet->BindCombinedImage(0, &texture->GetTextureDescriptorInfo());
		textureQuad->descriptorSet->UpdateDescriptorSets();
		mQuadList.push_back(textureQuad);

		return textureQuad;
	}
}