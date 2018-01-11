#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "vulkan/VulkanInclude.h"

#define VERTEX_SHADER_IDX 0
#define PIXEL_SHADER_IDX 1

namespace Vulkan
{
	class Shader
	{
	public:
		Shader();

		void AddShaderStage(VkPipelineShaderStageCreateInfo shaderStageCreateInfo);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	};

	class ShaderManager
	{
	public:
		ShaderManager(Device* device);
		~ShaderManager();
		Shader* CreateShader(std::string vertexShaderFilename, std::string pixelShaderFilename, std::string geometryShaderFilename = "NONE");
		Shader* CreateComputeShader(std::string computeShaderFilename);
	private:
		VkShaderModule LoadShader(std::string filename, VkShaderStageFlagBits stage);
	private:
		std::vector<Shader*> mLoadedShaders;
		Device* mDevice;
	};
}
