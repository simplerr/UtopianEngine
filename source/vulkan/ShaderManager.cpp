#include <cassert>
#include <fstream>
#include <iostream>
#include "ShaderManager.h"
#include "Device.h"
#include "VulkanDebug.h"
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <DirStackFileIncluder.h>
#include <ResourceLimits.h>

namespace Utopian::Vk
{
	const TBuiltInResource DefaultTBuiltInResource = {
		/* .MaxLights = */ 32,
		/* .MaxClipPlanes = */ 6,
		/* .MaxTextureUnits = */ 32,
		/* .MaxTextureCoords = */ 32,
		/* .MaxVertexAttribs = */ 64,
		/* .MaxVertexUniformComponents = */ 4096,
		/* .MaxVaryingFloats = */ 64,
		/* .MaxVertexTextureImageUnits = */ 32,
		/* .MaxCombinedTextureImageUnits = */ 80,
		/* .MaxTextureImageUnits = */ 32,
		/* .MaxFragmentUniformComponents = */ 4096,
		/* .MaxDrawBuffers = */ 32,
		/* .MaxVertexUniformVectors = */ 128,
		/* .MaxVaryingVectors = */ 8,
		/* .MaxFragmentUniformVectors = */ 16,
		/* .MaxVertexOutputVectors = */ 16,
		/* .MaxFragmentInputVectors = */ 15,
		/* .MinProgramTexelOffset = */ -8,
		/* .MaxProgramTexelOffset = */ 7,
		/* .MaxClipDistances = */ 8,
		/* .MaxComputeWorkGroupCountX = */ 65535,
		/* .MaxComputeWorkGroupCountY = */ 65535,
		/* .MaxComputeWorkGroupCountZ = */ 65535,
		/* .MaxComputeWorkGroupSizeX = */ 1024,
		/* .MaxComputeWorkGroupSizeY = */ 1024,
		/* .MaxComputeWorkGroupSizeZ = */ 64,
		/* .MaxComputeUniformComponents = */ 1024,
		/* .MaxComputeTextureImageUnits = */ 16,
		/* .MaxComputeImageUniforms = */ 8,
		/* .MaxComputeAtomicCounters = */ 8,
		/* .MaxComputeAtomicCounterBuffers = */ 1,
		/* .MaxVaryingComponents = */ 60,
		/* .MaxVertexOutputComponents = */ 64,
		/* .MaxGeometryInputComponents = */ 64,
		/* .MaxGeometryOutputComponents = */ 128,
		/* .MaxFragmentInputComponents = */ 128,
		/* .MaxImageUnits = */ 8,
		/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
		/* .MaxCombinedShaderOutputResources = */ 8,
		/* .MaxImageSamples = */ 0,
		/* .MaxVertexImageUniforms = */ 0,
		/* .MaxTessControlImageUniforms = */ 0,
		/* .MaxTessEvaluationImageUniforms = */ 0,
		/* .MaxGeometryImageUniforms = */ 0,
		/* .MaxFragmentImageUniforms = */ 8,
		/* .MaxCombinedImageUniforms = */ 8,
		/* .MaxGeometryTextureImageUnits = */ 16,
		/* .MaxGeometryOutputVertices = */ 256,
		/* .MaxGeometryTotalOutputComponents = */ 1024,
		/* .MaxGeometryUniformComponents = */ 1024,
		/* .MaxGeometryVaryingComponents = */ 64,
		/* .MaxTessControlInputComponents = */ 128,
		/* .MaxTessControlOutputComponents = */ 128,
		/* .MaxTessControlTextureImageUnits = */ 16,
		/* .MaxTessControlUniformComponents = */ 1024,
		/* .MaxTessControlTotalOutputComponents = */ 4096,
		/* .MaxTessEvaluationInputComponents = */ 128,
		/* .MaxTessEvaluationOutputComponents = */ 128,
		/* .MaxTessEvaluationTextureImageUnits = */ 16,
		/* .MaxTessEvaluationUniformComponents = */ 1024,
		/* .MaxTessPatchComponents = */ 120,
		/* .MaxPatchVertices = */ 32,
		/* .MaxTessGenLevel = */ 64,
		/* .MaxViewports = */ 16,
		/* .MaxVertexAtomicCounters = */ 0,
		/* .MaxTessControlAtomicCounters = */ 0,
		/* .MaxTessEvaluationAtomicCounters = */ 0,
		/* .MaxGeometryAtomicCounters = */ 0,
		/* .MaxFragmentAtomicCounters = */ 8,
		/* .MaxCombinedAtomicCounters = */ 8,
		/* .MaxAtomicCounterBindings = */ 1,
		/* .MaxVertexAtomicCounterBuffers = */ 0,
		/* .MaxTessControlAtomicCounterBuffers = */ 0,
		/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
		/* .MaxGeometryAtomicCounterBuffers = */ 0,
		/* .MaxFragmentAtomicCounterBuffers = */ 1,
		/* .MaxCombinedAtomicCounterBuffers = */ 1,
		/* .MaxAtomicCounterBufferSize = */ 16384,
		/* .MaxTransformFeedbackBuffers = */ 4,
		/* .MaxTransformFeedbackInterleavedComponents = */ 64,
		/* .MaxCullDistances = */ 8,
		/* .MaxCombinedClipAndCullDistances = */ 8,
		/* .MaxSamples = */ 4,
		/* .maxMeshOutputVerticesNV = */ 0,		// Note: Set to 0
		/* .maxMeshOutputPrimitivesNV = */ 0,
		/* .maxMeshWorkGroupSizeX_NV = */ 0,
		/* .maxMeshWorkGroupSizeY_NV = */ 0,
		/* .maxMeshWorkGroupSizeZ_NV = */ 0,
		/* .maxTaskWorkGroupSizeX_NV = */ 0,
		/* .maxTaskWorkGroupSizeY_NV = */ 0,
		/* .maxTaskWorkGroupSizeZ_NV = */ 0,
		/* .maxMeshViewCountNV = */ 0,
		/* .limits = */{
			/* .nonInductiveForLoops = */ 1,
			/* .whileLoops = */ 1,
			/* .doWhileLoops = */ 1,
			/* .generalUniformIndexing = */ 1,
			/* .generalAttributeMatrixVectorIndexing = */ 1,
			/* .generalVaryingIndexing = */ 1,
			/* .generalSamplerIndexing = */ 1,
			/* .generalVariableIndexing = */ 1,
			/* .generalConstantMatrixVectorIndexing = */ 1,
		}
	};

