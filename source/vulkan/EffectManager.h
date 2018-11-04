#pragma once
#include "utility/Module.h"
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include <vector>

namespace Utopian::Vk
{
	class EffectManager : public Module<EffectManager>
	{
	public:
		EffectManager();
		~EffectManager();

		template <typename T>
		SharedPtr<T> AddEffect(Device* device, RenderPass* renderPass);

		// Recompiles shaders if requested from the UI
		void Update();
	private:
		std::vector<SharedPtr<Effect>> mEffects;
	};

	EffectManager& gEffectManager();

	template<typename T>
	inline SharedPtr<T> EffectManager::AddEffect(Device* device, RenderPass* renderPass)
	{
		SharedPtr<T> effect = make_shared<T>(device, renderPass);

		mEffects.push_back(effect);
		return effect;
	}
}
