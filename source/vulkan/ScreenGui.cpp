#include <vector>
#include "vulkan/ScreenGui.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Vulkan
{
	ScreenGui::ScreenGui(Renderer* renderer)
	{
		std::vector<Vertex> vertices = 
		{
			{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }
		};

		mVertexBuffer = new Vulkan::Buffer(renderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertices.size() * sizeof(Vertex), vertices.data());

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		mIndexBuffer = new Vulkan::Buffer(renderer->GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.size() * sizeof(uint32_t), indices.data());

		mEffect.Init(renderer);

		mRenderer = renderer;
	}

	void ScreenGui::Render(Renderer* renderer, CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdBindPipeline(mEffect.GetPipeline());
		commandBuffer->CmdBindVertexBuffer(0, 1, mVertexBuffer);
		commandBuffer->CmdBindIndexBuffer(mIndexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		for (int i = 0; i < mQuadList.size(); i++)
		{
			ScreenQuadEffect::PushConstantBlock pushConstantBlock;

			float horizontalRatio = (float)mQuadList[i].width / renderer->GetWindowWidth();
			float verticalRatio = (float)mQuadList[i].height / renderer->GetWindowHeight();

			float offsetX = (float)(mQuadList[i].left + mQuadList[i].width / 2.0f) / renderer->GetWindowWidth();
			float offsetY = (float)(mQuadList[i].top + mQuadList[i].height / 2.0f) / renderer->GetWindowHeight();
			pushConstantBlock.world = glm::mat4();
			pushConstantBlock.world = glm::translate(pushConstantBlock.world, glm::vec3(offsetX * 2.0f - 1.0f, offsetY * 2.0f - 1.0, 0));
			pushConstantBlock.world = glm::scale(pushConstantBlock.world, glm::vec3(horizontalRatio, verticalRatio, 0));

			commandBuffer->CmdPushConstants(&mEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(ScreenQuadEffect::PushConstantBlock), &pushConstantBlock);
			VkDescriptorSet descriptorSets[1] = { mQuadList[i].descriptorSet->descriptorSet };
			commandBuffer->CmdBindDescriptorSet(&mEffect, 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			commandBuffer->CmdDrawIndexed(6, 1, 0, 0, 0); // NOTE: TODO: Hard coded to 6 indices
		}
	}

	void ScreenGui::AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Vulkan::Image* image, Vulkan::Sampler* sampler)
	{
		TextureQuad textureQuad = TextureQuad(left, top, width, height);

		textureQuad.descriptorSet = new Vulkan::DescriptorSet(mRenderer->GetDevice(), mEffect.GetDescriptorSetLayout(0), mEffect.GetDescriptorPool());
		textureQuad.descriptorSet->BindCombinedImage(0, image, sampler);
		textureQuad.descriptorSet->UpdateDescriptorSets();
		mQuadList.push_back(textureQuad);
	}
}