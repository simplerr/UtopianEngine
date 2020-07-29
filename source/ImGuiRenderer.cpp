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
        mVertexBuffer = std::make_shared<Vk::Buffer>(vulkanApp->GetDevice(), "ImGui vertex buffer");
        mIndexBuffer = std::make_shared<Vk::Buffer>(vulkanApp->GetDevice(), "ImGui index buffer");

        ImGui::CreateContext();

        SetDarkTheme();

        // Dimensions
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
        io.FontGlobalScale = mScale;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
        io.KeyMap[ImGuiKey_Backspace] = VK_BACK;

        mTextureDescriptorPool = std::make_shared<Vk::DescriptorPool>(vulkanApp->GetDevice());
        mTextureDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128);
        mTextureDescriptorPool->Create();

        mSampler = std::make_shared<Vk::Sampler>(vulkanApp->GetDevice());

        PrepareResources();

        mLastFrameTime = gTimer().GetTimestamp();

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

        io.Fonts->AddFontFromFileTTF("data/fonts/Roboto-Regular.ttf", 15.0f);

        // Create font texture
        unsigned char* fontData;
        int texWidth, texHeight, pixelSize;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight, &pixelSize);

        // Need to override vertex input description from shader since there is some special
        // treatment of U32 -> vec4 in ImGui
        mVertexDescription = std::make_shared<Vk::VertexDescription>();
        mVertexDescription->AddBinding(BINDING_0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
        mVertexDescription->AddAttribute(BINDING_0, Vk::Vec2Attribute());   // Location 0 : Position
        mVertexDescription->AddAttribute(BINDING_0, Vk::Vec2Attribute());   // Location 1 : Uv
        mVertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());    // Location 2 : Color

        Vk::EffectCreateInfo effectDesc;
        effectDesc.shaderDesc.vertexShaderPath = "data/shaders/imgui/uioverlay.vert";
        effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/imgui/uioverlay.frag";
        effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE; // The front facing order is reversed in the Imgui vertex buffer
        effectDesc.pipelineDesc.depthStencilState.depthTestEnable = VK_FALSE; // No depth testing
        effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE; // No depth testing
        effectDesc.pipelineDesc.blendingType = Vk::BlendingType::BLENDING_ALPHA;
        effectDesc.pipelineDesc.OverrideVertexInput(mVertexDescription);

        mImguiEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), mVulkanApp->GetRenderPass(), effectDesc);

        mTexture = Vk::gTextureLoader().CreateTexture(fontData, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight,
                                                      1, pixelSize, VK_IMAGE_ASPECT_COLOR_BIT, "ImGui font image");
        mImguiEffect->BindCombinedImage("fontSampler", *mTexture);

        io.Fonts->TexID = (ImTextureID)AddImage(*mTexture->GetImage());

        mCommandBuffer = new Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
        mCommandBuffer->SetActive(false); // Enable the command buffer when something is recorded to it (not the first frame)

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

        // A hack to disable the command buffer until something have been recorded to it
        static bool firstCall = true;
        if (firstCall)
        {
            mCommandBuffer->SetActive(true);
            firstCall = false;
        }
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

        if (vertexBufferSize == 0u || indexBufferSize == 0u)
            return;

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
        double delta = gTimer().GetElapsedTime(mLastFrameTime);
        delta /= 1000.0f; // To seconds
        mLastFrameTime = gTimer().GetTimestamp();
        io.DeltaTime = delta;

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

        StartDockSpace();
    }

    void ImGuiRenderer::EndFrame()
    {
        // DockSpace window end
        ImGui::End();

        ImGui::Render();
        ResetKeyStates();
    }

    void ImGuiRenderer::StartDockSpace()
    {
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        static bool fullscreen = true;
        if (fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetWorkPos());
            ImGui::SetNextWindowSize(viewport->GetWorkSize());
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            windowFlags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (fullscreen)
            ImGui::PopStyleVar(2);

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("UI"))
            {
               static bool darkTheme = true;
               if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0))
                  dockspaceFlags ^= ImGuiDockNodeFlags_PassthruCentralNode;
               if (ImGui::MenuItem("Dark theme", "", darkTheme))
               {
                  darkTheme = !darkTheme;

                  if (darkTheme)
                     SetDarkTheme();
                  else
                     SetLightTheme();
               }
               ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
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
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
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

    void ImGuiRenderer::SetDarkTheme()
    {
       // From Raikiris comment in https://github.com/ocornut/imgui/issues/707
       ImVec4* colors = ImGui::GetStyle().Colors;
       colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
       colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
       colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
       colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
       colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
       colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
       colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
       colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
       colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
       colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
       colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
       colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
       colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
       colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
       colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
       colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
       colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
       colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
       colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
       colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
       colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
       colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
       colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
       colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
       colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
       colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
       colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
       colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
       colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
       colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
       colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
       colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
       colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
       colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
       colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
       colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
       colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
       colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
       colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
       colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
       colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
       colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
       colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
       colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
       colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
       colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    void ImGuiRenderer::SetLightTheme()
    {
       // Light style based on https://github.com/ocornut/imgui/pull/511#issuecomment-175719267, by Pacôme Danhiez (user itamago)
       ImVec4* colors = ImGui::GetStyle().Colors;
       colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
       colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
       colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
       colors[ImGuiCol_ChildBg] = ImVec4(0.85f, 0.86f, 0.89f, 1.00f);
       colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
       colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
       colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
       colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
       colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
       colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
       colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
       colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
       colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
       colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
       colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
       colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
       colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
       colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
       colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
       colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
       colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
       colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
       colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
       colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
       colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
       colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
       colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
       colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
       colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
       colors[ImGuiCol_Tab] = ImVec4(0.62f, 0.70f, 0.74f, 1.00f);
       colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
       colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.40f, 0.43f, 1.00f);
       colors[ImGuiCol_TabUnfocused] = ImVec4(0.62f, 0.70f, 0.74f, 1.00f);
       colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.40f, 0.43f, 1.00f);
       colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
       colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
       colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
       colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
       colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
       colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
       colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
       colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
       colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
       colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
       colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
       colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }
}