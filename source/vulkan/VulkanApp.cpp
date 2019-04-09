#include <array>
#include <time.h>
#include <cstdlib>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/VulkanApp.h"
#include "Debug.h"
#include "StaticModel.h"
#include "Camera.h"
#include "LightData.h"
#include "TextureLoader.h"
#include "vulkan/handles/Device.h"
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
#include "ImGuiRenderer.h"
#include "utility/Utility.h"
#include "ScreenQuadRenderer.h"

#define VK_FLAGS_NONE 0
#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION true		// Debug validation layers toggle (affects performance a lot)

namespace Utopian::Vk
{
	VulkanApp::VulkanApp(Window* window)
		: VulkanBase(window, VULKAN_ENABLE_VALIDATION)
	{
		mSecondaryCommandBuffers.resize(0);
		SetClearColor(ColorRGB(47, 141, 255));
	}

	VulkanApp::~VulkanApp()
	{
		delete mPrimaryCommandBuffer;

		for (CommandBuffer* commandBuffer : mSecondaryCommandBuffers)
		{
			delete commandBuffer;
		}
	}

	void VulkanApp::Prepare()
	{
		VulkanBase::Prepare();

		mPrimaryCommandBuffer = new CommandBuffer(mDevice, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}

	void VulkanApp::PostInitPrepare()
	{
		mImGuiRenderer = new ImGuiRenderer(GetWindowWidth(), GetWindowHeight(), this);
	}

	void VulkanApp::SetClearColor(glm::vec4 color)
	{
		mClearColor = color;
	}

	glm::vec4 VulkanApp::GetClearColor()
	{
		return mClearColor;
	}

	void VulkanApp::AddSecondaryCommandBuffer(CommandBuffer* commandBuffer)
	{
		mSecondaryCommandBuffers.push_back(commandBuffer);
	}

	void VulkanApp::RecordRenderingCommandBuffer(VkFramebuffer frameBuffer)
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
		for (CommandBuffer* commandBuffer : mSecondaryCommandBuffers)
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

	void VulkanApp::Render()
	{
		UpdateImgui();

		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 
		RecordRenderingCommandBuffer(mFrameBuffers->GetCurrent());

		mDevice->GetQueue()->Submit(mPrimaryCommandBuffer, mWaitFence.get(), mJobGraphWaitSemaphore, GetRenderCompleteSemaphore());
	}

	void VulkanApp::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

	void VulkanApp::UpdateImgui()
	{
		mImGuiRenderer->Update();
	}

	void VulkanApp::ToggleUi()
	{
		mImGuiRenderer->ToggleVisible();
	}
	
	ImGuiRenderer* VulkanApp::GetUiOverlay()
	{
		return mImGuiRenderer;
	}
}	// VulkanLib namespace