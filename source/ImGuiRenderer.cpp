/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <algorithm>
#include "ImGuiRenderer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Texture.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/Image.h"
#include "vulkan/EffectManager.h"
#include "vulkan/Debug.h"
#include "Input.h"
#include "core/renderer/RendererUtility.h"

#define MOUSE_WHEEL_SCALING 150.0f

namespace Utopian
{
	bool ImGuiRenderer::mImguiVisible = true;

	ImGuiRenderer::ImGuiRenderer(Vk::VulkanApp* vulkanApp, uint32_t width, uint32_t height)
		: mVulkanApp(vulkanApp)
	{
		mVertexBuffer = std::make_shared<Vk::Buffer>(vulkanApp->GetDevice());
		mIndexBuffer = std::make_shared<Vk::Buffer>(vulkanApp->GetDevice());

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

		io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io.KeyMap[ImGuiKey_Backspace] = VK_BACK;

		mTextureDescriptorPool = std::make_shared<Vk::DescriptorPool>(vulkanApp->GetDevice());
		mTextureDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128);
		mTextureDescriptorPool->Create();

		mSampler = std::make_shared<Vk::Sampler>(vulkanApp->GetDevice());

		PrepareResources();

		mLastFrameTime = std::chrono::high_resolution_clock::now();
		mDeltaTime = 0.0;