	Shader::Shader()
	{

	}

	void Shader::AddShaderStage(VkPipelineShaderStageCreateInfo shaderStageCreateInfo)
	{
		shaderStages.push_back(shaderStageCreateInfo);
	}
	
	ShaderManager::ShaderManager(Device* device)
		: mDevice(device)
	{

	}

	ShaderManager::~ShaderManager()
	{
		// TODO: Fix this
		for (int i = 0; i < mLoadedShaders.size(); i++)
		{
			for (int j = 0; j < mLoadedShaders[i]->shaderStages.size(); j++)
			{
				vkDestroyShaderModule(mDevice->GetVkDevice(), mLoadedShaders[i]->shaderStages[j].module, nullptr);
			}

			delete mLoadedShaders[i];
		}
	}

	Shader* ShaderManager::CreateShader(std::string vertexShaderFilename, std::string pixelShaderFilename, std::string geometryShaderFilename)
	{
		// Vertex shader
		VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
		vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderCreateInfo.pName = "main";
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderCreateInfo.module = LoadShader(vertexShaderFilename.c_str(), VK_SHADER_STAGE_VERTEX_BIT);

		// Pixel shader
		VkPipelineShaderStageCreateInfo pixelShaderCreateInfo = {};
		pixelShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pixelShaderCreateInfo.pName = "main";
		pixelShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pixelShaderCreateInfo.module = LoadShader(pixelShaderFilename.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

		Shader* shader = new Shader();

		shader->AddShaderStage(vertexShaderCreateInfo);
		shader->AddShaderStage(pixelShaderCreateInfo);

		VkPipelineShaderStageCreateInfo geometryShaderCreateInfo = {};
		if (geometryShaderFilename != "NONE")
		{
			geometryShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			geometryShaderCreateInfo.pName = "main";
			geometryShaderCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			geometryShaderCreateInfo.module = LoadShader(geometryShaderFilename.c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);

			shader->AddShaderStage(geometryShaderCreateInfo);
		}

		mLoadedShaders.push_back(shader);
		
		return shader;
	}

	std::string GetFilePath(const std::string& str)
	{
		size_t found = str.find_last_of("/\\");
		return str.substr(0, found);
	}

	std::string GetSuffix(const std::string& name)
	{
		const size_t pos = name.rfind('.');
		return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
	}

	EShLanguage GetShaderStage(const std::string& stage)
	{
		if (stage == "vert") {
			return EShLangVertex;
		}
		else if (stage == "tesc") {
			return EShLangTessControl;
		}
		else if (stage == "tese") {
			return EShLangTessEvaluation;
		}
		else if (stage == "geom") {
			return EShLangGeometry;
		}
		else if (stage == "frag") {
			return EShLangFragment;
		}
		else if (stage == "comp") {
			return EShLangCompute;
		}
		else {
			assert(0 && "Unknown shader stage");
			return EShLangCount;
		}
	}

	const std::vector<unsigned int> ShaderManager::CompileShader(std::string filename)
	{
		glslang::InitializeProcess();

		std::ifstream file(filename);

		std::string glslString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const char* glslSource = glslString.c_str();

		EShLanguage shaderType = GetShaderStage(GetSuffix(filename));

		glslang::TShader shader(shaderType);
		shader.setStrings(&glslSource, 1);

		int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
		glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
		glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

		shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
		shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
		TBuiltInResource resources = DefaultTBuiltInResource;
		const int DefaultVersion = 100;

		/* Preprocess */
		DirStackFileIncluder includer;

		includer.pushExternalLocalDirectory(GetFilePath(filename));

		std::string preprocessedGLSL;

		if (!shader.preprocess(&resources, DefaultVersion, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
		{
			std::cout << "GLSL Preprocessing Failed for: " << filename << std::endl;
			std::cout << shader.getInfoLog() << std::endl;
			std::cout << shader.getInfoDebugLog() << std::endl;
		}

		const char* proprecessedStr = preprocessedGLSL.c_str();
		shader.setStrings(&proprecessedStr, 1);

		/* Compile */
		if (!shader.parse(&resources, 100, false, messages))
		{
			std::cout << "GLSL Parsing Failed for: " << filename << std::endl;
			std::cout << shader.getInfoLog() << std::endl;
			std::cout << shader.getInfoDebugLog() << std::endl;
		}

		/* Link */
		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages))
		{
			std::cout << "GLSL Linking Failed for: " << filename << std::endl;
			std::cout << shader.getInfoLog() << std::endl;
			std::cout << shader.getInfoDebugLog() << std::endl;
		}

		/* Convert to SPIRV */
		std::vector<unsigned int> spirV;
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
		glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirV, &logger, &spvOptions);

		return spirV;
	}

