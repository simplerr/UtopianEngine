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

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define VULKAN_ENABLE_VALIDATION false		// Debug validation layers toggle (affects performance a lot)

#define NUM_OBJECTS 10 // 64 * 4 * 4 * 2

namespace VulkanLib
{
	VulkanApp::VulkanApp() : VulkanBase(VULKAN_ENABLE_VALIDATION)
	{
		srand(time(NULL));
		mCamera = nullptr;
	}

	VulkanApp::~VulkanApp()
	{
		mUniformBuffer.Cleanup(GetDevice());
		mDescriptorPool.Cleanup(GetDevice());
		mDescriptorSet.Cleanup(GetDevice());

		// Cleanup pipeline layout
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

		vkDestroyBuffer(mDevice, mInstanceBuffer.buffer, nullptr);
		vkFreeMemory(mDevice, mInstanceBuffer.memory, nullptr);

		vkDestroyPipeline(mDevice, mPipelines.textured, nullptr);
		vkDestroyPipeline(mDevice, mPipelines.colored, nullptr);
		vkDestroyPipeline(mDevice, mPipelines.starsphere, nullptr);

		// The model loader is responsible for cleaning up the model data
		//mModelLoader.CleanupModels(mDevice);

		// Free the testing texture
		//mTextureLoader->destroyTexture(mTestTexture);

		for (int i = 0; i < mModels.size(); i++) {
			delete mModels[i].object;
		}

		vkDestroyFence(mDevice, mRenderFence, nullptr);

		// [TODO] Cleanup rendering command buffers
		//vkFreeCommandBuffers(mDevice, mCommandPool, mStaticCommandBuffers.size(), mStaticCommandBuffers.data());
	}

	void VulkanApp::Prepare()
	{
		VulkanBase::Prepare();

		// Create a fence for synchronization
		VkFenceCreateInfo fenceCreateInfo = vkTools::initializers::fenceCreateInfo(VK_FLAGS_NONE);
		vkCreateFence(mDevice, &fenceCreateInfo, NULL, &mRenderFence);

		SetupVertexDescriptions();			// Custom
		SetupDescriptorSetLayout();			// Must run before PreparePipelines() (VkPipelineLayout)
		PreparePipelines();
		LoadModels();						// Must run before SetupDescriptorSet() (Loads textures)
		PrepareUniformBuffers();			// Must run before SetupDescriptorSet() (Creates the uniform buffer)
		SetupDescriptorPool();
		SetupDescriptorSet();
		PrepareCommandBuffers();

		mPrepared = true;
	}

