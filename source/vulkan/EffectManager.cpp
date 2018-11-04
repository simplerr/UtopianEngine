#include "vulkan/EffectManager.h"
#include "vulkan/handles/Effect.h"
#include "imgui/imgui.h"

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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(10, 360));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("EffectManager", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PushItemWidth(300.0f);

		for (auto& effect : mEffects)
		{
			std::string path = effect->GetVertexShaderPath();
			if (ImGui::Button(path.c_str()))
			{
				effect->RecompileShader();
			}
		}

		ImGui::PopItemWidth();

		// ImGui functions end here
		ImGui::End();
		ImGui::PopStyleVar();
	}
}