	Shader* ShaderManager::CreateShaderOnline(std::string vertexShaderFilename, std::string pixelShaderFilename, std::string geometryShaderFilename)
	{
		/* Vertex shader */
		VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
		vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderCreateInfo.pName = "main";
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		const std::vector<unsigned int> vertexSpirv = CompileShader(vertexShaderFilename);

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = vertexSpirv.size() * sizeof(unsigned int);
		moduleCreateInfo.pCode = vertexSpirv.data();

		VkShaderModule vertexShaderModule;
		VulkanDebug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &vertexShaderModule));
		vertexShaderCreateInfo.module = vertexShaderModule;

		/* Pixel shader */
		VkPipelineShaderStageCreateInfo pixelShaderCreateInfo = {};
		pixelShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pixelShaderCreateInfo.pName = "main";
		pixelShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		const std::vector<unsigned int> pixelSpirv = CompileShader(pixelShaderFilename);

		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = pixelSpirv.size() * sizeof(unsigned int);
		moduleCreateInfo.pCode = pixelSpirv.data();

		VkShaderModule pixelShaderModule;
		VulkanDebug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &pixelShaderModule));
		pixelShaderCreateInfo.module = pixelShaderModule;

		Shader* shader = new Shader();

		shader->AddShaderStage(vertexShaderCreateInfo);
		shader->AddShaderStage(pixelShaderCreateInfo);

		// Todo: geometry shader

		mLoadedShaders.push_back(shader);

		return shader;
	}

	Shader* ShaderManager::CreateComputeShader(std::string computeShaderFilename)
	{
		// Compute shader
		VkPipelineShaderStageCreateInfo computeShaderCreateInfo = {};
		computeShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderCreateInfo.pName = "main";
		computeShaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderCreateInfo.module = LoadShader(computeShaderFilename.c_str(), VK_SHADER_STAGE_COMPUTE_BIT);

		Shader* shader = new Shader();

		shader->AddShaderStage(computeShaderCreateInfo);

		mLoadedShaders.push_back(shader);
		
		return shader;
	}

	VkShaderModule ShaderManager::LoadShader(std::string filename, VkShaderStageFlagBits stage)
	{
		std::ifstream is(filename.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

		if (is.is_open())
		{
			size_t size = is.tellg();
			is.seekg(0, std::ios::beg);
			char* shaderCode = new char[size];
			is.read(shaderCode, size);
			is.close();

			assert(size > 0);

			VkShaderModule shaderModule;
			VkShaderModuleCreateInfo moduleCreateInfo{};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = size;
			moduleCreateInfo.pCode = (uint32_t*)shaderCode;

			VulkanDebug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &shaderModule));

			delete[] shaderCode;

			return shaderModule;
		}
		else
		{
			std::cerr << "Error: Could not open shader file \"" << filename.c_str() << "\"" << std::endl;
			return nullptr;
		}
	}

	void ShaderCreateInfo::AddVertexShaderSource(std::string source)
	{
		mVertexShaderSources.push_back(source);
	}

	void ShaderCreateInfo::AddPixelShaderSource(std::string source)
	{
		mPixelShaderSources.push_back(source);
	}

	void ShaderCreateInfo::AddGeometryShaderSource(std::string source)
	{
		mGeometryShaderSources.push_back(source);
	}
}