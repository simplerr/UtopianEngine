#pragma once

#include <string>
#include <vector>
#include "vulkan/VulkanInclude.h"

#define VERTEX_SHADER_IDX 0
#define PIXEL_SHADER_IDX 1

namespace Utopian::Vk
{
	class ShaderCreateInfo
	{
	public:
		void AddVertexShaderSource(std::string source);
		void AddPixelShaderSource(std::string source);
		void AddGeometryShaderSource(std::string source);
	private:
		std::vector<std::string> mVertexShaderSources;
		std::vector<std::string> mPixelShaderSources;
		std::vector<std::string> mGeometryShaderSources;
	};

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
		Shader* CreateShaderOnline(std::string vertexShaderFilename, std::string pixelShaderFilename, std::string geometryShaderFilename = "NONE");
		Shader* CreateComputeShader(std::string computeShaderFilename);
		const std::vector<unsigned int> CompileShader(std::string filename);
	private:
		VkShaderModule LoadShader(std::string filename, VkShaderStageFlagBits stage);
	private:
		std::vector<Shader*> mLoadedShaders;
		Device* mDevice;
	};
}
