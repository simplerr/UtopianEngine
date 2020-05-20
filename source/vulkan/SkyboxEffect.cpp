#include "vulkan/SkyboxEffect.h"
#include "vulkan/ShaderFactory.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian::Vk
{
	SkyboxEffect::SkyboxEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass)
	{
		ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/skybox/skybox.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/skybox/skybox.frag";
		SetShaderCreateInfo(shaderCreateInfo);

		viewProjectionBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

		CreatePipeline();

		BindUniformBuffer("UBO_viewProjection", viewProjectionBlock);
	}

	void SkyboxEffect::SetCameraData(glm::mat4 view, glm::mat4 projection)
	{ 
		// Removes the translation components of the matrix to always keep the skybox at the same distance
		viewProjectionBlock.data.view = glm::mat4(glm::mat3(view));
		viewProjectionBlock.data.projection = projection;
		viewProjectionBlock.data.world = glm::scale(glm::mat4(), glm::vec3(10000.0f));
		UpdateMemory();
	}

	void SkyboxEffect::UpdateMemory()
	{
		viewProjectionBlock.UpdateMemory();
	}
}