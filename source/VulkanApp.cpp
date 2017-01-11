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
#include "VulkanDevice.h"

#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION true		// Debug validation layers toggle (affects performance a lot)

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

		// Cleanup pipeline layout
		vkDestroyPipelineLayout(GetDevice(), mPipelineLayout, nullptr);

		// Free the testing texture
		mTextureLoader->DestroyTexture(mTestTexture);

		for (int i = 0; i < mModels.size(); i++) {
			delete mModels[i].object;
		}

		vkDestroyFence(GetDevice(), mRenderFence, nullptr);

		// [TODO] Cleanup rendering command buffers
		vkFreeCommandBuffers(GetDevice(), mCommandPool, 1, &mPrimaryCommandBuffer);

		delete mTextureLoader;
	}

	void VulkanApp::Prepare()
	{
		VulkanBase::Prepare();

		// Create a fence for synchronization
		VkFenceCreateInfo fenceCreateInfo = vkTools::initializers::fenceCreateInfo(VK_FLAGS_NONE);
		vkCreateFence(GetDevice(), &fenceCreateInfo, NULL, &mRenderFence);

		SetupVertexDescriptions();			
		SetupDescriptorSetLayout();			// Must run before PreparePipelines() (VkPipelineLayout)
		PreparePipelines();
		PrepareUniformBuffers();			// Must run before SetupDescriptorSet() (Creates the uniform buffer)

		mTextureLoader = new TextureLoader(mVulkanDevice, mQueue);
		mTextureLoader->LoadTexture("data/textures/crate.jpg", &mTestTexture);

		SetupDescriptorPool();
		SetupDescriptorSet();
		PrepareCommandBuffers();

		mPrepared = true;
	}

	void VulkanApp::PrepareCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocateInfo = CreateInfo::CommandBuffer(mCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

		// Create the primary command buffer
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(GetDevice(), &allocateInfo, &mPrimaryCommandBuffer));

		// Create the secondary command buffer
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(GetDevice(), &allocateInfo, &mSecondaryCommandBuffer));
	}

	void VulkanApp::CompileShaders()
	{
		system("cd data/shaders/phong/ && generate-spirv.bat");
	}

	void VulkanApp::SetCamera(Camera * camera)
	{
		mCamera = camera;
	}

	void VulkanApp::AddModel(VulkanModel model)
	{
		mModels.push_back(model);
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

		// Creates a VkBuffer and maps it to a VkMemory (VulkanBase::CreateBuffer())
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

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = CreateInfo::PipelineLayout(1, &mDescriptorSet.setLayout);

		// Add push constants for the MVP matrix
		VkPushConstantRange pushConstantRanges = {};
		pushConstantRanges.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRanges.offset = 0;
		pushConstantRanges.size = sizeof(PushConstantBlock);

		pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRanges;

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(GetDevice(), &pPipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
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
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		shaderStages[0] = LoadShader("data/shaders/phong/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = LoadShader("data/shaders/phong/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
			
		mPipeline.Create(GetDevice(), mPipelineLayout, mRenderPass, &mVertexDescription, shaderStages);
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
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkClearValue clearValues[2];
		clearValues[0].color = { 0.2f, 0.2f, 0.8f, 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass;
		renderPassBeginInfo.renderArea.extent.width = GetWindowWidth();
		renderPassBeginInfo.renderArea.extent.height = GetWindowHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = frameBuffer;

		// Begin command buffer recording & the render pass
		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mPrimaryCommandBuffer, &beginInfo));
		vkCmdBeginRenderPass(mPrimaryCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);	// VK_SUBPASS_CONTENTS_INLINE

		//
		// Secondary command buffer
		//
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = mRenderPass;
		inheritanceInfo.framebuffer = frameBuffer;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

		VulkanDebug::ErrorCheck(vkBeginCommandBuffer(mSecondaryCommandBuffer, &commandBufferBeginInfo));

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.width = (float)GetWindowWidth();
		viewport.height = (float)GetWindowHeight();
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(mSecondaryCommandBuffer, 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = GetWindowWidth();
		scissor.extent.height = GetWindowHeight();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(mSecondaryCommandBuffer, 0, 1, &scissor);

		//
		// Testing push constant rendering with different matrices
		//
		for (auto& object : mModels)
		{
			// Bind the rendering pipeline (including the shaders)
			vkCmdBindPipeline(mSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.GetVkHandle());

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(mSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet.descriptorSet, 0, NULL);

			// Push the world matrix constant
			mPushConstants.world = object.object->GetWorldMatrix();
			mPushConstants.worldInvTranspose = object.object->GetWorldInverseTransposeMatrix();
			//mPushConstants.color = object.object->GetColor();
			vkCmdPushConstants(mSecondaryCommandBuffer, mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mPushConstants), &mPushConstants);

			// Bind triangle vertices
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(mSecondaryCommandBuffer, VERTEX_BUFFER_BIND_ID, 1, &object.mesh->vertices.buffer, offsets);		// [TODO] The renderer should group the same object models together
			vkCmdBindIndexBuffer(mSecondaryCommandBuffer, object.mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw indexed triangle	
			//vkCmdSetLineWidth(mSecondaryCommandBuffer, 1.0f);
			vkCmdDrawIndexed(mSecondaryCommandBuffer, object.mesh->GetNumIndices(), 1, 0, 0, 0);
		}

		// End secondary command buffer
		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mSecondaryCommandBuffer));

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(mSecondaryCommandBuffer);

		// This is where multithreaded command buffers can be added
		// ...

		// Execute render commands from the secondary command buffer
		vkCmdExecuteCommands(mPrimaryCommandBuffer, commandBuffers.size(), commandBuffers.data());

		// End command buffer recording & the render pass
		vkCmdEndRenderPass(mPrimaryCommandBuffer);
		VulkanDebug::ErrorCheck(vkEndCommandBuffer(mPrimaryCommandBuffer));
	}

	void VulkanApp::Draw()
	{
		//
		// Transition image format to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		//

		VulkanBase::PrepareFrame();

		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 
		RecordRenderingCommandBuffer(mFrameBuffers[mCurrentBuffer]);

		//
		// Do rendering
		//

		// Submit the recorded draw command buffer to the queue
		VkSubmitInfo submitInfo = {};

		submitInfo.pCommandBuffers = &mPrimaryCommandBuffer;					// Draw commands for the current command buffer

		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &mPresentComplete;							// Waits for swapChain.acquireNextImage to complete
		submitInfo.pSignalSemaphores = &mRenderComplete;						// swapChain.queuePresent will wait for this submit to complete
		VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		submitInfo.pWaitDstStageMask = &stageFlags;

		VulkanDebug::ErrorCheck(vkQueueSubmit(mQueue, 1, &submitInfo, mRenderFence));

		// Wait for fence to signal that all command buffers are ready
		VkResult fenceRes;
		do
		{
			fenceRes = vkWaitForFences(GetDevice(), 1, &mRenderFence, VK_TRUE, 100000000);
		} while (fenceRes == VK_TIMEOUT);

		VulkanDebug::ErrorCheck(fenceRes);
		vkResetFences(GetDevice(), 1, &mRenderFence);


		//
		// Transition image format to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		//

		VulkanBase::SubmitFrame();
	}

	void VulkanApp::Render()
	{
		//if (!prepared)
		//	return;

		vkDeviceWaitIdle(GetDevice());		// [NOTE] Is this really needed? - Yes, the validation layer complains otherwise!

		mCamera->Update();

		if (mPrepared) {
			UpdateUniformBuffers();
			Draw();
		}

		vkDeviceWaitIdle(GetDevice());		// [NOTE] Is this really needed? - Yes, the validation layer complains otherwise!
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