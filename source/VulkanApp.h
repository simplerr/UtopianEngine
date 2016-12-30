#pragma once
#include "VulkanBase.h"
#include "ModelLoader.h"
#include "ThreadPool.h"
#include "StaticModel.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VertexDescription.h"
#include "UniformBuffer.h"
#include "BigUniformBuffer.h"
#include "DescriptorSet.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

using namespace glm;

namespace VulkanLib
{
	class StaticModel;
	class Camera;
	class Object;
	class TextureData;
	class Light;

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

	struct Pipelines {
		VkPipeline textured;
		VkPipeline colored;
		VkPipeline starsphere;
		VkPipeline instanced;
	};

	struct PushConstantBlock {
		mat4 world;
		vec3 color;
	};

	struct VulkanModel
	{
		Object* object;
		StaticModel* mesh;
		VkPipeline pipeline;
	};

	struct ThreadData {
		VkCommandBuffer commandBuffer;
		VkCommandPool commandPool;
		std::vector<VulkanModel> threadObjects;

		DescriptorPool descriptorPool1;
		DescriptorSet descriptorSet;
		PushConstantBlock pushConstants;

		StaticModel model;
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
		void PrepareCommandBuffers();						// Custom
		void SetupVertexDescriptions();

		void SetupMultithreading(int numThreads);			// Custom
		void EnableInstancing(bool useInstancing);
		void EnableStaticCommandBuffers(bool useStaticCommandBuffers);
		void PrepareInstancing();

		void RecordStaticCommandBuffers();
		void BuildInstancingCommandBuffer(VkFramebuffer frameBuffer);
		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);
		void ThreadRecordCommandBuffer(int threadId, VkCommandBufferInheritanceInfo inheritanceInfo);

		virtual void Render();
		virtual void Update();
		void Draw();

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 
		//	High level code
		//
		void LoadModels();
		void CompileShaders();
		void SetCamera(Camera* camera);

		void AddModel(VulkanModel model);

		Pipelines						mPipelines;
		VkPipelineLayout				mPipelineLayout;

		// This gets regenerated each frame so there is no need for command buffer per frame buffer
		VkCommandBuffer					mPrimaryCommandBuffer;
		VkCommandBuffer					mSecondaryCommandBuffer;
		std::vector<VkCommandBuffer>	mStaticCommandBuffers;

		VkFence							mRenderFence = {};

		// 
		//	High level code
		//

		PushConstantBlock				mPushConstants;						// Gets updated with new push constants for each object

		vkTools::VulkanTexture			mTestTexture;						// NOTE: just for testing
		vkTools::VulkanTexture			mTerrainTexture;					// Testing for the terrain

		bool							mPrepared = false;

		Buffer							mInstanceBuffer;
		bool							mUseInstancing = false;
		bool							mUseStaticCommandBuffer = false;

		Camera*							mCamera;

		// Threads
		std::vector<ThreadData>			mThreadData;
		int								mNumThreads;
		int								mNumObjects;
		ThreadPool						mThreadPool;

		std::vector<VulkanModel>		mModels;

		int								mNextThreadId = 0;					// The thread to add new objects to

																			// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
																			// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
																			// inputState is the pVertexInputState when creating the graphics pipeline
		VertexDescription				mVertexDescription;
		BigUniformBuffer				mUniformBuffer;
		DescriptorPool					mDescriptorPool;
		DescriptorSet					mDescriptorSet;

	public:
		StaticModel*					mTestModel;
	};
}	// VulkanLib namespace
