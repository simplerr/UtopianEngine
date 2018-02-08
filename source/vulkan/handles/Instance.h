#pragma once

#include "vulkan/VulkanInclude.h"
#include <string>

namespace Utopian::Vk
{
	class Instance 
	{
	public:
		Instance(std::string appName, bool enableValidation);
		~Instance();

		VkInstance GetVkHandle();
	private:
		VkInstance mInstance;
	};
}
