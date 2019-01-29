#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	/** Wrapper for VkSampler. */
	class Sampler : public Handle<VkSampler>
	{
	public:
		/**
		 * @param create If false then the Create() function has to to be called manually after the constructor,
		 * this allows for modifying the sampler create info.
		 */
		Sampler(Device* device, bool create = true);

		void Create();

		/** Public so it can be modified before calling Create(). */
		VkSamplerCreateInfo createInfo = {};
	private:
	};
}
