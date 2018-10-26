#pragma once

#include <string>
#include <vector>
#include <map>
#include "vulkan/VulkanInclude.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include <glslang/Public/ShaderLang.h>

#define VERTEX_SHADER_IDX 0
#define PIXEL_SHADER_IDX 1

namespace Utopian::Vk
{
	enum UniformVariableType
	{
		UVT_SSBO,
		UVT_SAMPLER1D,
		UVT_SAMPLER2D,
		UVT_SAMPLER3D
	};

	/* Can be SSBOs and samplers */
	struct UniformVariableDesc
	{
		UniformVariableType type;
		std::string name;
		int set;
		int binding;
	};

	/* UBOs */
	struct UniformBlockDesc
	{
		std::string name;
		int set;
		int binding;
		int size;
	};

	struct ShaderReflection
	{
		std::map<std::string, UniformBlockDesc> uniformBlocks;
		std::map<std::string, UniformVariableDesc> combinedSamplers;
	};

	struct CompiledShader
	{
		std::vector<unsigned int> spirvBytecode;
		ShaderReflection reflection;
		VkShaderStageFlagBits shaderStage;
		VkShaderModule shaderModule;
	};

	class Shader
	{
	public:
		Shader();

		void AddShaderStage(VkPipelineShaderStageCreateInfo shaderStageCreateInfo);
		void AddCompiledShader(CompiledShader compiledShader);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<CompiledShader> compiledShaders;
	};

	class ShaderFactory : public Module<ShaderFactory>
	{
	public:
		ShaderFactory(Device* device);
		~ShaderFactory();
		Shader* CreateShader(std::string vertexShaderFilename, std::string pixelShaderFilename, std::string geometryShaderFilename = "NONE");
		Shader* CreateComputeShader(std::string computeShaderFilename);
		SharedPtr<Shader> CreateShaderOnline(std::string vertexShaderFilename, std::string pixelShaderFilename, std::string geometryShaderFilename = "NONE");
		CompiledShader CompileShader(std::string filename);
	private:
		ShaderReflection ExtractShaderLayout(glslang::TProgram& program);
		VkShaderModule LoadShader(std::string filename, VkShaderStageFlagBits stage);
	private:
		std::vector<Shader*> mLoadedShaders;
		Device* mDevice;
	};

	ShaderFactory& gShaderManager();
}
