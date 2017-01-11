#pragma once
#include "FragmentUniformBuffer.h"
#include "VulkanBase.h"
#include "ModelLoader.h"
#include "StaticModel.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VertexDescription.h"
#include "UniformBuffer.h"
#include "VertexUniformBuffer.h"
#include "DescriptorSet.h"
#include "TextureLoader.h"
#include "Pipeline.h"

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
		VkPipeline phong;
	};

	struct PushConstantBlock {
		mat4 world;
		mat4 worldInvTranspose;
		//vec3 color;
	};

	struct VulkanModel
	{
		Object* object;
		StaticModel* mesh;
		VkPipeline pipeline;
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
		void AddModel(VulkanModel model);

		//Pipelines						mPipelines;
		VkPipelineLayout				mPipelineLayout;

		// This gets regenerated each frame so there is no need for command buffer per frame buffer
		VkCommandBuffer					mPrimaryCommandBuffer;
		VkCommandBuffer					mSecondaryCommandBuffer;

		VkFence							mRenderFence = {};

		// 
		//	High level code
		//

		PushConstantBlock				mPushConstants;						// Gets updated with new push constants for each object

		bool							mPrepared = false;

		Camera*							mCamera;

		std::vector<VulkanModel>		mModels;

		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		VertexDescription				mVertexDescription;
		VertexUniformBuffer				mVertexUniformBuffer;
		FragmentUniformBuffer			mFragmentUniformBuffer;
		DescriptorPool					mDescriptorPool;
		DescriptorSet					mDescriptorSet;
		
		TextureLoader*					mTextureLoader;
		VulkanTexture					mTestTexture;
		Pipeline						mPipeline;

	public:
	};
}	// VulkanLib namespace