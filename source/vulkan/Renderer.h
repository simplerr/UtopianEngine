#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/VulkanInclude.h"
#include "VulkanBase.h"
#include "VertexDescription.h"
#include "ShaderBuffer.h"

using namespace glm;

namespace ECS
{
	class RenderSystem;
}

namespace Vulkan
{
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

		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();
		void PrepareCommandBuffers();						

		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);

		virtual void Render();
		virtual void Update();
		void Draw();

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 
		//	High level code
		//
		void CompileShaders();

		//Pipeline* GetPipeline(PipelineType pipelineType);
		DescriptorSetLayout* GetTextureDescriptorSetLayout();
		DescriptorPool* GetDescriptorPool();

		CommandBuffer* CreateCommandBuffer(VkCommandBufferLevel level);

		void SetCamera(Camera* camera);

	private:
		bool							mPrepared = false;

		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		
		CommandBuffer*					mPrimaryCommandBuffer;
		std::vector<CommandBuffer*>		mApplicationCommandBuffers;
		Fence*							mRenderFence;

		/* 
			They descriptor set layout for textures is handled by Renderer
			NOTE: Right now they have to be bound to binding=0	
		*/
		// TODO: Should not be here
		DescriptorSetLayout*			mTextureDescriptorSetLayout;
		DescriptorPool*					mDescriptorPool;

		Camera*							mCamera;
		TextOverlay*					mTextOverlay;

		// TODO: NOTE: HACK
	public:
		ShaderManager*					mShaderManager = nullptr;
		TextureLoader*					mTextureLoader = nullptr;
		ModelLoader*					mModelLoader = nullptr;
	};
}	// VulkanLib namespace
