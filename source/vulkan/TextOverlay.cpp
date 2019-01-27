#include <sstream>
#include <iomanip>
#include "vulkan/TextOverlay.h"
#include "vulkan/handles/Device.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/PipelineLegacy.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/VertexDescription.h"

namespace Utopian::Vk
{
	TextOverlay::TextOverlay(VulkanApp* vulkanApp)
	{
		mVulkanApp = vulkanApp;
		vulkanDevice = vulkanApp->GetDevice();

		mCommandBuffer = new Vk::CommandBuffer(vulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		mVulkanApp->AddSecondaryCommandBuffer(mCommandBuffer);

		// Create a renderpass that loads the current framebuffer content
		// and renders the text as an overlay
		mRenderPass = new RenderPass(vulkanApp->GetDevice(), vulkanApp->GetColorFormat(), vulkanApp->GetDepthFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, false);
		mRenderPass->attachments[RenderPassAttachment::COLOR_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		mRenderPass->Create();

		// Vertex buffer
		VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
		mVertexBuffer = new Buffer(vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, nullptr);

		// Load the texture
		static unsigned char font24pixels[STB_FONT_HEIGHT][STB_FONT_WIDTH];
		STB_FONT_NAME(stbFontData, font24pixels, STB_FONT_HEIGHT);
		mTexture = gTextureLoader().CreateTexture((void*)font24pixels, VK_FORMAT_R8_UNORM, STB_FONT_WIDTH, STB_FONT_HEIGHT, 1, sizeof(unsigned char));
		// Note: Texture descriptor set layout moved from VulkanApp, fix when updating TextOverlay to use new Effect
		//mTexture->CreateDescriptorSet(mVulkanApp->GetDevice(), mVulkanApp->GetTextureDescriptorSetLayout(), mVulkanApp->GetDescriptorPool());

		// NOTE: Uses the descriptor set layout for the texture from the VulkanApp
		// If more descriptors where to be needed they should be added as a separate descriptor set
		// This is cheating a bit since the shader don't use more than a sampler
		mPipelineLayout = new Utopian::Vk::PipelineLayout(mVulkanApp->GetDevice());
		// Note: Texture descriptor set layout moved from VulkanApp, fix when updating TextOverlay to use new Effect
		//mPipelineLayout->AddDescriptorSetLayout(mVulkanApp->GetTextureDescriptorSetLayout());
		mPipelineLayout->Create();

		mVertexDescription = new VertexDescription();
		mVertexDescription->AddBinding(0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX);					
		mVertexDescription->AddBinding(1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX);				
		mVertexDescription->AddAttribute(0, Vec2Attribute());	
		mVertexDescription->AddAttribute(0, Vec2Attribute());

		// Create the pipeline
		Utopian::Vk::Shader* shader = gShaderFactory().CreateShader("data/shaders/textoverlay/text.vert.spv", "data/shaders/textoverlay/text.frag.spv");
		mPipeline = new Utopian::Vk::PipelineLegacy(mVulkanApp->GetDevice(), mPipelineLayout, mVulkanApp->GetRenderPass(), mVertexDescription, shader);
		
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

		ToggleVisible();
	}

	TextOverlay::~TextOverlay()
	{
		// Free up all Vulkan resources requested by the text overlay
		delete mVertexBuffer;
		delete mRenderPass;
		delete mPipelineLayout;
		delete mPipeline;
		delete mTexture;
	}

	// Map buffer 
	void TextOverlay::BeginTextUpdate()
	{
		mVertexBuffer->MapMemory(0, VK_WHOLE_SIZE, 0, (void **)&mapped);
		numLetters = 0;
	}

	// Add text to the current buffer
	// todo : drop shadow? color attribute?
	void TextOverlay::AddText(std::string text, float x, float y, TextAlign align)
	{
		assert(mapped != nullptr);

		float fbW = (float)mVulkanApp->GetWindowWidth();
		float fbH = (float)mVulkanApp->GetWindowHeight();
		
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
		case ALIGN_RIGHT:
			x -= textWidth;
			break;
		case ALIGN_CENTER:
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

	void TextOverlay::AddText(std::string text, glm::vec3 vec, float x, float y, TextAlign align)
	{
		AddText(text, x, y, align);

		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << std::showpos;
		ss << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z;
		AddText(ss.str(), x, y + y_offset, align);
	}

	void TextOverlay::AddText(std::string text, glm::vec4 vec, float x, float y, TextAlign align)
	{
		AddText(text, x, y, align);

		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << std::showpos;
		ss << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w;
		AddText(ss.str(), x, y + y_offset, align);
	}

	void TextOverlay::AddText(std::string text, glm::mat4 mat, float x, float y, TextAlign align)
	{
		AddText(text, x, y, align);

		std::stringstream  ss;
		for (uint32_t i = 0; i < 4; i++)
		{
			ss.str("");
			ss << std::fixed << std::setprecision(2) << std::showpos;
			ss << mat[0][i] << " " << mat[1][i] << " " << mat[2][i] << " " << mat[3][i];
			AddText(ss.str(), x, y + y_offset + (float)i * 20.0f, align);
		}
	}

	// Unmap buffer and update command buffers
	void TextOverlay::EndTextUpdate()
	{
		mVertexBuffer->UnmapMemory();
		mapped = nullptr;
		UpdateCommandBuffers();
	}

	// Needs to be called by the application
	void TextOverlay::UpdateCommandBuffers()
	{
		mCommandBuffer->Begin(mVulkanApp->GetRenderPass(), mVulkanApp->GetCurrentFrameBuffer());

		mCommandBuffer->CmdSetViewPort(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());

		// TODO: This is currently done in the primary command buffer in VulkanApp
		mCommandBuffer->CmdBindPipeline(mPipeline);

		// Todo: Note: Disabled for now. TextOverlay need to have its own DescriptorSet for the texture since it's removed from Texture
		//vkCmdBindDescriptorSets(mCommandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 1, &mTexture->GetDescriptorSet()->descriptorSet, 0, NULL);

		mCommandBuffer->CmdBindVertexBuffer(0, 1, mVertexBuffer);
		mCommandBuffer->CmdBindVertexBuffer(1, 1, mVertexBuffer);
		for (uint32_t j = 0; j < numLetters; j++)
		{
			vkCmdDraw(mCommandBuffer->GetVkHandle(), 4, 1, j * 4, 0);
		}

		mCommandBuffer->End();
	}

	void TextOverlay::ToggleVisible()
	{
		mCommandBuffer->ToggleActive();
		mVisible = !mVisible;
	}

	bool TextOverlay::IsVisible()
	{
		return mVisible;
	}
}