#include <array>
#include <time.h>
#include <cstdlib>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "VulkanDebug.h"
#include "StaticModel.h"
#include "Camera.h"
#include "LightData.h"
#include "TextureLoader.h"
#include "Device.h"
#include "ShaderFactory.h"
#include "TextOverlay.h"
#include "ModelLoader.h"
#include "handles/DescriptorSet.h"
#include "handles/CommandBuffer.h"
#include "handles/CommandPool.h"
#include "handles/PipelineLegacy.h"
#include "handles/Fence.h"
#include "handles/PipelineLayout.h"
#include "handles/RenderPass.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"
#include "handles/DescriptorSetLayout.h"
#include "vulkan/UIOverlay.h"
#include "ScreenGui.h"

#define VK_FLAGS_NONE 0
#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION false		// Debug validation layers toggle (affects performance a lot)

namespace Utopian::Vk
{
	Renderer::Renderer() : VulkanBase(VULKAN_ENABLE_VALIDATION)
	{
		srand(time(NULL));
		mCamera = nullptr;
		mApplicationCommandBuffers.resize(0);
	}

	Renderer::~Renderer()
	{
		delete mDescriptorPool;
		delete mTextureDescriptorSetLayout;
		delete mPrimaryCommandBuffer;

		for (CommandBuffer* commandBuffer : mApplicationCommandBuffers)
		{
			delete commandBuffer;
		}

		//delete mTextOverlay;
		delete mScreenGui;
	}

	void Renderer::Prepare()
	{
		VulkanBase::Prepare();

		SetupDescriptorSetLayout();			// Must run before PreparePipelines() (VkPipelineLayout)

		SetupDescriptorPool();
		PrepareCommandBuffers();

		mPrepared = true;

		std::vector<Vk::ScreenQuadVertex> vertices =
		{
			{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }
		};

		mScreenQuad.vertexBuffer = new Utopian::Vk::Buffer(GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertices.size() * sizeof(Vk::ScreenQuadVertex), vertices.data());

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		mScreenQuad.indexBuffer = new Utopian::Vk::Buffer(GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.size() * sizeof(uint32_t), indices.data());
	}

	void Renderer::PostInitPrepare()
	{
		//mTextOverlay = new TextOverlay(this);
		mScreenGui = new ScreenGui(this);
		mUiOverlay = new UIOverlay(GetWindowWidth(), GetWindowHeight(), this);
	}

	void Renderer::PrepareCommandBuffers()
	{
		// Create the primary and secondary command buffers
		mPrimaryCommandBuffer = new CommandBuffer(mDevice, GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		mScreenGuiCommandBuffer = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}

	void Renderer::CompileShaders()
	{
		// [TODO] Move to ShaderManager
		system("cd data/shaders/phong/ && generate-spirv.bat");
		system("cd data/shaders/test/ && generate-spirv.bat");
		//system("cd data/shaders/normal_debug/ && generate-spirv.bat");
		system("cd data/shaders/textoverlay/ && generate-spirv.bat");
		system("cd data/shaders/terrain/ && generate-spirv.bat");
		system("cd data/shaders/marching_cubes/ && generate-spirv.bat");
		system("cd data/shaders/screenquad/ && generate-spirv.bat");
		system("cd data/shaders/water/ && generate-spirv.bat");
		//system("cd data/shaders/imgui/ && generate-spirv.bat");
		system("cd data/shaders/color/ && generate-spirv.bat");
		//system("cd data/shaders/gbuffer/ && generate-spirv.bat");
		//system("cd data/shaders/deferred/ && generate-spirv.bat");
		//system("cd data/shaders/ssao/ && generate-spirv.bat");
		//system("cd data/shaders/blur/ && generate-spirv.bat");
	}

	void Renderer::SetCamera(Utopian::Camera* camera)
	{
		mCamera = camera;
	}

	Utopian::Camera* Renderer::GetCamera()
	{
		return mCamera;
	}

	void Renderer::SetClearColor(glm::vec4 color)
	{
		mClearColor = color;
	}

	glm::vec4 Renderer::GetClearColor()
	{
		return mClearColor;
	}

	CommandBuffer* Renderer::CreateCommandBuffer(VkCommandBufferLevel level)
	{
		CommandBuffer* commandBuffer = new CommandBuffer(mDevice, GetCommandPool(), level);
		mApplicationCommandBuffers.push_back(commandBuffer);
		return commandBuffer;
	}

	void Renderer::RecordRenderingCommandBuffer(VkFramebuffer frameBuffer)
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { mClearColor.r, mClearColor.g, mClearColor.b, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass->GetVkHandle();
		renderPassBeginInfo.renderArea.extent.width = GetWindowWidth();
		renderPassBeginInfo.renderArea.extent.height = GetWindowHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = frameBuffer;

		// Begin command buffer recording & the render pass
		mPrimaryCommandBuffer->Begin();
		mPrimaryCommandBuffer->CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);	// VK_SUBPASS_CONTENTS_INLINE

		std::vector<VkCommandBuffer> commandBuffers;
		for (CommandBuffer* commandBuffer : mApplicationCommandBuffers)
		{
			if (commandBuffer->IsActive()) 
			{
				commandBuffers.push_back(commandBuffer->GetVkHandle());
			}
		}

		// This is where multithreaded command buffers can be added
		// ...

		// Execute render commands from the secondary command buffer
		vkCmdExecuteCommands(mPrimaryCommandBuffer->GetVkHandle(), commandBuffers.size(), commandBuffers.data());

		// End command buffer recording & the render pass
		mPrimaryCommandBuffer->CmdEndRenderPass();
		mPrimaryCommandBuffer->End();
	}

