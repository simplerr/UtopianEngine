#include "vulkan/EffectManager.h"
#include "vulkan/Effect.h"
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
		Vk::UIOverlay::BeginWindow("Effects", glm::vec2(10, 600), 300.0f);

		for (auto& effect : mEffects)
		{
			std::string path = effect->GetVertexShaderPath();
			if (ImGui::Button(path.c_str()))
			{
				effect->RecompileShader();
				NotifyCallbacks(effect->GetVertexShaderPath());
			}
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
}