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
	struct TrackedEffect
	{
		SharedPtr<Effect> effect;
		time_t lastModification;
	};

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
		uint64_t GetLatestModification(const ShaderCreateInfo& shaderCreateInfo);

	private:
		std::vector<TrackedEffect> mEffects;
		std::vector<std::function<void(std::string)>> mRecompileCallbacks;
	};

	EffectManager& gEffectManager();

	// NOTE: TODO: Remove this
	template<typename T>
	inline SharedPtr<T> EffectManager::AddEffect(Device* device, RenderPass* renderPass)
	{
		SharedPtr<T> effect = std::make_shared<T>(device, renderPass);
		TrackedEffect trackedEffect;
		trackedEffect.effect = effect;

		// Set current time
		time_t currentTime;
		time(&currentTime);
		trackedEffect.lastModification = currentTime;

		mEffects.push_back(trackedEffect);
		return effect;
	}

	template<typename T>
	inline SharedPtr<T> EffectManager::AddEffect(Device* device, RenderPass* renderPass, const ShaderCreateInfo& shaderCreateInfo)
	{
		TrackedEffect trackedEffect;
		trackedEffect.effect = std::make_shared<T>(device, renderPass, shaderCreateInfo);
		trackedEffect.lastModification = GetLatestModification(shaderCreateInfo);

		mEffects.push_back(trackedEffect);
		return trackedEffect.effect;
	}
}
