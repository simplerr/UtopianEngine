#include "vulkan/TextOverlay.h"
#include "vulkan/Device.h"
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"

namespace VulkanLib
{
	TextOverlay::TextOverlay(Renderer* renderer, Device *vulkanDevice, VkQueue queue, std::vector<VkFramebuffer> &framebuffers, VkFormat colorformat,
		VkFormat depthformat,
		uint32_t *framebufferwidth,
		uint32_t *framebufferheight,
		std::vector<VkPipelineShaderStageCreateInfo> shaderstages)
	{
		mRenderer = renderer;
		
		this->vulkanDevice = vulkanDevice;
		this->queue = queue;
		this->colorFormat = colorformat;
		this->depthFormat = depthformat;

		this->frameBuffers.resize(framebuffers.size());
		for (uint32_t i = 0; i < framebuffers.size(); i++)
		{
			this->frameBuffers[i] = &framebuffers[i];
		}

		this->shaderStages = shaderstages;

		this->frameBufferWidth = framebufferwidth;
		this->frameBufferHeight = framebufferheight;

		cmdBuffers.resize(framebuffers.size());
		prepareResources();
		prepareRenderPass();
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
		vkDestroyDescriptorSetLayout(vulkanDevice->GetVkDevice(), descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(vulkanDevice->GetVkDevice(), descriptorPool, nullptr);
		vkDestroyPipelineLayout(vulkanDevice->GetVkDevice(), pipelineLayout, nullptr);
		vkDestroyPipelineCache(vulkanDevice->GetVkDevice(), pipelineCache, nullptr);
		vkDestroyPipeline(vulkanDevice->GetVkDevice(), pipeline, nullptr);
		vkDestroyRenderPass(vulkanDevice->GetVkDevice(), renderPass, nullptr);
		vkDestroyCommandPool(vulkanDevice->GetVkDevice(), commandPool, nullptr);
	}

	// Prepare all vulkan resources required to render the font
	// The text overlay uses separate resources for descriptors (pool, sets, layouts), pipelines and command buffers
	void TextOverlay::prepareResources()
	{
		static unsigned char font24pixels[STB_FONT_HEIGHT][STB_FONT_WIDTH];
		STB_FONT_NAME(stbFontData, font24pixels, STB_FONT_HEIGHT);

		// Command buffer
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		// Vertex buffer
		VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
		mRenderer->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, nullptr, &mBuffer, mMemory);

		// TODO: Send VK_IMAGE_LAYOUT_UNDEFINED 
		mTextureLoader->CreateImage(STB_FONT_WIDTH,
									STB_FONT_HEIGHT,
									VK_FORMAT_R8_UNORM,
									VK_IMAGE_TILING_OPTIMAL, 
									VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									&image,
									&imageMemory);

		// Staging

		struct {
			VkDeviceMemory memory;
			VkBuffer buffer;
		} stagingBuffer;

		VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
		bufferCreateInfo.size = allocInfo.allocationSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanDebug::ErrorCheck(vkCreateBuffer(vulkanDevice->GetVkDevice(), &bufferCreateInfo, nullptr, &stagingBuffer.buffer));

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(vulkanDevice->GetVkDevice(), stagingBuffer.buffer, &memReqs);

		allocInfo.allocationSize = memReqs.size;
		// Get memory type index for a host visible buffer
		allocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VulkanDebug::ErrorCheck(vkAllocateMemory(vulkanDevice->GetVkDevice(), &allocInfo, nullptr, &stagingBuffer.memory));
		VulkanDebug::ErrorCheck(vkBindBufferMemory(vulkanDevice->GetVkDevice(), stagingBuffer.buffer, stagingBuffer.memory, 0));

		uint8_t *data;
		VulkanDebug::ErrorCheck(vkMapMemory(vulkanDevice->GetVkDevice(), stagingBuffer.memory, 0, allocInfo.allocationSize, 0, (void **)&data));
		// Size of the font texture is WIDTH * HEIGHT * 1 byte (only one channel)
		memcpy(data, &font24pixels[0][0], STB_FONT_WIDTH * STB_FONT_HEIGHT);
		vkUnmapMemory(vulkanDevice->GetVkDevice(), stagingBuffer.memory);

		// Copy to image

		VkCommandBuffer copyCmd;
		cmdBufAllocateInfo.commandBufferCount = 1;
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(vulkanDevice->GetVkDevice(), &cmdBufAllocateInfo, &copyCmd));

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));

		// Prepare for transfer
		vks::tools::setImageLayout(
			copyCmd,
			image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = STB_FONT_WIDTH;
		bufferCopyRegion.imageExtent.height = STB_FONT_HEIGHT;
		bufferCopyRegion.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer.buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);

		// Prepare for shader read
		vks::tools::setImageLayout(
			copyCmd,
			image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VulkanDebug::ErrorCheck(vkEndCommandBuffer(copyCmd));

		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &copyCmd;

		VulkanDebug::ErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VulkanDebug::ErrorCheck(vkQueueWaitIdle(queue));

		vkFreeCommandBuffers(vulkanDevice->GetVkDevice(), commandPool, 1, &copyCmd);
		vkFreeMemory(vulkanDevice->GetVkDevice(), stagingBuffer.memory, nullptr);
		vkDestroyBuffer(vulkanDevice->GetVkDevice(), stagingBuffer.buffer, nullptr);

		VkImageViewCreateInfo imageViewInfo = vks::initializers::imageViewCreateInfo();
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = imageInfo.format;
		imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,	VK_COMPONENT_SWIZZLE_A };
		imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		VulkanDebug::ErrorCheck(vkCreateImageView(vulkanDevice->GetVkDevice(), &imageViewInfo, nullptr, &view));

		// Sampler
		VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VulkanDebug::ErrorCheck(vkCreateSampler(vulkanDevice->GetVkDevice(), &samplerInfo, nullptr, &sampler));

		// Descriptor
		// Font uses a separate descriptor pool
		std::array<VkDescriptorPoolSize, 1> poolSizes;
		poolSizes[0] = vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);

		VkDescriptorPoolCreateInfo descriptorPoolInfo =
			vks::initializers::descriptorPoolCreateInfo(
				static_cast<uint32_t>(poolSizes.size()),
				poolSizes.data(),
				1);

		VulkanDebug::ErrorCheck(vkCreateDescriptorPool(vulkanDevice->GetVkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool));

		// Descriptor set layout
		std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings;
		setLayoutBindings[0] = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo =
			vks::initializers::descriptorSetLayoutCreateInfo(
				setLayoutBindings.data(),
				static_cast<uint32_t>(setLayoutBindings.size()));
		VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(vulkanDevice->GetVkDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

		// Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo =
			vks::initializers::pipelineLayoutCreateInfo(
				&descriptorSetLayout,
				1);
		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(vulkanDevice->GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

		// Descriptor set
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo =
			vks::initializers::descriptorSetAllocateInfo(
				descriptorPool,
				&descriptorSetLayout,
				1);

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(vulkanDevice->GetVkDevice(), &descriptorSetAllocInfo, &descriptorSet));

		VkDescriptorImageInfo texDescriptor =
			vks::initializers::descriptorImageInfo(
				sampler,
				view,
				VK_IMAGE_LAYOUT_GENERAL);

		std::array<VkWriteDescriptorSet, 1> writeDescriptorSets;
		writeDescriptorSets[0] = vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptor);
		vkUpdateDescriptorSets(vulkanDevice->GetVkDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VulkanDebug::ErrorCheck(vkCreatePipelineCache(vulkanDevice->GetVkDevice(), &pipelineCacheCreateInfo, nullptr, &pipelineCache));
	}

	// Prepare a separate pipeline for the font rendering decoupled from the main application
	void TextOverlay::preparePipeline()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
				0,
				VK_FALSE);

		VkPipelineRasterizationStateCreateInfo rasterizationState =
			vks::initializers::pipelineRasterizationStateCreateInfo(
				VK_POLYGON_MODE_FILL,
				VK_CULL_MODE_BACK_BIT,
				VK_FRONT_FACE_CLOCKWISE,
				0);

		// Enable blending
		VkPipelineColorBlendAttachmentState blendAttachmentState =
			vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_TRUE);

		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendState =
			vks::initializers::pipelineColorBlendStateCreateInfo(
				1,
				&blendAttachmentState);

		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			vks::initializers::pipelineDepthStencilStateCreateInfo(
				VK_TRUE,
				VK_TRUE,
				VK_COMPARE_OP_LESS_OR_EQUAL);

		VkPipelineViewportStateCreateInfo viewportState =
			vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

		VkPipelineMultisampleStateCreateInfo multisampleState =
			vks::initializers::pipelineMultisampleStateCreateInfo(
				VK_SAMPLE_COUNT_1_BIT,
				0);

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState =
			vks::initializers::pipelineDynamicStateCreateInfo(
				dynamicStateEnables.data(),
				static_cast<uint32_t>(dynamicStateEnables.size()),
				0);

		std::array<VkVertexInputBindingDescription, 2> vertexBindings = {};
		vertexBindings[0] = vks::initializers::vertexInputBindingDescription(0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX);
		vertexBindings[1] = vks::initializers::vertexInputBindingDescription(1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX);

		std::array<VkVertexInputAttributeDescription, 2> vertexAttribs = {};
		// Position
		vertexAttribs[0] = vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
		// UV
		vertexAttribs[1] = vks::initializers::vertexInputAttributeDescription(1, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2));

		VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
		inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
		inputState.pVertexBindingDescriptions = vertexBindings.data();
		inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
		inputState.pVertexAttributeDescriptions = vertexAttribs.data();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vks::initializers::pipelineCreateInfo(
				pipelineLayout,
				renderPass,
				0);

		pipelineCreateInfo.pVertexInputState = &inputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(vulkanDevice->GetVkDevice(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
	}

	// Prepare a separate render pass for rendering the text as an overlay
	void TextOverlay::prepareRenderPass()
	{
		VkAttachmentDescription attachments[2] = {};

		// Color attachment
		attachments[0].format = colorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		// Don't clear the framebuffer (like the renderpass from the example does)
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Depth attachment
		attachments[1].format = depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Use subpass dependencies for image layout transitions
		VkSubpassDependency subpassDependencies[2] = {};

		// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = 0;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Transition from initial to final
		subpassDependencies[1].srcSubpass = 0;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.flags = 0;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = NULL;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pResolveAttachments = NULL;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = NULL;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = NULL;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = subpassDependencies;

		VulkanDebug::ErrorCheck(vkCreateRenderPass(vulkanDevice->GetVkDevice(), &renderPassInfo, nullptr, &renderPass));
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

		const float charW = 1.5f / *frameBufferWidth;
		const float charH = 1.5f / *frameBufferHeight;

		float fbW = (float)*frameBufferWidth;
		float fbH = (float)*frameBufferHeight;
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

			mapped->x = (x + (float)charData->x0 * charW);
			mapped->y = (y + (float)charData->y0 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (x + (float)charData->x1 * charW);
			mapped->y = (y + (float)charData->y0 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = (x + (float)charData->x0 * charW);
			mapped->y = (y + (float)charData->y1 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t1;
			mapped++;

			mapped->x = (x + (float)charData->x1 * charW);
			mapped->y = (y + (float)charData->y1 * charH);
			mapped->z = charData->s1;
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
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.extent.width = *frameBufferWidth;
		renderPassBeginInfo.renderArea.extent.height = *frameBufferHeight;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (int32_t i = 0; i < cmdBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = *frameBuffers[i];

			VulkanDebug::ErrorCheck(vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport((float)*frameBufferWidth, (float)*frameBufferHeight, 0.0f, 1.0f);
			vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(*frameBufferWidth, *frameBufferHeight, 0, 0);
			vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);

			vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

			VkDeviceSize offsets = 0;
			vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, &mBuffer, &offsets);
			vkCmdBindVertexBuffers(cmdBuffers[i], 1, 1, &mBuffer, &offsets);
			for (uint32_t j = 0; j < numLetters; j++)
			{
				vkCmdDraw(cmdBuffers[i], 4, 1, j * 4, 0);
			}


			vkCmdEndRenderPass(cmdBuffers[i]);

			VulkanDebug::ErrorCheck(vkEndCommandBuffer(cmdBuffers[i]));
		}
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