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

using namespace glm;

namespace ECS
{
	class RenderSystem;
}

namespace Vulkan
{
	class Camera;
	class CommandBuffer;
	class Pipeline;
	class PipelineLayout;
	class Fence;
	class DescriptorSetLayout;
	class DescriptorSet;
	class DescriptorPool;
	class TextOverlay;
	class TextureLoader;

	enum PipelineType
	{
		PIPELINE_BASIC,
		PIPELINE_WIREFRAME,
		PIPELINE_TEST,
		PIPELINE_DEBUG
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

	class Renderer : public VulkanBase
	{
	public:
		Renderer();
		~Renderer();

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

		Pipeline* GetPipeline(PipelineType pipelineType);
		PipelineLayout* GetPipelineLayout();
		DescriptorSetLayout* GetTextureDescriptorSetLayout();
		DescriptorPool* GetDescriptorPool();
		VertexDescription* GetVertexDescription();

		CommandBuffer* CreateCommandBuffer(VkCommandBufferLevel level);

		void SetCamera(Camera* camera);

	public:
		// NOTE: These should probably be moved to RenderSystem
		DescriptorSet*					mCameraDescriptorSet;
		DescriptorSet*					mLightDescriptorSet;

	private:
		bool							mPrepared = false;

		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		VertexDescription				mVertexDescription;
		VertexUniformBuffer				mVertexUniformBuffer;
		FragmentUniformBuffer			mFragmentUniformBuffer;
		
		PipelineLayout*					mPipelineLayout;
		CommandBuffer*					mPrimaryCommandBuffer;
		CommandBuffer*					mSecondaryCommandBuffer; 
		std::vector<CommandBuffer*>		mApplicationCommandBuffers;
		Fence*							mRenderFence;

		std::map<int, Pipeline*>		mPipelines;

		DescriptorPool*					mDescriptorPool;

		DescriptorSetLayout*			mCameraDescriptorSetLayout;
		DescriptorSetLayout*			mLightDescriptorSetLayout;

		/* 
			They descriptor set layout for textures is handled by Renderer
			NOTE: Right now they have to be bound to binding=0	
		*/
		DescriptorSetLayout*			mTextureDescriptorSetLayout;

		const uint32_t					MAX_NUM_TEXTURES = 64;

		Camera*							mCamera;

		TextOverlay*					mTextOverlay;

		// TODO: NOTE: HACK
	public:
		ShaderManager*					mShaderManager = nullptr;
		TextureLoader*					mTextureLoader = nullptr;
	};
}	// VulkanLib namespace
