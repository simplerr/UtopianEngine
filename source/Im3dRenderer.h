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

		UNIFORM_BLOCK_BEGIN(ViewportBlock)
			UNIFORM_PARAM(glm::vec2, viewport)
		UNIFORM_BLOCK_END()

		Im3dRenderer(Vk::VulkanApp* vulkanApp, glm::vec2 viewportSize);
		~Im3dRenderer();

		void NewFrame();
		void EndFrame();
		void Render();

	private:
		void UploadVertexData();
		uint32_t GetTotalNumVertices();
	private:
		Vk::VulkanApp* mVulkanApp;
		Vk::CommandBuffer* mCommandBuffer;
		Im3d::VertexData* mMappedVertices;
		SharedPtr<Vk::VertexDescription> mVertexDescription;
		glm::vec2 mViewportSize;

		// Contains all vertices created by Im3d, when rendering offsets are used in this buffer.
		Vk::Buffer mVertexBuffer;

		SharedPtr<Vk::Effect> mLinesEffect;
		SharedPtr<Vk::Effect> mPointsEffect;
		SharedPtr<Vk::Effect> mTrianglesEffect;

		ViewProjection mViewProjectionBlock;
		ViewportBlock mViewportBlock;

		// Temp
		uint32_t mVertexCount;
	};
}
