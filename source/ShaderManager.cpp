#include <cassert>
#include <fstream>
#include <iostream>
#include "ShaderManager.h"
#include "Device.h"
#include "VulkanDebug.h"
#include "Shader.h"

namespace VulkanLib
{
	Shader::Shader(VkPipelineShaderStageCreateInfo vertexShaderCreateInfo, VkPipelineShaderStageCreateInfo pixelShaderCreateInfo)
	{
		shaderStages[0] = vertexShaderCreateInfo;
		shaderStages[1] = pixelShaderCreateInfo;
	}
	
	ShaderManager::ShaderManager(Device* device)
		: mDevice(device)
	{

	}

	ShaderManager::~ShaderManager()
	{
		for (int i = 0; i < mLoadedShaders.size(); i++)
		{
			vkDestroyShaderModule(mDevice->GetVkDevice(), mLoadedShaders[i]->shaderStages[VERTEX_SHADER_IDX].module, nullptr);
			vkDestroyShaderModule(mDevice->GetVkDevice(), mLoadedShaders[i]->shaderStages[PIXEL_SHADER_IDX].module, nullptr);

			delete mLoadedShaders[i];
		}
	}

	Shader* ShaderManager::CreateShader(std::string vertexShaderFilename, std::string pixelShaderFilename)
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

		Shader* shader = new Shader(vertexShaderCreateInfo, pixelShaderCreateInfo);

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
}