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

namespace Utopian::Vk
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
		PushConstantBlock() {
			world = mat4();
			worldInvTranspose = mat4();
		}

		PushConstantBlock(mat4 w) {
			world = w;
			worldInvTranspose = mat4();

			// Note: This needs to be done to have the physical world match the rendered world.
			// See https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/ for more information.
			world[3][0] = -world[3][0];
			world[3][1] = -world[3][1];
			world[3][2] = -world[3][2];
		}
		
		mat4 world;
		mat4 worldInvTranspose;
	};

	class Renderer : public VulkanBase
	{
	public:
		Renderer();
		~Renderer();

		void Prepare();
		void PostInitPrepare();

		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();
		void PrepareCommandBuffers();						

		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);

		virtual void Render();
		virtual void Update();
		void Draw();

		void AddScreenQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Image* image, Utopian::Vk::Sampler* sampler);
		void AddScreenQuad(uint32_t left, uint32_t top, uint32_t width, uint32_t height, Utopian::Vk::Texture* texture);
		void UpdateOverlay();

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void BeginUiUpdate();
		void EndUiUpdate();

		// 
		//	High level code
		//
		void CompileShaders();

		//Pipeline* GetPipeline(PipelineType pipelineType);
		DescriptorSetLayout* GetTextureDescriptorSetLayout();
		DescriptorPool* GetDescriptorPool();

		CommandBuffer* CreateCommandBuffer(VkCommandBufferLevel level);

		void SetCamera(Camera* camera);
		Camera* GetCamera();

		void SetClearColor(glm::vec4 color);
		glm::vec4 GetClearColor();

	private:
		bool							mPrepared = false;

		// We are assuming that the same Vertex structure is used everywhere since there only is 1 pipeline right now
		// inputState will have pointers to the binding and attribute descriptions after PrepareVertices()
		// inputState is the pVertexInputState when creating the graphics pipeline
		
		CommandBuffer*					mPrimaryCommandBuffer;
		CommandBuffer*					mScreenGuiCommandBuffer;
		std::vector<CommandBuffer*>		mApplicationCommandBuffers;

		/* 
			They descriptor set layout for textures is handled by Renderer
			NOTE: Right now they have to be bound to binding=0	
		*/
		// TODO: Should not be here
		DescriptorSetLayout*			mTextureDescriptorSetLayout;
		DescriptorPool*					mDescriptorPool;

		Camera*							mCamera;
		TextOverlay*					mTextOverlay;
		glm::vec4						mClearColor;

		// TODO: NOTE: HACK
	public:
		ShaderManager*					mShaderManager = nullptr;
		TextureLoader*					mTextureLoader = nullptr;
		ModelLoader*					mModelLoader = nullptr;
		UIOverlay*						mUiOverlay = nullptr;
		ScreenGui*						mScreenGui = nullptr;

	};
}	// VulkanLib namespace
