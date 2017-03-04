#include "vulkan/TextOverlay.h"
#include "vulkan/Device.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/VertexDescription.h"

namespace VulkanLib
{
	TextOverlay::TextOverlay(Renderer* renderer)
	{
		mRenderer = renderer;
		vulkanDevice = renderer->GetDevice();

		// Command buffer
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		// Create a renderpass that loads the current framebuffer content
		// and renders the text as an overlay
		mRenderPass = new RenderPass(renderer->GetDevice(), renderer->GetColorFormat(), renderer->GetDepthFormat());
		mRenderPass->attachments[RenderPassAttachment::COLOR_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		mRenderPass->Create();

		prepareResources();
		preparePipeline();
	}

	TextOverlay::~TextOverlay()
	{
		// Free up all Vulkan resources requested by the text overlay
		vkDestroySampler(vulkanDevice->GetVkDevice(), sampler, nullptr);
		vkDestroyImage(vulkanDevice->GetVkDevice(), image, nullptr);
		vkDestroyImageView(vulkanDevice->GetVkDevice(), view, nullptr);
		vkDestroyBuffer(vulkanDevice->GetVkDevice(), mBuffer, nullptr);
		vkFreeMemory(vulkanDevice->GetVkDevice(), mMemory, nullptr);
		vkFreeMemory(vulkanDevice->GetVkDevice(), imageMemory, nullptr);

		delete mRenderPass;
		delete mPipelineLayout;
		delete mPipeline;
		// TODO: Free the texture
	}

	// Prepare all vulkan resources required to render the font
	// The text overlay uses separate resources for descriptors (pool, sets, layouts), pipelines and command buffers
	void TextOverlay::prepareResources()
	{
		static unsigned char font24pixels[STB_FONT_HEIGHT][STB_FONT_WIDTH];
		STB_FONT_NAME(stbFontData, font24pixels, STB_FONT_HEIGHT);

		// Vertex buffer
		VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
		mRenderer->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, nullptr, &mBuffer, &mMemory);

		mTexture = mRenderer->mTextureLoader->LoadTexture((void*)font24pixels, VK_FORMAT_R8_UNORM, STB_FONT_WIDTH, STB_FONT_HEIGHT, STB_FONT_WIDTH * STB_FONT_HEIGHT);

		// NOTE: Uses the descriptor set layout for the texture from the Renderer
		// If more descriptors where to be needed they should be added as a separate descriptor set
		// This is cheating a bit since the shader don't use more than a sampler
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.push_back(mRenderer->GetTextureDescriptorSetLayout()->GetVkHandle());
		mPipelineLayout = new VulkanLib::PipelineLayout(mRenderer->GetDevice(), descriptorSetLayouts, nullptr);
	}

	// Prepare a separate pipeline for the font rendering decoupled from the main application
	void TextOverlay::preparePipeline()
	{
		mVertexDescription = new VertexDescription();
		mVertexDescription->AddBinding(0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX);					
		mVertexDescription->AddBinding(1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX);				
		mVertexDescription->AddAttribute(0, Vec2Attribute());	
		mVertexDescription->AddAttribute(0, Vec2Attribute());

		VulkanLib::Shader* shader = mRenderer->mShaderManager->CreateShader("data/shaders/textoverlay/text.vert.spv", "data/shaders/textoverlay/text.frag.spv");
		mPipeline = new VulkanLib::Pipeline(mRenderer->GetDevice(), mPipelineLayout, mRenderer->GetRenderPass(), mVertexDescription, shader);
		
        // Why triangle strip?
		mPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

		// Enable blending
		mPipeline->mBlendAttachmentState.blendEnable = VK_TRUE;
		mPipeline->mBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mPipeline->mBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mPipeline->mBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		mPipeline->mBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mPipeline->mBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mPipeline->mBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		mPipeline->mBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		mPipeline->Create();
	}

	// Map buffer 
	void TextOverlay::beginTextUpdate()
	{
		VulkanDebug::ErrorCheck(vkMapMemory(vulkanDevice->GetVkDevice(), mMemory, 0, VK_WHOLE_SIZE, 0, (void **)&mapped));
		numLetters = 0;
	}

	// Add text to the current buffer
	// todo : drop shadow? color attribute?
	void TextOverlay::addText(std::string text, float x, float y, TextAlign align)
	{
		assert(mapped != nullptr);

		float fbW = (float)mRenderer->GetWindowWidth();
		float fbH = (float)mRenderer->GetWindowHeight();
		
		const float charW = 1.5f / fbW;
		const float charH = 1.5f / fbH;

		x = (x / fbW * 2.0f) - 1.0f;
		y = (y / fbH * 2.0f) - 1.0f;

		// Calculate text width
		float textWidth = 0;
		for (auto letter : text)
		{
			stb_fontchar *charData = &stbFontData[(uint32_t)letter - STB_FIRST_CHAR];
			textWidth += charData->advance * charW;
		}

		switch (align)
		{
		case alignRight:
			x -= textWidth;
			break;
		case alignCenter:
			x -= textWidth / 2.0f;
			break;
		}

		// Generate a uv mapped quad per char in the new text
		for (auto letter : text)
		{
			stb_fontchar *charData = &stbFontData[(uint32_t)letter - STB_FIRST_CHAR];


			mapped->x = (x + (float)charData->x1 * charW);
			mapped->y = (y + (float)charData->y0 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (x + (float)charData->x0 * charW);
			mapped->y = (y + (float)charData->y0 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t0;
			mapped++;


			mapped->x = (x + (float)charData->x1 * charW);
			mapped->y = (y + (float)charData->y1 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t1;
			mapped++;

			mapped->x = (x + (float)charData->x0 * charW);
			mapped->y = (y + (float)charData->y1 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t1;
			mapped++;

			x += charData->advance * charW;

			numLetters++;
		}
	}

	// Unmap buffer and update command buffers
	void TextOverlay::endTextUpdate()
	{
		vkUnmapMemory(vulkanDevice->GetVkDevice(), mMemory);
		mapped = nullptr;
		updateCommandBuffers();
	}

	// Needs to be called by the application
	void TextOverlay::updateCommandBuffers()
	{
		mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());

		mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

		// TODO: This is currently done in the primary command buffer in Renderer
		mCommandBuffer->CmdBindPipeline(mPipeline);

		vkCmdBindDescriptorSets(mCommandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 1, &mTexture->descriptorSet->descriptorSet, 0, NULL);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(mCommandBuffer->GetVkHandle(), 0, 1, &mBuffer, &offsets);
		vkCmdBindVertexBuffers(mCommandBuffer->GetVkHandle(), 1, 1, &mBuffer, &offsets);

		mCommandBuffer->CmdBindVertexBuffer(0, 1, &mBuffer);
		mCommandBuffer->CmdBindVertexBuffer(1, 1, &mBuffer);
		for (uint32_t j = 0; j < numLetters; j++)
		{
			vkCmdDraw(mCommandBuffer->GetVkHandle(), 4, 1, j * 4, 0);
		}

		mCommandBuffer->End();
	}

	// Submit the text command buffers to a queue
	// Does a queue wait idle
	void TextOverlay::submit(VkQueue queue, uint32_t bufferindex)
	{
		if (!visible)
		{
			return;
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];

		VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(queue));
	}
}