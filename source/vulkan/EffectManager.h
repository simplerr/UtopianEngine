#pragma once
#include "utility/Module.h"
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/ShaderFactory.h"
#include <vector>
#include <functional>
#include <string>

namespace Utopian::Vk
{
	class EffectManager : public Module<EffectManager>
	{
	public:
		EffectManager();
		~EffectManager();

		template <typename T>
		SharedPtr<T> AddEffect(Device* device, RenderPass* renderPass);

		template <typename T>
		SharedPtr<T> AddEffect(Device* device, RenderPass* renderPass, const ShaderCreateInfo& shaderCreateInfo);

		/** Recompiles shaders if requested from the UI. */
		void Update();

		/** Registers a callback function to be called when a shader is recompiled. */
		template<class ...Args>
		void RegisterRecompileCallback(Args &&...args)
		{
			std::function<void(std::string)> recompileCallback = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
			mRecompileCallbacks.push_back(recompileCallback);
		}

	private:
		void NotifyCallbacks(std::string name);

	private:
		std::vector<SharedPtr<Effect>> mEffects;
		std::vector<std::function<void(std::string)>> mRecompileCallbacks;
	};

	EffectManager& gEffectManager();

	template<typename T>
	inline SharedPtr<T> EffectManager::AddEffect(Device* device, RenderPass* renderPass)
	{
		SharedPtr<T> effect = std::make_shared<T>(device, renderPass);

		mEffects.push_back(effect);
		return effect;
	}

	template<typename T>
	inline SharedPtr<T> EffectManager::AddEffect(Device* device, RenderPass* renderPass, const ShaderCreateInfo& shaderCreateInfo)
	{
		SharedPtr<T> effect = std::make_shared<T>(device, renderPass, shaderCreateInfo);

		mEffects.push_back(effect);
		return effect;
	}
}