	void VulkanApp::PrepareCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocateInfo = CreateInfo::CommandBuffer(mCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

		// Create the primary command buffer
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mPrimaryCommandBuffer));

		// Create the secondary command buffer
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		VulkanDebug::ErrorCheck(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mSecondaryCommandBuffer));
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
		if (mUseInstancing || mUseStaticCommandBuffer)
			mModels.push_back(model);
	}

	void VulkanApp::LoadModels()
	{
		// Load a random testing texture
		//mTextureLoader->loadTexture("data/textures/crate_bc3.dds", VK_FORMAT_BC3_UNORM_BLOCK, &mTestTexture);
	}

	void VulkanApp::PrepareUniformBuffers()
	{
		// Light
		Light light;
		light.SetPosition(150, 150, 150);
		light.SetDirection(1, -1, 0);
		light.SetAtt(1, 1, 0);
		light.SetIntensity(0, 0, 1);
		mUniformBuffer.lights.push_back(light);

		// Important to call this before CreateBuffer() since # lights affects the total size
		mUniformBuffer.constants.numLights = mUniformBuffer.lights.size();

		// Creates a VkBuffer and maps it to a VkMemory (VulkanBase::CreateBuffer())
		mUniformBuffer.CreateBuffer(this, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		UpdateUniformBuffers();
	}

	// Call this every time any uniform buffer should be updated (view changes etc.)
	void VulkanApp::UpdateUniformBuffers()
	{
		if (mCamera != nullptr)
		{
			mUniformBuffer.camera.projectionMatrix = mCamera->GetProjection();
			mUniformBuffer.camera.viewMatrix = mCamera->GetView();
			mUniformBuffer.camera.projectionMatrix = mCamera->GetProjection();
			mUniformBuffer.camera.eyePos = mCamera->GetPosition();
		}

		mUniformBuffer.constants.useInstancing = mUseInstancing;

		mUniformBuffer.UpdateMemory(GetDevice());
	}

	void VulkanApp::SetupDescriptorSetLayout()
	{
		mDescriptorSet.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);				// Uniform buffer binding: 0
		mDescriptorSet.AddLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);		// Combined image sampler binding: 1
		mDescriptorSet.CreateLayout(mDevice);

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = CreateInfo::PipelineLayout(1, &mDescriptorSet.setLayout);

		// Add push constants for the MVP matrix
		VkPushConstantRange pushConstantRanges = {};
		pushConstantRanges.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		pushConstantRanges.offset = 0;
		pushConstantRanges.size = sizeof(PushConstantBlock);

		pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRanges;

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
	}

	void VulkanApp::SetupDescriptorPool()
	{
		mDescriptorPool.CreatePoolFromLayout(mDevice, mDescriptorSet.GetLayoutBindings());
	}

	// [TODO] Let each thread have a seperate descriptor set!!
	void VulkanApp::SetupDescriptorSet()
	{
		mDescriptorSet.AllocateDescriptorSets(mDevice, mDescriptorPool.GetVkDescriptorPool());
		mDescriptorSet.BindUniformBuffer(0, &mUniformBuffer.GetDescriptor());
		mDescriptorSet.BindCombinedImage(1, &GetTextureDescriptorInfo(mTestTexture));
		mDescriptorSet.UpdateDescriptorSets(mDevice);
	}

	void VulkanApp::PreparePipelines()
	{
		// The pipeline consists of many stages, where each stage can have different states
		// Creating a pipeline is simply defining the state for every stage (and some more...)
		// ...

		// Input assembly state
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;

		// Color blend state
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
		blendAttachmentState[0].colorWriteMask = 0xf;
		blendAttachmentState[0].blendEnable = VK_FALSE;			// Blending disabled
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = blendAttachmentState;

		// Viewport state
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Dynamic state for the viewport so the pipeline don't have to be recreated when resizing the window
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = dynamicStateEnables.size();

		// Depth and stencil state
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;			// Stencil disabled
		depthStencilState.front = depthStencilState.back;

		// Multi sampling state
		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pSampleMask = NULL;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;		// Multi sampling not used

		// Load shader
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		shaderStages[0] = LoadShader("data/shaders/phong/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = LoadShader("data/shaders/phong/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		// Assign all the states to the pipeline
		// The states will be static and can't be changed after the pipeline is created
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = mPipelineLayout;
		pipelineCreateInfo.renderPass = mRenderPass;
		pipelineCreateInfo.pVertexInputState = &mVertexDescription.GetInputState();		// From base - &vertices.inputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();

		// Create the colored pipeline	
		//rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipelines.colored));
	}

	void VulkanApp::SetupVertexDescriptions()
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription.AddBinding(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);					// Per vertex

		if (mUseInstancing)
			mVertexDescription.AddBinding(INSTANCE_BUFFER_BIND_ID, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE);	// Per instance

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
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 0.0f };
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

		// Secondary command buffer for the sky sphere
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
			vkCmdBindPipeline(mSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.pipeline);

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(mSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet.descriptorSet, 0, NULL);

			// Push the world matrix constant
			mPushConstants.world = object.object->GetWorldMatrix(); // camera->GetProjection() * camera->GetView() * 
			mPushConstants.color = object.object->GetColor();
			vkCmdPushConstants(mSecondaryCommandBuffer, mPipelineLayout, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, sizeof(PushConstantBlock), &mPushConstants);

			// Bind triangle vertices
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(mSecondaryCommandBuffer, VERTEX_BUFFER_BIND_ID, 1, &object.mesh->vertices.buffer, offsets);		// [TODO] The renderer should group the same object models together
			vkCmdBindIndexBuffer(mSecondaryCommandBuffer, object.mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw indexed triangle	
			vkCmdSetLineWidth(mSecondaryCommandBuffer, 1.0f);
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
			fenceRes = vkWaitForFences(mDevice, 1, &mRenderFence, VK_TRUE, 100000000);
		} while (fenceRes == VK_TIMEOUT);

		VulkanDebug::ErrorCheck(fenceRes);
		vkResetFences(mDevice, 1, &mRenderFence);


		//
		// Transition image format to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		//

		VulkanBase::SubmitFrame();
	}

	void VulkanApp::Render()
	{
		//if (!prepared)
		//	return;

		vkDeviceWaitIdle(mDevice);		// [NOTE] Is this really needed? - Yes, the validation layer complains otherwise!

		mCamera->Update();

		if (mPrepared) {
			UpdateUniformBuffers();
			Draw();
		}

		vkDeviceWaitIdle(mDevice);		// [NOTE] Is this really needed? - Yes, the validation layer complains otherwise!
	}

	void VulkanApp::Update()
	{
		return;

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