#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include "FragmentUniformBuffer.h"
#include "VulkanBase.h"
#include "VertexDescription.h"
#include "UniformBuffer.h"
#include "VertexUniformBuffer.h"
#include "handles/DescriptorSet.h"

using namespace glm;

namespace ECS
{
	class RenderSystem;
}

namespace VulkanLib
{
	class Camera;
	class CommandBuffer;
	class Pipeline;
	class PipelineLayout;
	class Fence;
	class DescriptorSetLayout;

	enum PipelineType
	{
		PIPELINE_BASIC,
		PIPELINE_WIREFRAME,
		PIPELINE_TEST,
		PIPELINE_DEBUG
	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	};

	struct InstanceData {
		vec3 position;
		vec3 scale;
		vec3 color;
	};

	struct PushConstantBlock {
		mat4 world;
		mat4 worldInvTranspose;
		//vec3 color;
	};

	class VulkanApp : public VulkanBase
	{
	public:
		VulkanApp();
		~VulkanApp();

		void Prepare();

		void PrepareUniformBuffers();
		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();
		void SetupDescriptorSet();
		void PreparePipelines();
		void UpdateUniformBuffers();
		void PrepareCommandBuffers();						
		void SetupVertexDescriptions();

		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);

		virtual void Render();
		virtual void Update();
		void Draw();

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 
		//	High level code
		//
		void CompileShaders();
		void SetCamera(Camera* camera);

		Camera* GetCamera();
		Device* GetDeviceTmp() { return mDevice; } // [NOTE] [TODO] A hack to get the model loading in Game.cpp to workVulkanApp
		Pipeline* GetPipeline(PipelineType pipelineType);
		PipelineLayout* GetPipelineLayout();
		DescriptorSetLayout* GetTextureDescriptorSetLayout();
		DescriptorPool* GetDescriptorPool();

		CommandBuffer* CreateCommandBuffer(VkCommandBufferLevel level);

		// 
		//	High level code
		//

		PushConstantBlock				mPushConstants;						// Gets updated with new push constants for each object
		bool							mPrepared = false;
		Camera*							mCamera;

		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		VertexDescription				mVertexDescription;
		VertexUniformBuffer				mVertexUniformBuffer;
		FragmentUniformBuffer			mFragmentUniformBuffer;
		
		Pipeline*						mSolidPipeline;
		PipelineLayout*					mPipelineLayout;
		CommandBuffer*					mPrimaryCommandBuffer;
		CommandBuffer*					mSecondaryCommandBuffer; 
		CommandBuffer*					mDebugCommandBuffer;
		std::vector<CommandBuffer*>		mApplicationCommandBuffers;
		Fence*							mRenderFence;

		std::map<int, Pipeline*>		mPipelines;

		ECS::RenderSystem*				mRenderSystem;

		// Divide descriptor set
		DescriptorPool*					mDescriptorPool;
		DescriptorSet*					mCameraDescriptorSet;
		DescriptorSet*					mLightDescriptorSet;

		DescriptorSetLayout*			mCameraDescriptorSetLayout;
		DescriptorSetLayout*			mLightDescriptorSetLayout;
		DescriptorSetLayout*			mTextureDescriptorSetLayout;

		const uint32_t					MAX_NUM_TEXTURES = 10;
	public:
	};
}	// VulkanLib namespace
