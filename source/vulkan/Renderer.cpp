#include <array>
#include <time.h>
#include <cstdlib>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "VulkanDebug.h"
#include "StaticModel.h"
#include "Camera.h"
#include "VulkanHelpers.h"
#include "LightData.h"
#include "TextureLoader.h"
#include "Device.h"
#include "ShaderManager.h"
#include "TextOverlay.h"
#include "ModelLoader.h"
#include "handles/DescriptorSet.h"
#include "handles/CommandBuffer.h"
#include "handles/CommandPool.h"
#include "handles/Pipeline.h"
#include "handles/Fence.h"
#include "handles/PipelineLayout.h"
#include "handles/RenderPass.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"
#include "handles/DescriptorSetLayout.h"
#include "ecs/systems/RenderSystem.h"

#define VK_FLAGS_NONE 0
#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION true		// Debug validation layers toggle (affects performance a lot)

namespace Vulkan
{
	Renderer::Renderer() : VulkanBase(VULKAN_ENABLE_VALIDATION)
	{
		srand(time(NULL));
		mCamera = nullptr;
	}

	Renderer::~Renderer()
	{
		delete mDescriptorPool;
		delete mTextureDescriptorSetLayout;
		delete mRenderFence;
		delete mPrimaryCommandBuffer;

		for (CommandBuffer* commandBuffer : mApplicationCommandBuffers)
		{
			delete commandBuffer;
		}

		delete mShaderManager;
		delete mTextOverlay;
		delete mTextureLoader;
	}

	void Renderer::Prepare()
	{
		VulkanBase::Prepare();

		mRenderFence = new Fence(mDevice, VK_FLAGS_NONE);
		mShaderManager = new ShaderManager(mDevice);

		SetupDescriptorSetLayout();			// Must run before PreparePipelines() (VkPipelineLayout)

		SetupDescriptorPool();
		PrepareCommandBuffers();

		mTextureLoader = new Vulkan::TextureLoader(this, GetQueue()->GetVkHandle());
		mModelLoader = new Vulkan::ModelLoader(mTextureLoader);
		mTextOverlay = new TextOverlay(this);
		mPrepared = true;
	}

	void Renderer::PrepareCommandBuffers()
	{
		// Create the primary and secondary command buffers
		mPrimaryCommandBuffer = new CommandBuffer(mDevice, GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}

	void Renderer::CompileShaders()
	{
		// [TODO] Move to ShaderManager
		system("cd data/shaders/phong/ && generate-spirv.bat");
		system("cd data/shaders/test/ && generate-spirv.bat");
		system("cd data/shaders/normal_debug/ && generate-spirv.bat");
		system("cd data/shaders/textoverlay/ && generate-spirv.bat");
		system("cd data/shaders/terrain/ && generate-spirv.bat");
		system("cd data/shaders/marching_cubes/ && generate-spirv.bat");
		system("cd data/shaders/screenquad/ && generate-spirv.bat");
		system("cd data/shaders/water/ && generate-spirv.bat");
	}

	void Renderer::SetCamera(Camera* camera)
	{
		mCamera = camera;
	}

	DescriptorSetLayout* Renderer::GetTextureDescriptorSetLayout()
	{
		return mTextureDescriptorSetLayout;
	}

	DescriptorPool* Renderer::GetDescriptorPool()
	{
		return mDescriptorPool;
	}

	CommandBuffer* Renderer::CreateCommandBuffer(VkCommandBufferLevel level)
	{
		CommandBuffer* commandBuffer = new CommandBuffer(mDevice, GetCommandPool(), level);
		mApplicationCommandBuffers.push_back(commandBuffer);
		return commandBuffer;
	}

	void Renderer::SetupDescriptorSetLayout()
	{
		// Here we want to split the descriptor set into 3 different ones
		// The PipelineLayout can take a an array of several DescriptorSetLayout
		// And when rendering vkCmdBindDescriptorSets also takes an array of DescriptorSets

		mTextureDescriptorSetLayout = new DescriptorSetLayout(GetDevice());

		//// TODO: Separate DescriptorSet and DescriptorSetLayout
		//// DescriptorSets should be able to be created from an existing DescriptorSetLayout
		mTextureDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);			// Combined image sampler binding: 2

		mTextureDescriptorSetLayout->Create();
	}

	void Renderer::SetupDescriptorPool()
	{
		mDescriptorPool = new DescriptorPool(GetDevice());
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64);
		mDescriptorPool->Create();
	}

	void Renderer::RecordRenderingCommandBuffer(VkFramebuffer frameBuffer)
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 0.0f };
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
		mPrimaryCommandBuffer->CmdBeginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);	// VK_SUBPASS_CONTENTS_INLINE

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
		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 
		RecordRenderingCommandBuffer(mFrameBuffers->GetCurrent());

		mQueue->Submit(mPrimaryCommandBuffer, mRenderFence);

		// Wait for all command buffers to complete
		mRenderFence->Wait(); 
	}

	void Renderer::Render()
	{
		mCamera->Update();

		if (mPrepared) {
			Draw();
		}
	}

	void Renderer::Update()
	{
		if (mTextOverlay->IsVisible())
		{
			mTextOverlay->BeginTextUpdate();

			mTextOverlay->AddText("Camera pos", mCamera->GetPosition(), 5.0f, 5.0f, TextOverlay::ALIGN_LEFT);
			mTextOverlay->AddText("Camera dir", mCamera->GetDirection(), 5.0f, 50.0f, TextOverlay::ALIGN_LEFT);

			glm::mat4 mat = glm::mat4();
			mTextOverlay->AddText("Camera view matrix", mCamera->GetView(), 5.0f, 90.0f, TextOverlay::ALIGN_LEFT);
			mTextOverlay->EndTextUpdate();
		}
	}

	void Renderer::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_SPACE)
			{
				mTextOverlay->ToggleVisible();
			}
			break;
		}

		// Default message handling
		VulkanBase::HandleMessages(hwnd, msg, wParam, lParam);

		// Let the camera handle user input
		mCamera->HandleMessages(hwnd, msg, wParam, lParam);
	}
}	// VulkanLib namespace