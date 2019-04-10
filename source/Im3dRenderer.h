#pragma once
#include "vulkan/vulkaninclude.h"
#include "im3d/im3d.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"
#include "glm/glm.hpp"

namespace Utopian
{
	class Im3dRenderer
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_BLOCK_END()

		Im3dRenderer(Vk::VulkanApp* vulkanApp, glm::vec2 viewportSize);
		~Im3dRenderer();

		void NewFrame();
		void EndFrame();
		void Render();

	private:
		void UploadVertexData();
	private:
		Vk::VulkanApp* mVulkanApp;
		Vk::CommandBuffer* mCommandBuffer;
		SharedPtr<Vk::Effect> mEffect;
		ViewProjection mViewProjectionBlock;
		Vk::Buffer mVertexBuffer;
		Im3d::VertexData* mMappedVertices;
		SharedPtr<Vk::VertexDescription> mVertexDescription;
		glm::vec2 mViewportSize;

		// Temp
		uint32_t mVertexCount;
	};
}
