/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <algorithm>
#include "UIOverlay.h"
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
#include "Input.h"

namespace Utopian::Vk 
{
	UIOverlay::UIOverlay(uint32_t width, uint32_t height, Utopian::Vk::Renderer* renderer)
		: mRenderer(renderer)
	{
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
		io.FontGlobalScale = mScale;

		PrepareResources();
	}

	/** Free up all Vulkan resources acquired by the UI overlay */
	UIOverlay::~UIOverlay()
	{
		
	}

	/** Prepare all vulkan resources required to render the UI overlay */
	void UIOverlay::PrepareResources()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight, pixelSize;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight, &pixelSize);

		// Note: Must be performed before Init()
		mImguiEffect.mTexture = mRenderer->mTextureLoader->CreateTexture(fontData, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, 1, pixelSize);
		mImguiEffect.Init(mRenderer);

		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}


	void UIOverlay::UpdateCommandBuffers()
	{
		ImGuiIO& io = ImGui::GetIO();

		mCommandBuffer->Begin();

		mCommandBuffer->CmdSetViewPort(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
		mCommandBuffer->CmdSetScissor(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

		mCommandBuffer->CmdBindPipeline(mImguiEffect.GetPipeline(0));
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
				mCommandBuffer->CmdDrawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}

		mCommandBuffer->End();
	}

	/** Update vertex and index buffer containing the imGui elements when required */
	void UIOverlay::Update()
	{
		// Update mouse state
		ImGuiIO& io = ImGui::GetIO();
		glm::vec2 mousePos = gInput().GetMousePosition();
		io.MousePos = ImVec2(mousePos.x, mousePos.y);
		io.MouseDown[0] = gInput().KeyDown(VK_LBUTTON);
		io.MouseDown[1] = gInput().KeyDown(VK_RBUTTON);

		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffers = false;

		if (!imDrawData) { return; };

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		// Update buffers only if vertex or index count has been changed compared to current buffer size

		// Vertex buffer
		if ((mVertexBuffer.GetVkBuffer() == VK_NULL_HANDLE) || (mVertexCount != imDrawData->TotalVtxCount)) {
			mVertexBuffer.UnmapMemory();
			mVertexBuffer.Destroy();
			mVertexBuffer.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
			mVertexCount = imDrawData->TotalVtxCount;
			mVertexBuffer.MapMemory(0, VK_WHOLE_SIZE, 0, (void**)&mMappedVertices);
			updateCmdBuffers = true;
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if ((mIndexBuffer.GetVkBuffer() == VK_NULL_HANDLE) || (mIndexCount < imDrawData->TotalIdxCount)) {
			mIndexBuffer.UnmapMemory();
			mIndexBuffer.Destroy();
			mIndexBuffer.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize);
			mIndexCount = imDrawData->TotalIdxCount;
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
			UpdateCommandBuffers();
		}
	}

	void UIOverlay::Resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(width), (float)(height));
		UpdateCommandBuffers();
	}

	Utopian::Vk::CommandBuffer* UIOverlay::GetCommandBuffer() const
	{
		return mCommandBuffer;
	}

	void UIOverlay::TextV(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		ImGui::TextV(format, args);
		va_end(args);
	}
}