#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class Sampler : public Handle<VkSampler>
	{
	public:
		/**
		@param create If false then the Create() function has to to be called manually after the constructor,
			          this allows for modifying the renderpass configuration.
		*/
		Sampler(Device* device, bool create = true);

		void Create();

		VkSamplerCreateInfo samplerInfo = {};
	private:
	};
}
