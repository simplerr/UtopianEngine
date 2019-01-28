#include <vector>
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Vertex.h"
#include "vulkan/EffectManager.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Utopian
{
	ScreenQuadUi& gScreenQuadUi()
	{
		return ScreenQuadUi::Instance();
	}

	ScreenQuadUi::ScreenQuadUi(Vk::VulkanApp* vulkanApp)
	{
		mVulkanApp = vulkanApp;

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(vulkanApp->GetDevice(),
			vulkanApp->GetRenderPass(),
			"data/shaders/screenquad/screenquad.vert",
			"data/shaders/screenquad/screenquad.frag");
		mEffect->CreatePipeline();

		mCommandBuffer = new Vk::CommandBuffer(vulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		mVulkanApp->AddSecondaryCommandBuffer(mCommandBuffer);

		mDescriptorPool = std::make_shared<Vk::DescriptorPool>(vulkanApp->GetDevice());
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20);
		mDescriptorPool->Create();
	
		CreateQuadBuffers();
	}

	void ScreenQuadUi::CreateQuadBuffers()
	{
		std::vector<Vk::ScreenQuadVertex> vertices =
		{
			{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) }
		};

		mScreenQuad.vertexBuffer = new Utopian::Vk::Buffer(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertices.size() * sizeof(Vk::ScreenQuadVertex), vertices.data());

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		mScreenQuad.indexBuffer = new Utopian::Vk::Buffer(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.size() * sizeof(uint32_t), indices.data());
	}

	void ScreenQuadUi::Render(Vk::VulkanApp* vulkanApp)
	{
		mCommandBuffer->Begin(vulkanApp->GetRenderPass(), vulkanApp->GetCurrentFrameBuffer());
		mCommandBuffer->CmdSetViewPort(vulkanApp->GetWindowWidth(), vulkanApp->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(vulkanApp->GetWindowWidth(), vulkanApp->GetWindowHeight());

		mCommandBuffer->CmdBindPipeline(mEffect->GetPipeline());

		for (uint32_t layer = NUM_MAX_LAYERS; layer > 0u; layer--)
		{
			for (int i = 0; i < mQuadList.size(); i++)
			{
				if (mQuadList[i]->visible == false || mQuadList[i]->layer != (layer - 1))
					continue;

				PushConstantBlock pushConstantBlock;

				float horizontalRatio = (float)mQuadList[i]->width / vulkanApp->GetWindowWidth();
				float verticalRatio = (float)mQuadList[i]->height / vulkanApp->GetWindowHeight();

				float offsetX = (float)(mQuadList[i]->left + mQuadList[i]->width / 2.0f) / vulkanApp->GetWindowWidth();
				float offsetY = (float)(mQuadList[i]->top + mQuadList[i]->height / 2.0f) / vulkanApp->GetWindowHeight();
				pushConstantBlock.world = glm::mat4();
				pushConstantBlock.world = glm::translate(pushConstantBlock.world, glm::vec3(offsetX * 2.0f - 1.0f, offsetY * 2.0f - 1.0, 0));
				pushConstantBlock.world = glm::scale(pushConstantBlock.world, glm::vec3(horizontalRatio, verticalRatio, 0));

				mCommandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstantBlock), &pushConstantBlock);
				VkDescriptorSet descriptorSets[1] = { mQuadList[i]->descriptorSet->GetVkHandle() };
				mCommandBuffer->CmdBindDescriptorSet(mEffect->GetPipelineInterface(), 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);

				mCommandBuffer->CmdBindVertexBuffer(0, 1, mScreenQuad.vertexBuffer);
				mCommandBuffer->CmdBindIndexBuffer(mScreenQuad.indexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
				mCommandBuffer->CmdDrawIndexed(6, 1, 0, 0, 0);
			}
		}

		mCommandBuffer->End();
	}

	SharedPtr<ScreenQuad> ScreenQuadUi::AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler, uint32_t layer)
	{
		SharedPtr<ScreenQuad> textureQuad = std::make_shared<ScreenQuad>(left, top, width, height, layer);

		textureQuad->descriptorSet = new Utopian::Vk::DescriptorSet(mVulkanApp->GetDevice(), mEffect.get(), 0, mDescriptorPool.get());
		textureQuad->descriptorSet->BindCombinedImage(0, image, sampler);
		textureQuad->descriptorSet->UpdateDescriptorSets();
		mQuadList.push_back(textureQuad);

		return textureQuad;
	}

	SharedPtr<ScreenQuad> ScreenQuadUi::AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, VkImageView imageView, Utopian::Vk::Sampler* sampler, uint32_t layer)
	{
		SharedPtr<ScreenQuad> textureQuad = std::make_shared<ScreenQuad>(left, top, width, height, layer);

		textureQuad->descriptorSet = new Utopian::Vk::DescriptorSet(mVulkanApp->GetDevice(), mEffect.get(), 0, mDescriptorPool.get());
		textureQuad->descriptorSet->BindCombinedImage(0, imageView, sampler);
		textureQuad->descriptorSet->UpdateDescriptorSets();
		mQuadList.push_back(textureQuad);

		return textureQuad;
	}

	SharedPtr<ScreenQuad> ScreenQuadUi::AddQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture, uint32_t layer)
	{
		SharedPtr<ScreenQuad> textureQuad = std::make_shared<ScreenQuad>(left, top, width, height, layer);

		textureQuad->descriptorSet = new Utopian::Vk::DescriptorSet(mVulkanApp->GetDevice(), mEffect.get(), 0, mDescriptorPool.get());
		textureQuad->descriptorSet->BindCombinedImage(0, texture->GetTextureDescriptorInfo());
		textureQuad->descriptorSet->UpdateDescriptorSets();
		mQuadList.push_back(textureQuad);

		return textureQuad;
	}

	void ScreenQuadUi::ToggleVisible(uint32_t layer)
	{
		for (int i = 0; i < mQuadList.size(); i++)
		{
			if (mQuadList[i]->layer == layer)
				mQuadList[i]->visible = !mQuadList[i]->visible;
		}
	}

	void ScreenQuadUi::SetVisible(uint32_t layer, bool visible)
	{
		for (int i = 0; i < mQuadList.size(); i++)
		{
			if (mQuadList[i]->layer == layer)
				mQuadList[i]->visible = visible;
		}
	}
}