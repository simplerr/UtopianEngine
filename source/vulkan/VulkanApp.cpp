#include <array>
#include <time.h>
#include <cstdlib>
#include <thread>
#include "VulkanApp.h"
#include "VulkanDebug.h"
#include "StaticModel.h"
#include "Camera.h"
#include "Object.h"
#include "VulkanHelpers.h"
#include "Light.h"
#include "TextureLoader.h"
#include "Device.h"
#include "ShaderManager.h"
#include "handles/CommandBuffer.h"
#include "handles/CommandPool.h"
#include "handles/Pipeline.h"
#include "handles/Fence.h"
#include "handles/PipelineLayout.h"
#include "handles/RenderPass.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"
#include "ecs/systems/RenderSystem.h"

#define VK_FLAGS_NONE 0
#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION false		// Debug validation layers toggle (affects performance a lot)

namespace VulkanLib
{
	VulkanApp::VulkanApp() : VulkanBase(VULKAN_ENABLE_VALIDATION)
	{
		srand(time(NULL));
		mCamera = nullptr;
	}

	VulkanApp::~VulkanApp()
	{
		mVertexUniformBuffer.Cleanup(GetDevice());
		mFragmentUniformBuffer.Cleanup(GetDevice());
		mDescriptorPool.Cleanup(GetDevice());
		mDescriptorSet.Cleanup(GetDevice());

		for (auto const& pipeline : mPipelines)
		{
			delete pipeline.second;
		}

		delete mPipelineLayout;

		delete mRenderFence;

		// Free the testing texture
		mTextureLoader->DestroyTexture(mTestTexture);

		for (int i = 0; i < mModels.size(); i++) {
			delete mModels[i].object;
		}

		delete mPrimaryCommandBuffer;
		delete mSecondaryCommandBuffer;

		delete mTextureLoader;
	}

	void VulkanApp::Prepare()
	{
		VulkanBase::Prepare();

		// Create a fence for synchronization
		mRenderFence = new Fence(GetDevice(), VK_FLAGS_NONE);

		SetupVertexDescriptions();			
		SetupDescriptorSetLayout();			// Must run before PreparePipelines() (VkPipelineLayout)
		PreparePipelines();
		PrepareUniformBuffers();			// Must run before SetupDescriptorSet() (Creates the uniform buffer)

		mTextureLoader = new TextureLoader(mDevice, mQueue->GetVkHandle());
		mTextureLoader->LoadTexture("data/textures/crate.jpg", &mTestTexture);

		SetupDescriptorPool();
		SetupDescriptorSet();
		PrepareCommandBuffers();

		mPrepared = true;
	}

	void VulkanApp::PrepareCommandBuffers()
	{
		// Create the primary and secondary command buffers
		mPrimaryCommandBuffer = new CommandBuffer(GetDevice(), GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		mSecondaryCommandBuffer = new CommandBuffer(GetDevice(), GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}

	void VulkanApp::CompileShaders()
	{
		// [TODO] Move to ShaderManager
		system("cd data/shaders/phong/ && generate-spirv.bat");
		system("cd data/shaders/test/ && generate-spirv.bat");
	}

	void VulkanApp::SetCamera(Camera * camera)
	{
		mCamera = camera;
	}

	void VulkanApp::AddModel(VulkanModel model)
	{
		mModels.push_back(model);
	}

	void VulkanApp::SetRenderSystem(ECS::RenderSystem* renderSystem)
	{
		mRenderSystem = renderSystem;
	}
	
	void VulkanApp::PrepareUniformBuffers()
	{
		// Create the fragment shader uniform buffer
		Light light;
		light.SetMaterials(vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(1, -1, 1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.8f, 1.0f);
		light.SetType(LightType::DIRECTIONAL_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mFragmentUniformBuffer.lights.push_back(light);

		light.SetMaterials(vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(-1, -1, -1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.5f, 1.0f);
		light.SetType(LightType::SPOT_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mFragmentUniformBuffer.lights.push_back(light);

		// Important to call this before CreateBuffer() since # lights affects the total size
		mFragmentUniformBuffer.constants.numLights = mFragmentUniformBuffer.lights.size();

		mFragmentUniformBuffer.CreateBuffer(this, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Create the vertex shader uniform buffer
		mVertexUniformBuffer.CreateBuffer(this, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		UpdateUniformBuffers();
	}

	// Call this every time any uniform buffer should be updated (view changes etc.)
	void VulkanApp::UpdateUniformBuffers()
	{
		if (mCamera != nullptr)
		{
			mVertexUniformBuffer.camera.projectionMatrix = mCamera->GetProjection();
			mVertexUniformBuffer.camera.viewMatrix = mCamera->GetView();
			mVertexUniformBuffer.camera.projectionMatrix = mCamera->GetProjection();
			mVertexUniformBuffer.camera.eyePos = mCamera->GetPosition();
		}

		mVertexUniformBuffer.UpdateMemory(GetDevice());
		mFragmentUniformBuffer.UpdateMemory(GetDevice());
	}

	void VulkanApp::SetupDescriptorSetLayout()
	{
		mDescriptorSet.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);				// Uniform buffer binding: 0
		mDescriptorSet.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);				// Uniform buffer binding: 1
		mDescriptorSet.AddLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);		// Combined image sampler binding: 2
		mDescriptorSet.CreateLayout(GetDevice());

		PushConstantRange pushConstantRange = PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstantBlock));
		mPipelineLayout = new PipelineLayout(GetDevice(), &mDescriptorSet.setLayout, &pushConstantRange);
	}

	void VulkanApp::SetupDescriptorPool()
	{
		mDescriptorPool.CreatePoolFromLayout(GetDevice(), mDescriptorSet.GetLayoutBindings());
	}

	// [TODO] Let each thread have a seperate descriptor set!!
	void VulkanApp::SetupDescriptorSet()
	{
		mDescriptorSet.AllocateDescriptorSets(GetDevice(), mDescriptorPool.GetVkDescriptorPool());
		mDescriptorSet.BindUniformBuffer(0, &mVertexUniformBuffer.GetDescriptor());
		mDescriptorSet.BindUniformBuffer(1, &mFragmentUniformBuffer.GetDescriptor());
		mDescriptorSet.BindCombinedImage(2, &mTestTexture.GetTextureDescriptorInfo());
		mDescriptorSet.UpdateDescriptorSets(GetDevice());
	}

	void VulkanApp::PreparePipelines()
	{
		// Load shader
		// [TODO] Move this into Pipeline?
		Shader* shader = mShaderManager->CreateShader("data/shaders/phong/phong.vert.spv", "data/shaders/phong/phong.frag.spv");
			
		// Solid pipeline
		Pipeline*  pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, shader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_BASIC] = pipeline;

		// Wireframe pipeline
		pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, shader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_WIREFRAME] = pipeline;

		// Test pipeline
		Shader* testShader = mShaderManager->CreateShader("data/shaders/test/test.vert.spv", "data/shaders/test/test.frag.spv");
		pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, testShader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_TEST] = pipeline;

	}

