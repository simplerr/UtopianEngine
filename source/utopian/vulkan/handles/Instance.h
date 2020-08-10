#pragma once

#include "vulkan/VulkanPrerequisites.h"
#include <string>

namespace Utopian::Vk
{
	/** Wrapper for the Vulkan instance. */
	class Instance 
	{
	public:
		Instance(std::string appName, bool enableValidation);
		~Instance();

		VkInstance GetVkHandle();
		bool IsExtensionSupported(std::string extension) const;
	private:
		VkInstance mInstance;
	};
}
