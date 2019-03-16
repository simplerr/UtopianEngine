#include "vulkan/EffectManager.h"
#include "vulkan/Effect.h"
#include "vulkan/Debug.h"
#include "imgui/imgui.h"
#include "vulkan/UIOverlay.h"

namespace Utopian::Vk
{
	EffectManager::EffectManager()
	{
		
	}

	EffectManager::~EffectManager()
	{
	}

	EffectManager& gEffectManager()
	{
		return EffectManager::Instance();
	}

	void EffectManager::Update()
	{
		Vk::UIOverlay::BeginWindow("Effects", glm::vec2(10, 750), 300.0f);

		if (ImGui::Button("Recompile modified shaders"))
		{
			for (auto& trackedEffect : mEffects)
			{
				ShaderCreateInfo shaderCreateInfo = trackedEffect.effect->GetShaderCreateInfo();
				time_t updatedModificationTime = GetLatestModification(shaderCreateInfo);
				if (updatedModificationTime > trackedEffect.lastModification)
				{
					trackedEffect.effect->RecompileShader();
					NotifyCallbacks(trackedEffect.effect->GetVertexShaderPath());
					trackedEffect.lastModification = updatedModificationTime;

					Debug::ConsolePrint("Recompiled \"" + shaderCreateInfo.fragmentShaderPath + "\"");
				}
			}
		}

		if (ImGui::Button("Recompile all shaders"))
		{
			for (auto& trackedEffect : mEffects)
			{
				ShaderCreateInfo shaderCreateInfo = trackedEffect.effect->GetShaderCreateInfo();
				time_t updatedModificationTime = GetLatestModification(shaderCreateInfo);
				trackedEffect.effect->RecompileShader();
				NotifyCallbacks(trackedEffect.effect->GetVertexShaderPath());
				trackedEffect.lastModification = updatedModificationTime;
			}

			Debug::ConsolePrint("Recompiled all shaders");
		}

		Vk::UIOverlay::EndWindow();
	}

	void EffectManager::NotifyCallbacks(std::string name)
	{
		for (auto& callback : mRecompileCallbacks)
		{
			callback(name);
		}
	}

	uint64_t EffectManager::GetLatestModification(const ShaderCreateInfo& shaderCreateInfo)
	{
		time_t latest = 0;
		struct stat file_details;

		if (stat(shaderCreateInfo.vertexShaderPath.c_str(), &file_details) == 0)
			latest = glm::max(latest, file_details.st_mtime);

		if (stat(shaderCreateInfo.fragmentShaderPath.c_str(), &file_details) == 0)
			latest = glm::max(latest, file_details.st_mtime);

		if (stat(shaderCreateInfo.geometryShaderPath.c_str(), &file_details) == 0)
			latest = glm::max(latest, file_details.st_mtime);

		if (stat(shaderCreateInfo.tescShaderPath.c_str(), &file_details) == 0)
			latest = glm::max(latest, file_details.st_mtime);

		if (stat(shaderCreateInfo.teseShaderPath.c_str(), &file_details) == 0)
			latest = glm::max(latest, file_details.st_mtime);

		//printf("Last modified time: %s", ctime(&latest));
		return latest;
	}
}