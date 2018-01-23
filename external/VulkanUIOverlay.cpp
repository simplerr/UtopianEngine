/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <algorithm>
#include "VulkanUIOverlay.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/Renderer.h"
#include "vulkan/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/Buffer.h"

namespace Vulkan 
{
	UIOverlay::UIOverlay(uint32_t width, uint32_t height, Vulkan::Renderer* renderer)
	{
		mRenderer = renderer;
		this->width = width;
		this->height = height;

		// Init ImGui
		// Color scheme
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
		io.FontGlobalScale = scale;

		prepareResources();
	}

	/** Free up all Vulkan resources acquired by the UI overlay */
	UIOverlay::~UIOverlay()
	{
		
	}

	/** Prepare all vulkan resources required to render the UI overlay */
	void UIOverlay::prepareResources()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight, pixelSize;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight, &pixelSize);

		// Note: Must be performed before Init()
		mImguiEffect.mTexture = mRenderer->mTextureLoader->CreateTexture(fontData, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, 1, pixelSize);
		mImguiEffect.Init(mRenderer);

		mRenderer->AddScreenQuad(20, 500, 512, 64, mImguiEffect.mTexture);

		//mCommandBuffer = new CommandBuffer(mRenderer->GetDevice(), mRenderer->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		// Is a fence needed?
	}


	/** Update the command buffers to reflect UI changes */
	void UIOverlay::updateCommandBuffers()
	{
		ImGuiIO& io = ImGui::GetIO();

		VkClearValue clearValues[2];
		clearValues[0].color = { 0, 0, 0 };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mImguiEffect.mRenderPass->GetVkHandle();
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 0;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = mRenderer->GetCurrentFrameBuffer(); // TODO: NOTE: Should not be like this

		mCommandBuffer->Begin();
		//mCommandBuffer->CmdBeginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		mCommandBuffer->CmdSetViewPort(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
		mCommandBuffer->CmdSetScissor(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

		mCommandBuffer->CmdBindPipeline(mImguiEffect.GetPipeline());
		VkDescriptorSet descriptorSets[1] = { mImguiEffect.mDescriptorSet0->descriptorSet };
		vkCmdBindDescriptorSets(mCommandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mImguiEffect.GetPipelineLayout(), 0, 1, descriptorSets, 0, NULL);

		mCommandBuffer->CmdBindVertexBuffer(0, 1, &mVertexBuffer);
		mCommandBuffer->CmdBindIndexBuffer(mIndexBuffer.GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

		// UI scale and translate via push constants
		ImguiEffect::PushConstantBlock pushConstBlock;
		pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		pushConstBlock.translate = glm::vec2(-1.0f);
		mCommandBuffer->CmdPushConstants(&mImguiEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstBlock), &pushConstBlock);

		// Render commands
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;
		for (int32_t j = 0; j < imDrawData->CmdListsCount; j++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[j];
			for (int32_t k = 0; k < cmd_list->CmdBuffer.Size; k++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[k];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				mCommandBuffer->CmdSetScissor(scissorRect);
				mCommandBuffer->CmdDrawIndexed(pcmd->ElemCount, 1, 0, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}

		//mCommandBuffer->CmdEndRenderPass();
		mCommandBuffer->End();
	}

	/** Update vertex and index buffer containing the imGui elements when required */
	void UIOverlay::update()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffers = false;

		if (!imDrawData) { return; };

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		// Update buffers only if vertex or index count has been changed compared to current buffer size

		// Vertex buffer
		if ((mVertexBuffer.GetVkBuffer() == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
			mVertexBuffer.UnmapMemory();
			mVertexBuffer.Destroy();
			mVertexBuffer.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
			vertexCount = imDrawData->TotalVtxCount;
			mVertexBuffer.MapMemory(0, VK_WHOLE_SIZE, 0, (void**)&mMappedVertices);
			updateCmdBuffers = true;
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if ((mIndexBuffer.GetVkBuffer() == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
			mIndexBuffer.UnmapMemory();
			mIndexBuffer.Destroy();
			mIndexBuffer.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize);
			indexCount = imDrawData->TotalIdxCount;
			mIndexBuffer.MapMemory(0, VK_WHOLE_SIZE, 0, (void**)&mMappedIndices);
			updateCmdBuffers = true;
		}

		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)mMappedVertices;
		ImDrawIdx* idxDst = (ImDrawIdx*)mMappedIndices;

		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		// Flush to make writes visible to GPU
		mVertexBuffer.Flush();
		mIndexBuffer.Flush();

		if (updateCmdBuffers) {
			updateCommandBuffers();
		}
	}

	void UIOverlay::resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(width), (float)(height));
		this->width = width;
		this->height = height;
		updateCommandBuffers();
	}

	/** Submit the overlay command buffers to a queue */
	void UIOverlay::submit(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo)
	{
		//if (!visible) {
		//	return;
		//}

		//submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];
		//submitInfo.commandBufferCount = 1;

		//VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
	
		//VK_CHECK_RESULT(vkWaitForFences(createInfo.device->logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX));
		//VK_CHECK_RESULT(vkResetFences(createInfo.device->logicalDevice, 1, &fence));
	}

	Vulkan::CommandBuffer* UIOverlay::GetCommandBuffer() const
	{
		return mCommandBuffer;
	}

	bool UIOverlay::header(const char *caption)
	{
		return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
	}

	bool UIOverlay::checkBox(const char *caption, bool *value)
	{
		return ImGui::Checkbox(caption, value);
	}

	bool UIOverlay::checkBox(const char *caption, int32_t *value)
	{
		bool val = (*value == 1);
		bool res = ImGui::Checkbox(caption, &val);
		*value = val;
		return res;
	}

	bool UIOverlay::inputFloat(const char *caption, float *value, float step, uint32_t precision)
	{
		return ImGui::InputFloat(caption, value, step, step * 10.0f, precision);
	}

	bool UIOverlay::sliderFloat(const char* caption, float* value, float min, float max)
	{
		return ImGui::SliderFloat(caption, value, min, max);
	}

	bool UIOverlay::sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max)
	{
		return ImGui::SliderInt(caption, value, min, max);
	}

	bool UIOverlay::comboBox(const char *caption, int32_t *itemindex, std::vector<std::string> items)
	{
		if (items.empty()) {
			return false;
		}
		std::vector<const char*> charitems;
		charitems.reserve(items.size());
		for (size_t i = 0; i < items.size(); i++) {
			charitems.push_back(items[i].c_str());
		}
		uint32_t itemCount = static_cast<uint32_t>(charitems.size());
		return ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
	}

	bool UIOverlay::button(const char *caption)
	{
		return ImGui::Button(caption);
	}

	void UIOverlay::text(const char *formatstr, ...)
	{
		va_list args;
		va_start(args, formatstr);
		ImGui::TextV(formatstr, args);
		va_end(args);
	}
}