		gInput().RegisterMouseInsideUiCallback(&ImGuiRenderer::IsMouseInsideUi);
		gInput().RegisterKeydownCallback(&ImGuiRenderer::KeydownCallback);
		gInput().RegisterUiCaptureCallback(&ImGuiRenderer::IsKeyboardCapture);
	}

	/** Free up all Vulkan resources acquired by the UI overlay */
	ImGuiRenderer::~ImGuiRenderer()
	{
		
	}

	/** Prepare all vulkan resources required to render the UI overlay */
	void ImGuiRenderer::PrepareResources()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->AddFontFromFileTTF("data/fonts/Roboto-Medium.ttf", 16.0f);

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight, pixelSize;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight, &pixelSize);

		// Note: Must be performed before Init()
		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/imgui/uioverlay.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/imgui/uioverlay.frag";

		mImguiEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), mVulkanApp->GetRenderPass(), shaderCreateInfo);

		gRendererUtility().SetAlphaBlending(mImguiEffect->GetPipeline());

		// The front facing order is reversed in the Imgui vertex buffer
		mImguiEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		// No depth testing
		mImguiEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mImguiEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;

		// Need to override vertex input description from shader since there is some special
		// treatment of U32 -> vec4 in ImGui
		mVertexDescription = std::make_shared<Vk::VertexDescription>();
		mVertexDescription->AddBinding(BINDING_0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
		mVertexDescription->AddAttribute(BINDING_0, Vk::Vec2Attribute());	// Location 0 : Position
		mVertexDescription->AddAttribute(BINDING_0, Vk::Vec2Attribute());	// Location 1 : Uv
		mVertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());	// Location 2 : Color

		mImguiEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);

		mImguiEffect->CreatePipeline();

		mTexture = Vk::gTextureLoader().CreateTexture(fontData, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, 1, pixelSize);
		mImguiEffect->BindCombinedImage("fontSampler", *mTexture);

		io.Fonts->TexID = (ImTextureID)AddImage(*mTexture->GetImage());

		mCommandBuffer = new Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		mVulkanApp->AddSecondaryCommandBuffer(mCommandBuffer);
	}

	void ImGuiRenderer::UpdateCommandBuffers()
	{
		ImGuiIO& io = ImGui::GetIO();

		mCommandBuffer->Begin(mVulkanApp->GetRenderPass(), mVulkanApp->GetCurrentFrameBuffer());

		mCommandBuffer->CmdSetViewPort(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
		mCommandBuffer->CmdSetScissor(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

		mCommandBuffer->CmdBindPipeline(mImguiEffect->GetPipeline());

		mCommandBuffer->CmdBindVertexBuffer(0, 1, mVertexBuffer.get());
		mCommandBuffer->CmdBindIndexBuffer(mIndexBuffer->GetVkHandle(), 0, VK_INDEX_TYPE_UINT16);

		// UI scale and translate via push constants
		PushConstantBlock pushConstBlock;
		pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		pushConstBlock.translate = glm::vec2(-1.0f);
		mCommandBuffer->CmdPushConstants(mImguiEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConstBlock), &pushConstBlock);

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

				VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
				vkCmdBindDescriptorSets(mCommandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mImguiEffect->GetPipelineInterface()->GetPipelineLayout(), 0, 1, desc_set, 0, NULL);

				mCommandBuffer->CmdDrawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}

		mCommandBuffer->End();
	}

	/** Update vertex and index buffer containing the imGui elements when required */
	void ImGuiRenderer::Render()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffers = false;

		if (!imDrawData) { return; };

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		// Update buffers only if vertex or index count has been changed compared to current buffer size

		// Vertex buffer
		if ((mVertexBuffer->GetVkHandle() == VK_NULL_HANDLE) || (mVertexCount != imDrawData->TotalVtxCount)) {
			mVertexBuffer->UnmapMemory();
			mVertexBuffer->Destroy();
			mVertexBuffer->Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
			mVertexCount = imDrawData->TotalVtxCount;
			mVertexBuffer->MapMemory((void**)&mMappedVertices);
			updateCmdBuffers = true;
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if ((mIndexBuffer->GetVkHandle() == VK_NULL_HANDLE) || (mIndexCount < imDrawData->TotalIdxCount)) {
			mIndexBuffer->UnmapMemory();
			mIndexBuffer->Destroy();
			mIndexBuffer->Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
			mIndexCount = imDrawData->TotalIdxCount;
			mIndexBuffer->MapMemory((void**)&mMappedIndices);
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

		// Todo: Note: Currently the command buffer needs to be updated each frame since if it is not
		// then the framebuffer used when beginning the command buffer will get out of sync with the current one from the swap chain.
		const bool alwaysUpdate = true;
		if (alwaysUpdate || updateCmdBuffers) {
			UpdateCommandBuffers();
		}
	}
	
	ImTextureID ImGuiRenderer::AddImage(const Vk::Image& image)
	{
		VkDescriptorSet descriptorSet;

		// Create Descriptor Set:
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = mTextureDescriptorPool->GetVkHandle();
		alloc_info.descriptorSetCount = 1;
		VkDescriptorSetLayout descriptorSetLayout = mImguiEffect->GetPipelineInterface()->GetDescriptorSetLayout(0)->GetVkHandle();
		alloc_info.pSetLayouts = &descriptorSetLayout;
		Vk::Debug::ErrorCheck(vkAllocateDescriptorSets(mVulkanApp->GetDevice()->GetVkDevice(), &alloc_info, &descriptorSet));

		// Update the Descriptor Set:
		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = mSampler->GetVkHandle();
		desc_image[0].imageView = image.GetView();
		desc_image[0].imageLayout = image.GetFinalLayout();

		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = descriptorSet;
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = desc_image;
		vkUpdateDescriptorSets(mVulkanApp->GetDevice()->GetVkDevice(), 1, write_desc, 0, NULL);

		return (ImTextureID)descriptorSet;
	}

	void ImGuiRenderer::FreeTexture(ImTextureID textureId)
	{
		VkDescriptorSet descriptorSet = (VkDescriptorSet)textureId;
		mTextureDescriptorsToFree.push_back(descriptorSet);
	}

	void ImGuiRenderer::NewFrame()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Update elapsed frame time
		auto now = std::chrono::high_resolution_clock::now();
		mDeltaTime = std::chrono::duration<double, std::milli>(now - mLastFrameTime).count();
		mDeltaTime /= 1000.0f; // To seconds
		mLastFrameTime = now;
		io.DeltaTime = mDeltaTime;

		// Update mouse state
		glm::vec2 mousePos = gInput().GetMousePosition();
		io.MousePos = ImVec2(mousePos.x, mousePos.y);
		io.MouseDown[0] = gInput().KeyDown(VK_LBUTTON, false);
		io.MouseDown[1] = gInput().KeyDown(VK_RBUTTON, false);
		io.MouseWheel = gInput().MouseDz() / MOUSE_WHEEL_SCALING;

		// No mouse input if Imgui is not visible
		if (!mImguiVisible)
		{
			io.MouseDown[0] = false;
			io.MouseDown[1] = false;
			io.MouseWheel = 0.0f;
		}

		ImGui::NewFrame();
	}

	void ImGuiRenderer::EndFrame()
	{
		ImGui::Render();
		ResetKeyStates();
	}

	void ImGuiRenderer::Resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(width), (float)(height));
		UpdateCommandBuffers();
	}

	Utopian::Vk::CommandBuffer* ImGuiRenderer::GetCommandBuffer() const
	{
		return mCommandBuffer;
	}

	void ImGuiRenderer::TextV(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		ImGui::TextV(format, args);
		va_end(args);
	}
	
	void ImGuiRenderer::BeginWindow(std::string label, glm::vec2 position, float itemWidth)
	{
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(position.x, position.y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin(label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::PushItemWidth(itemWidth);
	}

	void ImGuiRenderer::EndWindow()
	{
		ImGui::PopItemWidth();
		ImGui::End();
		//ImGui::PopStyleVar();
	}
	
	void ImGuiRenderer::ToggleVisible()
	{
		mImguiVisible = !mImguiVisible;
		mCommandBuffer->SetActive(mImguiVisible);
	}

	bool ImGuiRenderer::IsMouseInsideUi()
	{
		return mImguiVisible && ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
	}
	
	bool ImGuiRenderer::IsKeyboardCapture()
	{
		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureKeyboard;
	}

	void ImGuiRenderer::KeydownCallback(char key)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(key);
		io.KeysDown[key] = true;
	}

	void ImGuiRenderer::ResetKeyStates()
	{
		ImGuiIO& io = ImGui::GetIO();
		for (uint32_t i = 0; i < 512; i++)
			io.KeysDown[i] = false;
	}

	void ImGuiRenderer::GarbageCollect()
	{
		// Clear garbage collected textures
		if (mTextureDescriptorsToFree.size() > 0)
		{
			vkFreeDescriptorSets(mVulkanApp->GetDevice()->GetVkDevice(), mTextureDescriptorPool->GetVkHandle(), mTextureDescriptorsToFree.size(), mTextureDescriptorsToFree.data());
			mTextureDescriptorsToFree.clear();
		}
	}
}