	void VulkanApp::SetupVertexDescriptions()
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription.AddBinding(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);					// Per vertex

		// We need to tell Vulkan about the memory layout for each attribute
		// 5 attributes: position, normal, texture coordinates, tangent and color
		// See Vertex struct
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec3Attribute());	// Location 0 : Position
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec3Attribute());	// Location 1 : Color
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec3Attribute());	// Location 2 : Normal
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec2Attribute());	// Location 3 : Texture
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec4Attribute());	// Location 4 : Tangent
	}

	void VulkanApp::RecordRenderingCommandBuffer(VkFramebuffer frameBuffer)
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { 0.2f, 0.2f, 0.8f, 0.0f };
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
		vkCmdBeginRenderPass(mPrimaryCommandBuffer->GetVkHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);	// VK_SUBPASS_CONTENTS_INLINE

		//
		// Secondary command buffer
		//
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = mRenderPass->GetVkHandle();
		inheritanceInfo.framebuffer = frameBuffer;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

		mSecondaryCommandBuffer->Begin(mRenderPass, frameBuffer);
		VkCommandBuffer secondaryCommandBuffer = mSecondaryCommandBuffer->GetVkHandle();

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.width = (float)GetWindowWidth();
		viewport.height = (float)GetWindowHeight();
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(secondaryCommandBuffer, 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = GetWindowWidth();
		scissor.extent.height = GetWindowHeight();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(secondaryCommandBuffer, 0, 1, &scissor);

		// The render system does the actual rendering
		mRenderSystem->Render(mSecondaryCommandBuffer, mPipelines, mPipelineLayout, mDescriptorSet);

		// End secondary command buffer
		mSecondaryCommandBuffer->End();

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(mSecondaryCommandBuffer->GetVkHandle());

		// This is where multithreaded command buffers can be added
		// ...

		// Execute render commands from the secondary command buffer
		vkCmdExecuteCommands(mPrimaryCommandBuffer->GetVkHandle(), commandBuffers.size(), commandBuffers.data());

		// End command buffer recording & the render pass
		vkCmdEndRenderPass(mPrimaryCommandBuffer->GetVkHandle());
		mPrimaryCommandBuffer->End();
	}

	void VulkanApp::Draw()
	{
		VulkanBase::PrepareFrame();

		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 
		RecordRenderingCommandBuffer(mFrameBuffers->GetCurrent());

		mQueue->Submit(mPrimaryCommandBuffer, mRenderFence);

		// Wait for all command buffers to complete
		mRenderFence->Wait(); 

		VulkanBase::SubmitFrame();
	}

	void VulkanApp::Render()
	{
		mCamera->Update();

		if (mPrepared) {
			UpdateUniformBuffers();
			Draw();
		}
	}

	void VulkanApp::Update()
	{
		//return;
		// Rotate the objects
		for (auto& object : mModels)
		{
			// [NOTE] Just for testing
			float speed = 5.0f;
			if (object.object->GetId() == OBJECT_ID_PROP)
				object.object->AddRotation(glm::radians(speed), glm::radians(speed), glm::radians(speed));
		}
	}

	void VulkanApp::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Default message handling
		VulkanBase::HandleMessages(hwnd, msg, wParam, lParam);

		// Let the camera handle user input
		mCamera->HandleMessages(hwnd, msg, wParam, lParam);
	}
}	// VulkanLib namespace