	void Renderer::Draw()
	{
		// Render screen overlay UI

		mScreenGuiCommandBuffer->Begin(GetRenderPass(), GetCurrentFrameBuffer());
		mScreenGuiCommandBuffer->CmdSetViewPort(GetWindowWidth(), GetWindowHeight());
		mScreenGuiCommandBuffer->CmdSetScissor(GetWindowWidth(), GetWindowHeight());

		mScreenGui->Render(this, mScreenGuiCommandBuffer);

		mScreenGuiCommandBuffer->End();

		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 
		RecordRenderingCommandBuffer(mFrameBuffers->GetCurrent());

		mQueue->Submit(mPrimaryCommandBuffer, nullptr);
	}

	SharedPtr<ScreenQuad> Renderer::AddScreenQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler, uint32_t layer)
	{
		return mScreenGui->AddQuad(left, top, width, height, image, sampler, layer);
	}

	SharedPtr<ScreenQuad> Renderer::AddScreenQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, VkImageView imageView, Utopian::Vk::Sampler* sampler, uint32_t layer)
	{
		return mScreenGui->AddQuad(left, top, width, height, imageView, sampler, layer);
	}

	SharedPtr<ScreenQuad> Renderer::AddScreenQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture, uint32_t layer)
	{
		//return mScreenGui->AddQuad(left, top, width, height, texture, layer);
		return nullptr;
	}

	void Renderer::Render()
	{
		if (mPrepared) {
			Draw();
		}
	}

	void Renderer::Update()
	{
		UpdateOverlay();

		// if (mTextOverlay->IsVisible())
		// {
		// 	mTextOverlay->BeginTextUpdate();

		// 	mTextOverlay->AddText("Camera pos", mCamera->GetPosition(), 5.0f, 5.0f, TextOverlay::ALIGN_LEFT);
		// 	mTextOverlay->AddText("Camera dir", mCamera->GetDirection(), 5.0f, 50.0f, TextOverlay::ALIGN_LEFT);

		// 	glm::mat4 mat = glm::mat4();
		// 	mTextOverlay->AddText("Camera view matrix", mCamera->GetView(), 5.0f, 90.0f, TextOverlay::ALIGN_LEFT);
		// 	mTextOverlay->EndTextUpdate();
		// }
	}

	void Renderer::DrawScreenQuad(CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdBindVertexBuffer(0, 1, mScreenQuad.vertexBuffer);
		commandBuffer->CmdBindIndexBuffer(mScreenQuad.indexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
		commandBuffer->CmdDrawIndexed(6, 1, 0, 0, 0);
	}

	void Renderer::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_SPACE)
			{
				//mTextOverlay->ToggleVisible();
			}
			break;
		}

		// Default message handling
		VulkanBase::HandleMessages(hwnd, msg, wParam, lParam);
	}

	void Renderer::BeginUiUpdate()
	{
		ImGui::NewFrame();
	}

	void Renderer::EndUiUpdate()
	{
		ImGui::Render();

		mUiOverlay->Update();
	}

	void Renderer::ToggleUi()
	{
		mUiOverlay->ToggleVisible();
	}

	void Renderer::UpdateOverlay()
	{
	}
}	// VulkanLib namespace