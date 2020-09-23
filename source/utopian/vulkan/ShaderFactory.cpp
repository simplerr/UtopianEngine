#include <cassert>
#include <fstream>
#include <iostream>
#include "ShaderFactory.h"
#include "vulkan/handles/Device.h"
#include "Debug.h"
#include "core/Log.h"
#include <glslang/SPIRV/GlslangToSpv.h>
#include <DirStackFileIncluder.h>
#include <ResourceLimits.h>
#include "core/renderer/Renderer.h"

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

	ShaderFactory& gShaderFactory()
	{
		return ShaderFactory::Instance();
	}

	Shader::Shader(Device* device)
	{
		mDevice = device;
	}

	Shader::~Shader()
	{
      for (auto shader : compiledShaders)
      {
         vkDestroyShaderModule(mDevice->GetVkDevice(), shader->shaderModule, nullptr);
      }
	}

	void Shader::AddShaderStage(VkPipelineShaderStageCreateInfo shaderStageCreateInfo)
	{
		shaderStages.push_back(shaderStageCreateInfo);
	}

	void Shader::AddCompiledShader(SharedPtr<CompiledShader> compiledShader)
	{
		compiledShaders.push_back(compiledShader);

		VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
		shaderCreateInfo.module = compiledShader->shaderModule;
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderCreateInfo.pName = "main";
		shaderCreateInfo.stage = compiledShader->shaderStage;
		AddShaderStage(shaderCreateInfo);
	}

	int Shader::NameToBinding(std::string name)
	{
		for (auto& compiledShader : compiledShaders)
		{
			auto nameMapping = compiledShader->reflection.nameMappings;
			if (nameMapping.find(name) != nameMapping.end())
			{
				return nameMapping[name].binding;
			}
		}

		assert(0);
	}

	int Shader::NameToSet(std::string name)
	{
		for (auto& compiledShader : compiledShaders)
		{
			auto nameMapping = compiledShader->reflection.nameMappings;
			if (nameMapping.find(name) != nameMapping.end())
			{
				return nameMapping[name].set;
			}
		}

		assert(0);
	}

	const VertexDescription* Shader::GetVertexDescription() const
	{
		for (auto& compiledShader : compiledShaders)
		{
			if (compiledShader->shaderStage == VK_SHADER_STAGE_VERTEX_BIT)
				return compiledShader->reflection.vertexDescription.get();
		}

		assert(0);
	}

	bool Shader::IsComputeShader() const
	{
		for (auto& compiledShader : compiledShaders)
		{
			if (compiledShader->shaderStage == VK_SHADER_STAGE_COMPUTE_BIT)
				return true;
		}

		return false;
	}
	
	ShaderFactory::ShaderFactory(Device* device)
		: mDevice(device)
	{
	}

	ShaderFactory::~ShaderFactory()
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

	EShLanguage GetGlslangStage(const std::string& stage)
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

	VkShaderStageFlagBits GetVulkanShaderStage(const std::string& suffix)
	{
		if (suffix == "vert")
			return VK_SHADER_STAGE_VERTEX_BIT;
		else if (suffix == "frag")
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		else if (suffix == "geom")
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		else if (suffix == "tesc")
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		else if (suffix == "tese")
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		else if (suffix == "comp")
			return VK_SHADER_STAGE_COMPUTE_BIT;
		else {
			assert(0 && "Unknown shader stage");
		}
	}

	SharedPtr<CompiledShader> ShaderFactory::CompileShader(std::string filename)
	{
		SharedPtr<CompiledShader> compiledShader = nullptr;
		bool error = false;

		glslang::InitializeProcess();

		std::ifstream file(filename);

		std::string glslString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const char* glslSource = glslString.c_str();

		EShLanguage shaderType = GetGlslangStage(GetSuffix(filename));

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

		for (auto& directory : mIncludeDirectories)
			includer.pushExternalLocalDirectory(directory);

		std::string preprocessedGLSL;

		if (!shader.preprocess(&resources, DefaultVersion, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
		{
			UTO_LOG("GLSL Preprocessing Failed for: " + filename);
			UTO_LOG(shader.getInfoLog());
			UTO_LOG(shader.getInfoDebugLog());
			error = true;
		}

		const char* proprecessedStr = preprocessedGLSL.c_str();
		shader.setStrings(&proprecessedStr, 1);

		/* Compile */
		if (!error && !shader.parse(&resources, 100, false, messages))
		{
         UTO_LOG("GLSL Parsing Failed for: " + filename);
         UTO_LOG(shader.getInfoLog());
         UTO_LOG(shader.getInfoDebugLog());
			error = true;
		}

		/* Link */
		glslang::TProgram program;
		program.addShader(&shader);

		if (!error && !program.link(messages))
		{
         UTO_LOG("GLSL Linking Failed for: " + filename);
         UTO_LOG(shader.getInfoLog());
         UTO_LOG(shader.getInfoDebugLog());
			error = true;
		}

		if (!error)
		{
			/* Convert to SPIRV */
			std::vector<unsigned int> spirV;
			spv::SpvBuildLogger logger;
			glslang::SpvOptions spvOptions;
			glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirV, &logger, &spvOptions);

			/* Shader reflection */
			ShaderReflection reflection = ExtractShaderLayout(program, shaderType);

			compiledShader = std::make_shared<CompiledShader>();
			compiledShader->spirvBytecode = spirV;
			compiledShader->reflection = reflection;
			compiledShader->shaderStage = GetVulkanShaderStage(GetSuffix(filename));
		}

		return compiledShader;
	}

	void ShaderFactory::AddIncludeDirectory(std::string directory)
	{
		mIncludeDirectories.push_back(directory);
	}

	/*
		Currently only supports reflection of UBOs and combines image samplers
	*/
	ShaderReflection ShaderFactory::ExtractShaderLayout(glslang::TProgram& program, EShLanguage shaderType)
	{
		ShaderReflection reflection;

		program.mapIO();
		program.buildReflection();

		/* Parse uniform blocks */
		int numBlocks = program.getNumLiveUniformBlocks();
		for (int i = 0; i < numBlocks; i++)
		{
			const glslang::TType* ttype = program.getUniformBlockTType(i);
			const glslang::TQualifier& qualifier = ttype->getQualifier();
			const char* name = program.getUniformBlockName(i);

			if (!qualifier.hasBinding())
			{
				if (qualifier.layoutPushConstant)
				{
					PushConstantDesc desc;
					desc.name = name;
					desc.size = program.getUniformBlockSize(i); 
					reflection.pushConstants[desc.name] = desc;
				}
				else
					assert(0);
			}
			else
			{
				if (qualifier.storage == glslang::EvqBuffer)	// SSBO
				{
					UniformBlockDesc desc;
					desc.size = program.getUniformBlockSize(i); // todo: / 4
					desc.set = qualifier.layoutSet;
					desc.binding = qualifier.layoutBinding;
					desc.name = name;
					reflection.storageBuffers[desc.name] = desc;
					reflection.nameMappings[desc.name] = NameMapping(desc.set, desc.binding);
				}
				else if (qualifier.storage == glslang::EvqUniform)
				{
					UniformBlockDesc desc;
					desc.size = program.getUniformBlockSize(i); // todo: / 4
					desc.set = qualifier.layoutSet;
					desc.binding = qualifier.layoutBinding;
					desc.name = name;
					reflection.uniformBlocks[desc.name] = desc;
					reflection.nameMappings[desc.name] = NameMapping(desc.set, desc.binding);
				}
			}
		}

		/* Parse individual uniforms */
		int numUniforms = program.getNumLiveUniformVariables();
		for (int i = 0; i < numUniforms; i++)
		{
			const glslang::TType* ttype = program.getUniformTType(i);
			const glslang::TQualifier& qualifier = ttype->getQualifier();
			const char* name = program.getUniformName(i);
			const int32_t arraySize = program.getUniformArraySize(i);

			glslang::TBasicType basicType = ttype->getBasicType();
			if (ttype->getBasicType() == glslang::EbtSampler)
			{
				if (!qualifier.hasBinding())
				{
					assert(0);
				}

				UniformVariableDesc desc;
				desc.name = name;
				desc.set = qualifier.layoutSet;
				desc.binding = qualifier.layoutBinding;
				desc.arraySize = arraySize;

				const glslang::TSampler& sampler = ttype->getSampler();

				/* Combined image samplers */
				if (sampler.isCombined())
				{
					switch (sampler.dim)
					{
						case glslang::Esd1D:	desc.type = UVT_SAMPLER1D; break;
						case glslang::Esd2D:	desc.type = UVT_SAMPLER2D; break;
						case glslang::Esd3D:	desc.type = UVT_SAMPLER3D; break;
						case glslang::EsdCube:	desc.type = UVT_SAMPLERCUBE; break;
					}

					reflection.combinedSamplers[desc.name] = desc;
					reflection.nameMappings[desc.name] = NameMapping(desc.set, desc.binding);
				}
				else if (sampler.isImage())
				{
					switch (sampler.dim)
					{
						case glslang::Esd1D:	desc.type = UVT_SAMPLER1D; break;
						case glslang::Esd2D:	desc.type = UVT_SAMPLER2D; break;
						case glslang::Esd3D:	desc.type = UVT_SAMPLER3D; break;
						case glslang::EsdCube:	desc.type = UVT_SAMPLERCUBE; break;
					}

					reflection.images[desc.name] = desc;
					reflection.nameMappings[desc.name] = NameMapping(desc.set, desc.binding);
				}
				else
				{
					// Todo: Map uniform variables in UBOs to the correct block (not needed for pipeline layout creation)
				}
			}
		}

		/* Parse vertex input layout if it's a vertex shader */
		if (shaderType == EShLangVertex)
		{
			ReflectVertexInput(program, &reflection);
		}

		return reflection;
	}

	void ShaderFactory::ReflectVertexInput(glslang::TProgram& program, ShaderReflection* reflection)
	{
		// The VertexDescription class assumes that the input attributes
		// are added in the same order as their layout location.
		// Therefor they are added to a map which is sorted on the location index.
		std::map<uint32_t, const glslang::TType*> inputMap;
		for (uint32_t i = 0; i < program.getNumLiveAttributes(); i++)
		{
			const glslang::TType* ttype = program.getAttributeTType(i);
			uint32_t location = ttype->getQualifier().layoutLocation;

			if (location == (uint32_t)-1)
				assert(0);

			inputMap[location] = ttype;
		}

		reflection->vertexDescription = std::make_shared<VertexDescription>();
		uint32_t totalSize = 0;
		for (auto& iter : inputMap)
		{
			const glslang::TType* ttype = iter.second;

			if (ttype->isVector())
			{
				uint32_t vectorSize = ttype->getVectorSize();
				glslang::TBasicType basicType = ttype->getBasicType();

				if (basicType == glslang::EbtFloat)
				{
					switch (vectorSize)
					{
						// Todo: Binding is hardcoded to 0
					case 2:
						reflection->vertexDescription->AddAttribute(BINDING_0, Vec2Attribute());
						totalSize += sizeof(glm::vec2);
						break;
					case 3:
						reflection->vertexDescription->AddAttribute(BINDING_0, Vec3Attribute());
						totalSize += sizeof(glm::vec3);
						break;
					case 4:
						reflection->vertexDescription->AddAttribute(BINDING_0, Vec4Attribute());
						totalSize += sizeof(glm::vec4);
						break;
					default:
						assert(0);
						break;
					}
				}
				else
					assert(0);
			}
			// Note: Todo: For some reason when using gl_VertexIndex this assert gets triggered.
			//else
			//	assert(0);
		}

		// Only add binding if attributes are found
		if (totalSize != 0)
			reflection->vertexDescription->AddBinding(BINDING_0, totalSize, VK_VERTEX_INPUT_RATE_VERTEX);
	}

   SharedPtr<Shader> ShaderFactory::CreateShader(const ShaderCreateInfo& shaderCreateInfo)
   {
		SharedPtr<Shader> shader = nullptr;

		/* Vertex shader */
		SharedPtr<CompiledShader> compiledVertexShader;
		if (shaderCreateInfo.vertexShaderPath != "NONE")
		{
			compiledVertexShader = CompileShader(shaderCreateInfo.vertexShaderPath);
			if (compiledVertexShader != nullptr)
			{
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = compiledVertexShader->spirvBytecode.size() * sizeof(unsigned int);
				moduleCreateInfo.pCode = compiledVertexShader->spirvBytecode.data();

				Debug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &compiledVertexShader->shaderModule));
			}
		}

		/* Pixel shader */
		SharedPtr<CompiledShader> compiledPixelShader;
		if (shaderCreateInfo.fragmentShaderPath != "NONE")
		{
			compiledPixelShader = CompileShader(shaderCreateInfo.fragmentShaderPath);
			if (compiledPixelShader != nullptr)
			{
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = compiledPixelShader->spirvBytecode.size() * sizeof(unsigned int);
				moduleCreateInfo.pCode = compiledPixelShader->spirvBytecode.data();

				Debug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &compiledPixelShader->shaderModule));
			}
		}

		/* Geometry shader */
		SharedPtr<CompiledShader> compiledGeometryShader;
		if (shaderCreateInfo.geometryShaderPath != "NONE")
		{
			compiledGeometryShader = CompileShader(shaderCreateInfo.geometryShaderPath);
			if (compiledGeometryShader != nullptr)
			{
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = compiledGeometryShader->spirvBytecode.size() * sizeof(unsigned int);
				moduleCreateInfo.pCode = compiledGeometryShader->spirvBytecode.data();

				Debug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &compiledGeometryShader->shaderModule));
			}
		}	

		/* Tessellation control shader */
		SharedPtr<CompiledShader> compiledTescShader;
		if (shaderCreateInfo.tescShaderPath != "NONE")
		{
			compiledTescShader = CompileShader(shaderCreateInfo.tescShaderPath);
			if (compiledTescShader != nullptr)
			{
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = compiledTescShader->spirvBytecode.size() * sizeof(unsigned int);
				moduleCreateInfo.pCode = compiledTescShader->spirvBytecode.data();

				Debug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &compiledTescShader->shaderModule));
			}
		}	

		/* Tessellation evaluation shader */
		SharedPtr<CompiledShader> compiledTeseShader;
		if (shaderCreateInfo.teseShaderPath != "NONE")
		{
			compiledTeseShader = CompileShader(shaderCreateInfo.teseShaderPath);
			if (compiledTeseShader != nullptr)
			{
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = compiledTeseShader->spirvBytecode.size() * sizeof(unsigned int);
				moduleCreateInfo.pCode = compiledTeseShader->spirvBytecode.data();

				Debug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &compiledTeseShader->shaderModule));
			}
		}	

		/* Compute shader */
		SharedPtr<CompiledShader> compiledComputeShader;
		if (shaderCreateInfo.computeShaderPath != "NONE")
		{
			compiledComputeShader = CompileShader(shaderCreateInfo.computeShaderPath);
			if (compiledComputeShader != nullptr)
			{
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = compiledComputeShader->spirvBytecode.size() * sizeof(unsigned int);
				moduleCreateInfo.pCode = compiledComputeShader->spirvBytecode.data();

				Debug::ErrorCheck(vkCreateShaderModule(mDevice->GetVkDevice(), &moduleCreateInfo, NULL, &compiledComputeShader->shaderModule));
			}
		}

		if (compiledVertexShader != nullptr && compiledPixelShader != nullptr)
		{
			shader = std::make_shared<Shader>(mDevice);
			shader->AddCompiledShader(compiledVertexShader);
			shader->AddCompiledShader(compiledPixelShader);

			if (shaderCreateInfo.geometryShaderPath != "NONE")
			{
				assert(compiledGeometryShader);
				shader->AddCompiledShader(compiledGeometryShader);
			}

			if (shaderCreateInfo.tescShaderPath != "NONE")
			{
				assert(compiledTescShader);
				shader->AddCompiledShader(compiledTescShader);
			}

			if (shaderCreateInfo.teseShaderPath != "NONE")
			{
				assert(compiledTeseShader);
				shader->AddCompiledShader(compiledTeseShader);
			}
		}

		if (compiledComputeShader != nullptr)
		{
			shader = std::make_shared<Shader>(mDevice);
			shader->AddCompiledShader(compiledComputeShader);
		}

		return shader;
	}
}