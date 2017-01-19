#pragma once
#include <string>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

#define VERTEX_SHADER_IDX 0
#define PIXEL_SHADER_IDX 1

namespace VulkanLib
{
	class Device;

	class Shader
	{
	public:
		Shader(VkPipelineShaderStageCreateInfo vertexShaderCreateInfo, VkPipelineShaderStageCreateInfo pixelShaderCreateInfo);

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	};

	class ShaderManager
	{
	public:
		ShaderManager(Device* device);
		~ShaderManager();
		Shader* CreateShader(std::string vertexShaderFilename, std::string pixelShaderFilename);
	private:
		VkShaderModule LoadShader(std::string filename, VkShaderStageFlagBits stage);
	private:
		std::vector<Shader*> mLoadedShaders;
		Device* mDevice;
	};
}
